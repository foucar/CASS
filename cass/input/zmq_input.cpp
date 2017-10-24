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
    _quitWhenDone(quitwhendone)
{}

struct myvisitor : msgpack::v2::null_visitor
{
  bool visit_str(const char *str, uint32_t size)
  {
    cout << string(str,size) << endl;
    return true;
  }
};

typedef map<string,msgpack::object> m_t;
bool iterate(msgpack::object &obj, int depth,
             vector<float>&data, vector<int>&shape,
             const string& searchKey, string acckey="")
{
  //cout << acckey << " " <<searchKey<< " " <<(acckey == searchKey)<< endl;
  m_t m(obj.as<m_t>());
  /** when we've got down to the point where we see detector data,
   *  check if it is really the detector data
   */
  if ((searchKey == acckey) &&
      (m.find("nd") != m.end()) && (m["nd"].type == msgpack::type::BOOLEAN) &&
      (m["nd"].as<bool>()) &&
      (m.find("data") != m.end()) && (m["data"].type == msgpack::type::STR) &&
      (m.find("type") != m.end()) && (m["type"].type == msgpack::type::STR) &&
      (m.find("shape") != m.end()) && (m["shape"].type == msgpack::type::ARRAY) )
  {
    cout <<m["type"].as<string>() << " HUHU" << endl;
    m["shape"].convert(shape);
    if (m["type"].as<string>() == "<f4")
    {
      size_t cssize(m["data"].via.str.size);
      const float *d(reinterpret_cast<const float*>(m["data"].via.str.ptr));
      data.assign(d,d+cssize/4);
    }
    return true;
  }

  /** just go through the msgpack object and iterate down to the nested maps */
  for (m_t::iterator it(m.begin()); it!= m.end();++it)
  {
    string flattenkey(acckey);
    /** separate the keys of the nested dictionaries by a '$' character */
    if(!flattenkey.empty())
      flattenkey.append("$");
    flattenkey.append(it->first);
    if (it->second.type == msgpack::type::MAP)
    {
      iterate(it->second,depth+1,data,shape,searchKey,flattenkey);
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
  string detKey(s.value("DetectorKey","").toString().toStdString());
  int CASSID(s.value("CASSID",30).toInt());
  s.endGroup();

  /** connect to the zmq socket */
  zmq::context_t context (1);
  zmq::socket_t sock (context, ZMQ_SUB);
  sock.connect(serverAddress);
  sock.setsockopt(ZMQ_SUBSCRIBE, "",0);
  cout << serverAddress << endl;
  cout << detKey << endl;

  /** run until the thread is told to quit */
  Log::add(Log::DEBUG0,"ZMQInput::run(): starting loop");

  /** the data to be retrieved */
  vector<float> detData;
  vector<int> detDataShape;
  uint64_t id(0);

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

      success=iterate(obj,0,detData,detDataShape,detKey);
    }
    if (!success)
      continue;

    /** we got detector data from the whole train, so we have to extract the
     *  data bunch by bunch and add that to the ringbuffer
     */
    const uint32_t nCols(detDataShape[3]);
    const uint32_t nRows(detDataShape[2]);
    const uint32_t nTiles(detDataShape[1]);
    //const uint32_t nBunches(detDataShape[0]);
    //cout << "ncols:"<<nCols <<" nRows:"<<nRows<< " nTiles"<<nTiles<<endl;
    vector<float>::iterator detBegin(detData.begin());
    while(detBegin != detData.end())
    {
      /** generate and set variable to keep the size of the retrieved data */
      uint64_t datasize(nCols*nRows*nTiles*4);

      /** retrieve a new element from the ringbuffer, continue with next iteration
       *  in case the retrieved element is the iterator to the last element of the
       *  buffer.
       */
      rbItem_t rbItem(getNextFillable());
      if (rbItem == _ringbuffer.end())
        continue;
      CASSEvent &evt(*rbItem->element);

      /** get reference to all devices of the CASSEvent and an iterator*/
      CASSEvent::devices_t &devices(evt.devices());
      CASSEvent::devices_t::iterator devIt;
      /** retrieve the pixel detector part of the cassevent */
      devIt = devices.find(CASSEvent::PixelDetectors);
      if(devIt == devices.end())
        throw runtime_error("ZMQInput: CASSEvent does not contains a pixeldetector device");
      pixeldetector::Device &pixdev (dynamic_cast<pixeldetector::Device&>(*(devIt->second)));
      /** retrieve the right detector from the cassevent and reset it*/
      pixeldetector::Detector &det(pixdev.dets()[CASSID]);
      det.frame().clear();
      /** copy the det data to the frame */
      det.frame().assign(detBegin,detBegin+(nCols*nRows*nTiles));
      //cout << *detBegin << " "<<det.frame()[0]<< " "<< id<<endl;
      det.columns() = nCols;
      det.rows() = nRows*nTiles;
      det.id() = ++id;
      evt.id() = id;
      /** now advance the iterator to the next image in the train */
      detBegin += (nCols*nRows*nTiles);

      /** tell the ringbuffer that we're done with the event */
      newEventAdded(datasize);
      _ringbuffer.doneFilling(rbItem, datasize);
    }
  }
  Log::add(Log::INFO,"ZMQInput::run(): Quitting loop");
}
