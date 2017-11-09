// Copyright (C) 2017 Lutz Foucar

/**
 * @file zmq_input.cpp contains input that uses ZMQ as interface
 *
 * @author Lutz Foucar
 */

#include <iostream>
#include <zmq.hpp>
#include <msgpack.hpp>

#include "zmq_input.h"

#include "cass_settings.h"
#include "cass_exceptions.hpp"
#include "log.h"

#include "acqiris_device.hpp"
#include "pixeldetector.hpp"
#include "machine_device.hpp"

using namespace cass;
using namespace std;


void ZMQInput::instance(RingBuffer<CASSEvent> &buffer,
                   Ratemeter &ratemeter,
                   Ratemeter &loadmeter,
                   bool quitwhendone,
                   QObject *parent)
{
  if(_instance)
    throw logic_error("ZMQInput::instance(): The instance of the base class is already initialized");
  _instance = shared_pointer(new ZMQInput(buffer,ratemeter,loadmeter,quitwhendone,parent));
}

ZMQInput::ZMQInput(RingBuffer<CASSEvent> &ringbuffer,
                   Ratemeter &ratemeter,
                   Ratemeter &loadmeter,
                   bool quitwhendone,
                   QObject *parent)
  : InputBase(ringbuffer,ratemeter,loadmeter,parent),
    _quitWhenDone(quitwhendone),
    _counter(0),
    _scounter(0)
{}

struct Info
{
  Info():CASSID(0) {}
  void clear()
  {
    data.clear();
    shape.clear();
  }

  bool isPerTrain;
  std::string CASSValueName;
  std::string CASSDeviceType;
  int CASSID;
  std::vector<float> data;
  std::vector<int> shape;
};

typedef std::map<std::string,Info> extractmap_t;


/** read the string like payload of an msgpack object into an vector of floats
 *
 * @tparam type the type of data wrapped in the msgpack object as string
 * @param out reference to the vector where the data will be written to
 * @param obj the msgpack object who's payload will be written to the out vector
 *
 * @author Lutz Foucar
 */
template <typename type>
void readNDArrayDataAsString(std::vector<float>& out, const msgpack::object &obj)
{
  const type *d(reinterpret_cast<const type*>(obj.via.str.ptr));
  size_t payloadsize(obj.via.str.size);
  out.assign(d,d+payloadsize/sizeof(type));
}

/** read the binary payload of an msgpack object into an vector of floats
 *
 * @tparam type the type of data wrapped in the msgpack object as string
 * @param out reference to the vector where the data will be written to
 * @param obj the msgpack object who's payload will be written to the out vector
 *
 * @author Lutz Foucar
 */
template <typename type>
void readNDArrayDataAsBinary(std::vector<float>& out, const msgpack::object &obj)
{
  const type *d(reinterpret_cast<const type*>(obj.via.bin.ptr));
  size_t payloadsize(obj.via.bin.size);
  out.assign(d,d+payloadsize/sizeof(type));
}

