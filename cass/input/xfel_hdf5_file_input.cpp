// Copyright (C) 2017 Lutz Foucar

/**
 * @file xfel_hdf5_file_input.cpp contains class for reading xfel hdf5 data files
 *
 * @author Lutz Foucar
 */

#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <sstream>
#include <stdexcept>

#include <QtCore/QFileInfo>

#include "xfel_hdf5_file_input.h"

#include "cass_event.h"
#include "cass_settings.h"
#include "log.h"
#include "hdf5_handle.hpp"
#include "machine_device.hpp"
#include "acqiris_device.hpp"
#include "pixeldetector.hpp"

using namespace std;
using namespace cass;

namespace cass
{
/** A machine vale
 *
 * allow the user to have a CASSName next to the hdf5 key as the key might be
 * unreadable. Also allow the index in case the value is contained in it
 *
 * @author Lutz Foucar
 */
struct machineVal
{
  /** the hdf5 file key */
  string h5key;

  /** the name of the key within the cass event */
  string cassname;

  /** in case the machinval in question is contained within an array this is
   *  the inxed of the array where the machine value iswritten to
   */
  int idx;
};
}//end namespace cass

void XFELHDF5FileInput::instance(string filelistname,
                             RingBuffer<CASSEvent> &ringbuffer,
                             Ratemeter &ratemeter, Ratemeter &loadmeter,
                             bool quitWhenDone,
                             QObject *parent)
{
  if(_instance)
    throw logic_error("HDF5FileInput::instance(): The instance of the base class is already initialized");
  _instance = shared_pointer(new XFELHDF5FileInput(filelistname,ringbuffer,ratemeter,loadmeter,quitWhenDone,parent));
}

XFELHDF5FileInput::XFELHDF5FileInput(string filelistname,
                                     RingBuffer<CASSEvent> &ringbuffer,
                                     Ratemeter &ratemeter, Ratemeter &loadmeter,
                                     bool quitWhenDone,
                                     QObject *parent)
  : InputBase(ringbuffer,ratemeter,loadmeter,parent),
    _quitWhenDone(quitWhenDone),
    _filelistname(filelistname)
{
  Log::add(Log::VERBOSEINFO, "HDF5FileInput::HDF5FileInput: constructed");
}

void XFELHDF5FileInput::load()
{
}

