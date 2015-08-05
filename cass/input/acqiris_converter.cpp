//Copyright (C) 2009, 2010, 2011 Lutz Foucar

/**
 * @file acqiris_converter.cpp file contains the definition of the converter
 *                             for the xtc containing acqiris data.
 *
 * @author Lutz Foucar
 */

#include <cassert>

#include "acqiris_converter.h"
#include "cass_event.h"
#include "log.h"

#include "pdsdata/xtc/Xtc.hh"
#include "pdsdata/xtc/TypeId.hh"
#include "pdsdata/xtc/DetInfo.hh"
#include "pdsdata/acqiris/ConfigV1.hh"
#include "pdsdata/acqiris/DataDescV1.hh"
#include "pdsdata/xtc/Src.hh"

using namespace cass;
using namespace ACQIRIS;
using namespace std;

// =================define static members =================
ConversionBackend::shared_pointer Converter::_instance;
QMutex Converter::_mutex;

ConversionBackend::shared_pointer Converter::instance()
{
  QMutexLocker locker(&_mutex);
  if(!_instance)
  {
    _instance = ConversionBackend::shared_pointer(new Converter());
  }
  return _instance;
}
// ========================================================

Converter::Converter()
{
  _pdsTypeList.push_back(Pds::TypeId::Id_AcqConfig);
  _pdsTypeList.push_back(Pds::TypeId::Id_AcqWaveform);
}

void Converter::operator()(const Pds::Xtc* xtc, CASSEvent* evt)
{
  //check whether xtc is a configuration or a event//
  switch (xtc->contains.id())
  {
    //if it is a configuration then check for which Acqiris
  case (Pds::TypeId::Id_AcqConfig) :
    {
      //extract the detectorinfo//
      const Pds::DetInfo& info = *(Pds::DetInfo*)(&xtc->src);
      //we need to make sure that only instruments we know of are present//
      assert(static_cast<int>(info.detector()) == static_cast<int>(Camp1) ||
             static_cast<int>(info.detector()) == static_cast<int>(Camp2) ||
             static_cast<int>(info.detector()) == static_cast<int>(Camp3) ||
             static_cast<int>(info.detector()) == static_cast<int>(Camp4) ||
             static_cast<int>(info.detector()) == static_cast<int>(XPP));
      //retrieve a reference to the nbr of Channels for this instrument//
      size_t &nbrChannels = _numberOfChannels[info.detector()];
      //make sure the number is smaller than 20, which is the maximum nbr of channels//
      assert(nbrChannels <= 20);
      unsigned version = xtc->contains.version();
      switch (version)
      {
        //if it is the right configuration then initialize the storedevent with the configuration//
      case 1:
        {
          //get the config//
          const Pds::Acqiris::ConfigV1 &config =
              *reinterpret_cast<const Pds::Acqiris::ConfigV1*>(xtc->payload());
          //extract how many channels are in the acqiris device//
          nbrChannels = config.nbrChannels();
          Log::add(Log::VERBOSEINFO, "AcqirisConverter::ConfigXtc: Instrument " +
              toString(info.detector()) + " has " + toString(config.nbrChannels()) +
              " Channels");
        }
        break;
      default:
        throw runtime_error("Unsupported acqiris configuration version '" +
                            toString(version) + "'");
        break;
      }
    }
    break;


    //if it is a event then extract all information from the event//
  case (Pds::TypeId::Id_AcqWaveform):
    {
      //extract the detectorinfo//
      const Pds::DetInfo& info = *(Pds::DetInfo*)(&xtc->src);
      //we need to make sure that only instruments we know of are present//
      assert(static_cast<int>(info.detector()) == static_cast<int>(Camp1) ||
             static_cast<int>(info.detector()) == static_cast<int>(Camp2) ||
             static_cast<int>(info.detector()) == static_cast<int>(Camp3) ||
             static_cast<int>(info.detector()) == static_cast<int>(Camp4) ||
             static_cast<int>(info.detector()) == static_cast<int>(XPP));
      //retrieve  the nbr of Channels for this instrument//
      const size_t nbrChannels = _numberOfChannels[info.detector()];
      //make sure the number is smaller than 20
      assert(nbrChannels <= 20);
      //extract the datadescriptor (waveform etc) from the xtc//
      const Pds::Acqiris::DataDescV1 *datadesc =
          reinterpret_cast<const Pds::Acqiris::DataDescV1*>(xtc->payload());
      //retrieve a pointer to the right acqiris instrument//
      Device &dev =
          dynamic_cast<Device&>(*(evt->devices()[CASSEvent::Acqiris]));
      //retrieve a reference to the right instrument//
      Instrument & instr = dev.instruments()[info.detector()];
      //retrieve a reference to the channel container of the instrument//
      Instrument::channels_t &channels = instr.channels();
      //resize the channel vector to how many channels are in the device//
      channels.resize(nbrChannels);
      //initialize the channel values from the datadescriptor//
      for (Instrument::channels_t::iterator it=channels.begin();
          it != channels.end();
          ++it)
      {
        //retrieve a reference instead of a pointer//
        const Pds::Acqiris::DataDescV1 &dd = *datadesc;
        //retrieve a reference to the channel we are working on//
        Channel &chan         = *it;
        //extract the infos from the datadesc//
        chan.channelNbr()     = 99;
        chan.horpos()         = dd.timestamp(0).horPos();
        chan.offset()         = dd.offset();
        chan.gain()           = dd.gain();
        chan.sampleInterval() = dd.sampleInterval();
        //extract waveform from the datadescriptor//
        const short* waveform = dd.waveform();
        //we need to shift the pointer so that it looks at the first real point of the waveform//
        waveform += dd.indexFirstPoint();
        //retrieve a reference to our waveform//
        Channel::waveform_t &mywaveform = chan.waveform();
        //resize our waveform vector to hold all the entries of the waveform//
        mywaveform.resize(dd.nbrSamplesInSeg());
//        std::cout <<"AcqirisConverter: "
//            <<"gain:"<<dd.gain()<<" "
//            <<"nbrSamples:"<<dd.nbrSamplesInSeg()<<" "
//            <<"idxFiPoint:"<<dd.indexFirstPoint()<< " "
//            <<"offset:"<<dd.offset()<< " "
//            <<"sampInter:"<<dd.sampleInterval()<< " "
//            <<std::endl;
//        printf("*** %e %d %d %e %e\n",dd.gain(),dd.nbrSamplesInSeg(),dd.indexFirstPoint(),dd.offset(),dd.sampleInterval());
        //copy the datapoints of the waveform//
        //we have to swap the byte order for some reason that still has to be determined//
        for (size_t iWave=0;iWave<dd.nbrSamplesInSeg();++iWave)
          mywaveform[iWave] = (waveform[iWave]&0x00ff<<8) | (waveform[iWave]&0xff00>>8);

        //change to the next Channel//
        datadesc = datadesc->nextChannel();
      }
    }
    break;
  default:
    throw logic_error(string("ACQIRIS::Converter(): xtc type '") +
                      Pds::TypeId::name(xtc->contains.id()) +
                      "' is not handled by Acqiris Converter");
    break;
  }
}