bool iterate(const msgpack::object &obj, int depth,
             extractmap_t& emap, string acckey="")
{
  typedef map<string,msgpack::object> m_t;
  m_t m(obj.as<m_t>());

  /** just go through the msgpack object and */
  for (m_t::iterator it(m.begin()); it!= m.end();++it)
  {
//    for (extractmap_t::const_iterator eit(emap.begin()); eit != emap.end(); ++eit)
//       cout << eit->first << " " << eit->second.shape.size()<<endl;
    string flattenkey(acckey);
    /** separate the keys of the nested dictionaries by a '$' character */
    if(!flattenkey.empty())
      flattenkey.append("$");
    flattenkey.append(it->first);
    /** check if we're interested in the data */
    if ((emap.find(flattenkey) != emap.end()))
    {
      //cout << flattenkey <<" found it!!!"<<endl;
      /** get a reference to the info that will now attempt to fill. */
      Info &info(emap[flattenkey]);
      /** check if its ndarray data */
      if (it->second.type == msgpack::type::MAP)
      {
        m_t mp(it->second.as<m_t>());
        if ((mp.find("nd") != m.end()) &&
            (mp["nd"].type == msgpack::type::BOOLEAN) &&
            (mp["nd"].as<bool>()) &&
            (mp.find("data") != m.end()) &&
            (mp.find("type") != m.end()) &&
            (mp["type"].type == msgpack::type::STR) &&
            (mp.find("shape") != m.end()) &&
            (mp["shape"].type == msgpack::type::ARRAY))
        {
          /** if it is then extract the shape and the data according to the
           *  type of the data an how it is packed
           */
          mp["shape"].convert(info.shape);
          //cout <<flattenkey<< " "<<info.shape.size()<<endl;
          if (mp["data"].type == msgpack::type::STR)
          {
            if (mp["type"].as<string>() == "<f4")
            {
              readNDArrayDataAsString<float>(info.data,mp["data"]);
            }
            else if (mp["type"].as<string>() == "<f8")
            {
              readNDArrayDataAsString<double>(info.data,mp["data"]);
            }
            else if (mp["type"].as<string>() == "<u8")
            {
              readNDArrayDataAsString<uint64_t>(info.data,mp["data"]);
            }
            else if (mp["type"].as<string>() == "<u2")
            {
              readNDArrayDataAsString<uint16_t>(info.data,mp["data"]);
            }
            else if (mp["type"].as<string>() == "|u1")
            {
              readNDArrayDataAsString<uint8_t>(info.data,mp["data"]);
            }
            else
            {
              Log::add(Log::WARNING,"ZMQInput::ParseMSGPACKObject: '" + flattenkey +
                       "': The type '" + (mp["type"].as<string>()) +
                       "' of the string type ndarray data is not " +
                       "supported.");
            }
          }
          else if (m["data"].type == msgpack::type::BIN)
          {
            if (mp["type"].as<string>() == "<f4")
            {
              readNDArrayDataAsBinary<float>(info.data,mp["data"]);
            }
            else if (mp["type"].as<string>() == "<f8")
            {
              readNDArrayDataAsBinary<double>(info.data,mp["data"]);
            }
            else if (mp["type"].as<string>() == "<u8")
            {
              readNDArrayDataAsBinary<uint64_t>(info.data,mp["data"]);
            }
            else if (mp["type"].as<string>() == "<u2")
            {
              readNDArrayDataAsBinary<uint16_t>(info.data,mp["data"]);
            }
            else if (mp["type"].as<string>() == "|u1")
            {
              readNDArrayDataAsBinary<uint8_t>(info.data,mp["data"]);
            }
            else
            {
              Log::add(Log::WARNING,"ZMQInput::ParseMSGPACKObject: '" + flattenkey +
                       "': The type '" + (mp["type"].as<string>()) +
                       "' of the binary type ndarray data is not " +
                       "supported.");
            }
          }
          //cout << flattenkey << " " << info.data.size() <<endl;
        }
      } // end parsing the ndarray type data

      /** check if its an array */
      if (it->second.type == msgpack::type::ARRAY)
      {
        it->second.convert(info.data);
      }

      /** check if its a single data value */
      if (it->second.type == msgpack::type::BOOLEAN ||
          it->second.type == msgpack::type::FLOAT32 ||
          it->second.type == msgpack::type::FLOAT64 ||
          it->second.type == msgpack::type::POSITIVE_INTEGER ||
          it->second.type == msgpack::type::NEGATIVE_INTEGER)
      {
        info.data.resize(1);
        it->second.convert(info.data.front());
      }

    }//end info found in extraction map

    /** if we are not interested in the data then just check to see if its
     *  another map that we should iterate into
     */
    else if (it->second.type == msgpack::type::MAP)
    {
      iterate(it->second,depth+1,emap,flattenkey);
    }
  }

  return true;
}