void XFELHDF5FileInput::runthis()
{
  /** load the settings from the ini file */
  CASSSettings s;
  s.beginGroup("XFELHDF5FileInput");

  /** the string for the event id of the current event */
  string EventIDName = s.value("EventIDKey","EventID").toString().toStdString();

  /** list of machine variables to extract from the hdf5 files */
  typedef vector<machineVal> machineVals_t;
  machineVals_t machineVals;
  int size = s.beginReadArray("MachineValues");
  for (int i = 0; i < size; ++i)
  {
    s.setArrayIndex(i);
    string machineValName(s.value("HDF5Key","Invalid").toString().toStdString());
    string CASSName(s.value("CASSName","Invalid").toString().toStdString());
    /** skip if the value name has not been set */
    if (machineValName == "Invalid")
      continue;
    CASSName = CASSName == "Invalid" ? machineValName : CASSName;
    machineVals.push_back(machineVals_t::value_type());
    machineVals.back().h5key = machineValName;
    machineVals.back().cassname = CASSName;
    machineVals.back().idx = s.value("ArrayIndex",0).toUInt();
  }
  s.endArray();

  /** list of acqiris channels that should be extracted from the hdf5 files */
  typedef vector<AcqirisParams> acqirisVals_t;
  acqirisVals_t acqirisVals;
  size = s.beginReadArray("AcqirisValues");
  for (int i = 0; i < size; ++i)
  {
    s.setArrayIndex(i);
    string datakey= s.value("HDF5DataKey","Invalid").toString().toStdString();
    /** skip if the name has not been set */
    if (datakey == "Invalid")
      continue;
    acqirisVals.push_back(AcqirisParams());
    acqirisVals.back().Instrument = s.value("CASSInstrumentID",1).toUInt();
    acqirisVals.back().ChannelNumber = s.value("CASSChannelNumber",0).toUInt();
    acqirisVals.back().DataKey = datakey;
    acqirisVals.back().HorposKey = s.value("HDF5HorposKey","Invalid").toString().toStdString();
    acqirisVals.back().VertOffsetKey = s.value("HDF5VerticalOffsetKey","Invalid").toString().toStdString();
    acqirisVals.back().GainKey = s.value("HDF5GainKey","Invalid").toString().toStdString();
    acqirisVals.back().SampleIntervalKey = s.value("HDF5SampleIntervalKey","Invalid").toString().toStdString();
  }
  s.endArray();

  /** list of pixeldetectors that should be extracted from the hdf5 files */
  typedef vector<PixeldetectorParams> pixdetVals_t;
  pixdetVals_t pixdetVals;
  size = s.beginReadArray("PixelDetectorValues");
  for (int i = 0; i < size; ++i)
  {
    s.setArrayIndex(i);
    string datakey= s.value("HDF5DataKey","Invalid").toString().toStdString();
    /** skip if the name has not been set */
    if (datakey == "Invalid")
      continue;
    pixdetVals.push_back(PixeldetectorParams());
    pixdetVals.back().DataKey = datakey;
    pixdetVals.back().CASSID = s.value("CASSID",0).toInt();
  }
  s.endArray();

  s.endGroup();

  _status = lmf::PausableThread::running;
  Tokenizer tokenize;

  /** retrieve all files in a list from the file */
  Log::add(Log::VERBOSEINFO,"HDF5FileInput::run(): try to open filelist '" +
           _filelistname + "'");
  ifstream filelistfile(_filelistname.c_str());
  if (!filelistfile.is_open())
    throw invalid_argument("HDF5FileInput::run(): filelist '" + _filelistname +
                           "' could not be opened");
  vector<string> filelist(tokenize(filelistfile));
  filelistfile.close();

  /** go through the list of files and create a processor for each file and
   *  add them to the list of processors
   */
  vector<string>::const_iterator filelistIt(filelist.begin());
  vector<string>::const_iterator filelistEnd(filelist.end());
  uint64_t eventcounter(0);
  for (;(!shouldQuit()) && (filelistIt != filelistEnd); ++filelistIt)
  {
    /** check if file exists, then load it */
    string filename(*filelistIt);
    QFileInfo info(QString::fromStdString(filename));
    if (info.exists())
    {
      /** open the hdf5 file */
      hdf5::Handler h5handle(filename,"r");

      /** get the list of groups of the root group ('/') as they should reflect
       *  the events
       */
      list<string> events(h5handle.rootGroups());

      /** iterate through the list and extract the requested informations */
      list<string>::const_iterator it(events.begin());
      list<string>::const_iterator EventsEnd(events.end());
      for (; (!shouldQuit()) && (it != EventsEnd); ++it)
      {

        /** retrieve a new element from the ringbuffer. If one didn't get a
         *  an element (when the end iterator of the buffer is returned).
         *  Continue to the next iteration, where it is checked if the thread
         *  should quit.
         */
        InputBase::rbItem_t rbItem(getNextFillable());
        if (rbItem == _ringbuffer.end())
          continue;
        CASSEvent& evt(*rbItem->element);

        /** fill the cassevent object with the contents from the file */
        bool isGood(true);

        /** set the event id from the retrieved id field */
        if (h5handle.dimension(*it + "/" + EventIDName) != 0)
          throw invalid_argument("XFELHDF5FileInput:run(): EventID from '"+
                                 *it + "/" + EventIDName + "' is not a scalar number");
        evt.id() = h5handle.readScalar<int>(*it + "/" + EventIDName);

        /** check if the eventid is valid (non zero number) */
        if (evt.id() == 0)
          isGood = false;


        /** get reference to all devices of the CASSEvent and an iterator*/
        CASSEvent::devices_t &devices(evt.devices());
        CASSEvent::devices_t::iterator devIt;

        /** try to retrieve the data */
        try
        {
          /** ####  machine data  #### */

          /** check if the event contains the machine data container, if so get a
           *  reference to it. Otherwise throw an error.
           */
          devIt = devices.find(CASSEvent::MachineData);
          if (devIt == devices.end())
            throw runtime_error("XFELHDF5FileInput():The CASSEvent does not contain a Machine Data Device");
          MachineData::Device &md (dynamic_cast<MachineData::Device&>(*(devIt->second)));

          /** go through all requested machine data events and retrieve the corresponding
           *  values for the tag */
          machineVals_t::const_iterator machineValsIter(machineVals.begin());
          machineVals_t::const_iterator machineValsEnd(machineVals.end());
          for (; machineValsIter != machineValsEnd; ++machineValsIter)
          {
            const string key(*it + "/" + machineValsIter->h5key);
            /** check if dimension is correct */
            if (h5handle.dimension(key) == 0)
            {
              float machineValue(h5handle.readScalar<float>(key));
              md.BeamlineData()[machineValsIter->cassname] = machineValue;
            }
            else if (h5handle.dimension(key) == 1)
            {
              vector<double> array;
              size_t length(0);
              h5handle.readArray(array, length, key);
              md.BeamlineData()[machineValsIter->cassname] = array[machineValsIter->idx];
            }
            else
            {
              throw runtime_error("XFELHDF5FileInput(): file '"+ filename + "': key '" +
                                  key + "' is neither a scalar nor an array value");
            }
          }


          /** acqiris data */

          /** retrieve a pointer to the right acqiris instrument */
          devIt = devices.find(CASSEvent::Acqiris);
          if (devIt == devices.end())
            throw runtime_error("HDF5FileInput():The CASSEvent does not contain a Acqiris Device");
          ACQIRIS::Device &acq (dynamic_cast<ACQIRIS::Device&>(*(devIt->second)));
          /** go through all requested acqiris data */
          acqirisVals_t::const_iterator acqirisValsIter(acqirisVals.begin());
          acqirisVals_t::const_iterator acqirisValsEnd(acqirisVals.end());
          for (; acqirisValsIter != acqirisValsEnd; ++acqirisValsIter)
          {
            /** retrieve a reference to the right instrument */
            ACQIRIS::Instrument &instr(acq.instruments()[acqirisValsIter->Instrument]);
            /** retrieve a reference to the channel container of the instrument */
            ACQIRIS::Instrument::channels_t &channels(instr.channels());
            /** resize the channel vector to how many channels are in the device */
            channels.resize(acqirisValsIter->ChannelNumber + 1);
            /** retrieve a reference to the channel we are working on */
            ACQIRIS::Channel &chan(channels[acqirisValsIter->ChannelNumber]);
            /** retrieve a reference to waveform container */
            ACQIRIS::Channel::waveform_t &waveform = chan.waveform();
            /** extract the meta infos from the hdf5 file */
            chan.channelNbr()     = acqirisValsIter->ChannelNumber;
            chan.horpos()         = (acqirisValsIter->HorposKey == "Invalid") ? 0 : h5handle.readScalar<double>(*it + "/" + acqirisValsIter->HorposKey);
            chan.offset()         = (acqirisValsIter->VertOffsetKey == "Invalid") ? 0 : h5handle.readScalar<double>(*it + "/" + acqirisValsIter->VertOffsetKey);
            chan.gain()           = (acqirisValsIter->GainKey == "Invalid") ? 1 : h5handle.readScalar<double>(*it + "/" + acqirisValsIter->GainKey);
            chan.sampleInterval() = (acqirisValsIter->SampleIntervalKey == "Invalid") ? 1e-9 : h5handle.readScalar<double>(*it + "/" + acqirisValsIter->SampleIntervalKey);
            /** extract the wavefrom and copy it to the CASSEvent container */
            const string dkey(*it + "/" + acqirisValsIter->DataKey);
            vector<float> array;
            if (h5handle.dimension(dkey) == 1)
            {
              size_t length(0);
              h5handle.readArray(array, length, dkey);
              waveform.resize(length);
            }
            else if (h5handle.dimension(dkey) == 2)
            {
              pair<size_t,size_t> shape;
              h5handle.readMatrix(array, shape, dkey);
              if (shape.second != 1)
                throw invalid_argument("XFELHDF5FileInput: file '" + filename +
                                       "': acqiris data key '" + dkey +
                                       "' is a matrix of '" + toString(shape.first) +
                                       "x" + toString(shape.second) + "'");
              waveform.resize(shape.first*shape.second);
            }
            else
            {
              throw invalid_argument("XFELHDF5FileInput: file '" + filename +
                                     "': acqiris data key '" + dkey +
                                     "' is not an array. It has dimension '" +
                                     toString(h5handle.dimension(dkey)) + "'");
            }
            copy(array.begin(),array.end(),waveform.begin());
            instr.id() = evt.id();
          }


          /** image data */

          /** retrieve the pixel detector part of the cassevent */
          devIt = devices.find(CASSEvent::PixelDetectors);
          if(devIt == devices.end())
            throw runtime_error("XFELHDF5FileInput: CASSEvent does not contains a pixeldetector device");
          pixeldetector::Device &pixdev (dynamic_cast<pixeldetector::Device&>(*(devIt->second)));
          /** go through all requested detectors and retrieve the data */
          pixdetVals_t::const_iterator pixdetValsIter(pixdetVals.begin());
          pixdetVals_t::const_iterator pixdetValsEnd(pixdetVals.end());
          for(; pixdetValsIter != pixdetValsEnd; ++pixdetValsIter)
          {
            /** retrieve the right detector from the cassevent and reset it*/
            pixeldetector::Detector &det(pixdev.dets()[pixdetValsIter->CASSID]);
            det.frame().clear();
            /** get the data from the file */
            string dkey(*it + "/" + pixdetValsIter->DataKey);
            if (h5handle.dimension(dkey) != 2)
              throw invalid_argument("XFELHDF5FileInput: file '" + filename +
                                     "': pixeldetector data key '" + dkey +
                                     "' is not a matrix. It has dimension '" +
                                     toString(h5handle.dimension(dkey)) + "'");
            pair<size_t,size_t> shape;
            h5handle.readMatrix(det.frame(), shape, dkey);
            /** set the metadata of the frame */
            det.columns() = shape.first;
            det.rows() = shape.second;
            det.id() = evt.id();
          }

        }
        catch(const hdf5::DatasetError &error)
        {
          Log::add(Log::ERROR,"XFELHDF5FileInput: file '" + filename +
                   "' is missing data. Error is '" +  error.what() + "'");
          isGood = false;
        }

        /** add the event back to the ringbuffer and let the world know if its
         *  a good event. Also do some output about the rate
         */
        if (!isGood)
          Log::add(Log::WARNING,"XFELHDF5FileInput:: Event with id '"+
                   toString(rbItem->element->id()) + "' is bad: skipping Event");
        else
          ++eventcounter;
        rbItem->element->setFilename(filename.c_str());
        newEventAdded(rbItem->element->datagrambuffer().size());
        _ringbuffer.doneFilling(rbItem, isGood);
      }
    }
    else
      Log::add(Log::ERROR,"XFELHDF5FileInput::run(): could not open '" + filename + "'");
  }

  /** quit the program if requested otherwise wait until the program is signalled
   *  to quit
   */
  Log::add(Log::INFO,"XFELHDF5FileInput::run(): Finished with all files.");
  if(!_quitWhenDone)
    while(!shouldQuit())
      this->sleep(1);
  Log::add(Log::VERBOSEINFO, "XFELHDF5FileInput::run(): closing the input");
  Log::add(Log::INFO,"XFELHDF5FileInput::run(): Extracted '" +
           toString(eventcounter) + "' events.");
}