void ZMQInput::runthis()
{
  _status = lmf::PausableThread::running;

  CASSSettings s;
  s.beginGroup("ZMQInput");
  string functiontype(s.value("DataType","agat").toString().toStdString());
  string serverAddress(s.value("ServerAddress","tcp://53.104.0.52:10000").toString().toStdString());

  extractmap_t emap;
  int size = s.beginReadArray("DataFields");
  for (int i = 0; i < size; ++i)
  {
    s.setArrayIndex(i);
    string key(s.value("Name","BAD").toString().toStdString());
    if (key == "BAD")
      continue;
    int cassid = (s.value("CASSID",0).toInt());
    if (cassid < 0)
      continue;
    string dev(s.value("DeviceType","PixelDetector").toString().toLower().toStdString());
    if ((dev != "pixeldetector") && (dev != "machinedata"))
    {
      Log::add(Log::INFO,"ZMQInput: DeviceType '" + dev + "' of DataField '" +
               key + "' is unkown");
      continue;
    }
    const string valname(s.value("CASSValueName","Unused").toString().toStdString());
    const bool perTrain(s.value("IsPerTrain",false).toBool());
    emap[key].CASSID = cassid;
    emap[key].CASSDeviceType = dev;
    emap[key].CASSValueName = valname;
    emap[key].isPerTrain = perTrain;
  }
  s.endArray();//DataFields
  s.endGroup();//ZMQInput

  /** connect to the zmq socket */
  zmq::context_t context (1);
  zmq::socket_t sock (context, ZMQ_SUB);
  sock.connect(serverAddress);
  sock.setsockopt(ZMQ_SUBSCRIBE, "",0);
  Log::add(Log::INFO,"ZMQInput: Connecting to '" + serverAddress + "'");
  string output = "ZMQInput: Trying to retrieve:";
  for (extractmap_t::const_iterator eit(emap.begin()); eit != emap.end(); ++eit)
  {
    output += " DataField '" + eit->first + "'";
    output += " (";
    output += "CASSID '" + toString(eit->second.CASSID) + "'";
    output += "; DeviceType '" + eit->second.CASSDeviceType + "'";
    output += "; ValueName '" + eit->second.CASSValueName + "'";
    output += ");";
  }
  Log::add(Log::INFO,output);

  /** run until the thread is told to quit */
  Log::add(Log::DEBUG0,"ZMQInput::run(): starting loop");

  while(!shouldQuit())
  {
    /** here we can safely pause the execution */
    pausePoint();

    /** now retrive new data from the socket */
    zmq::message_t mess;
    sock.recv(&mess);

    /** now deserialize the data from the socket */
    size_t off(0);
    bool success(false);
    while(off != mess.size())
    {
      msgpack::object_handle objH;
      msgpack::unpack(objH,static_cast<const char*>(mess.data()),mess.size(),off);
      msgpack::object obj(objH.get());

      /** clear the info container */
      extractmap_t::iterator emIter(emap.begin());
      extractmap_t::const_iterator emIterEnd(emap.end());
      for (; emIter != emIterEnd; ++emIter)
        emIter->second.clear();
      success=iterate(obj,0,emap);
    }
    if (!success)
      continue;

    /** we got detector data from the whole train, so we have to extract the
     *  data bunch by bunch and add that to the ringbuffer
     */
    extractmap_t::const_iterator emapIt(emap.begin());
    //cout << emapIt->second.shape.size() <<endl;
    for(;emapIt != emap.end(); ++emapIt)
      if (emapIt->second.shape.size() == 4)
        break;
    if (emapIt == emap.end())
    {
      cout << "can't find the number of bunches of this train" << endl;
      continue;
    }
    const Info& ifo(emapIt->second);
    const uint32_t nCols(ifo.shape[3]);
    const uint32_t nRows(ifo.shape[2]);
    const uint32_t nTiles(ifo.shape[1]);
    const uint32_t nBunches(ifo.shape[0]);
//    cout << "ncols:"<<nCols <<" nRows:"<<nRows<< " nTiles"<<nTiles<<
//            " nBunches"<<nBunches<<endl;
    /** how many pixels has a detector */
    const size_t nPixels(nCols*nRows*nTiles);
    for (size_t i(0); i< nBunches; ++i)
    {
      /** retrieve a new element from the ringbuffer, continue with next iteration
       *  in case the retrieved element is the iterator to the last element of the
       *  buffer.
       */
      rbItem_t rbItem(getNextFillable());
      if (rbItem == _ringbuffer.end())
        continue;
      CASSEvent &evt(*rbItem->element);
      evt.id() = _counter;

      /** get reference to all devices of the CASSEvent and an iterator*/
      CASSEvent::devices_t &devices(evt.devices());
      CASSEvent::devices_t::iterator devIt;
      /** go through the list of requested infos an put them in to the event */
      extractmap_t::const_iterator eIt(emap.begin());
      extractmap_t::const_iterator eEnd(emap.end());
      for (; eIt != eEnd; ++eIt)
      {
        /** check if the requested data was sent */
        if (eIt->second.data.empty())
        {
          Log::add(Log::WARNING,string("ZMQInput: There is no data for ") +
                   "datafield '" + eIt->first + "'");
          continue;
        }
        if (eIt->second.CASSDeviceType == "pixeldetector")
        {
          string outp = eIt->first + " [";
          for (size_t ii(0); ii < eIt->second.shape.size(); ++ii)
            outp += toString(eIt->second.shape[ii]) + ",";
          outp.replace(outp.size()-1,1,"]");
          Log::add(Log::DEBUG0,outp);
          /** retrieve the pixel detector part of the cassevent */
          devIt = devices.find(CASSEvent::PixelDetectors);
          if(devIt == devices.end())
            throw runtime_error(string("ZMQInput: CASSEvent does not ") +
                                       "contain a pixeldetector device");
          pixeldetector::Device &pixdev (dynamic_cast<pixeldetector::Device&>(*(devIt->second)));
          /** retrieve the right detector from the cassevent and reset it*/
          pixeldetector::Detector &det(pixdev.dets()[eIt->second.CASSID]);
          det.frame().clear();
          /** get iterator to the corresponding data and advance it to the right
           *  pulse within the train
           */
          pixeldetector::Detector::frame_t::const_iterator detBegin(eIt->second.data.begin());
          advance(detBegin,i*nPixels);
          /** copy the det data to the frame */
          det.frame().assign(detBegin,detBegin+(nPixels));
          det.columns() = nCols;
          det.rows() = nRows*nTiles;
          det.id() = _counter;
        }
        else if (eIt->second.CASSDeviceType == "machinedata")
        {
          /** retrieve the pixel detector part of the cassevent */
          devIt = devices.find(CASSEvent::MachineData);
          if(devIt == devices.end())
            throw runtime_error(string("ZMQInput: CASSEvent does not ") +
                                       "contain a pixeldetector device");
          MachineData::Device &md (dynamic_cast<MachineData::Device&>(*(devIt->second)));
          if (eIt->second.isPerTrain)
            md.BeamlineData()[eIt->second.CASSValueName] = eIt->second.data[0];
          else
            md.BeamlineData()[eIt->second.CASSValueName] = eIt->second.data[i];
        }
      }

      /** tell the ringbuffer that we're done with the event */
      ++_counter;
      _ringbuffer.doneFilling(rbItem, 1);
    }
    newEventAdded(mess.size());
  }
  Log::add(Log::INFO,"ZMQInput::run(): Quitting loop");
}
