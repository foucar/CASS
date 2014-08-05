// Copyright (C) 2009,2010,2013 Lutz Foucar

/**
 * @file sharedmemory_input.cpp file contains definition of class that interfaces
 *                              the LCLS shared memory
 *
 * @author Lutz Foucar
 */

#include <QtCore/QMutexLocker>

#include <iostream>
#include <iomanip>
#include "sharedmemory_input.h"
#include "format_converter.h"
#include "pdsdata/xtc/Dgram.hh"
#include "log.h"


using namespace cass;
using namespace std;

void SharedMemoryInput::instance(const string &partitionTag,
                                 int index,
                                 cass::RingBuffer<CASSEvent> &ringbuffer,
                                 Ratemeter &ratemeter,
                                 Ratemeter &loadmeter,
                                 QObject *parent)
{
  if(_instance)
    throw logic_error("SharedMemoryInput::instance(): The instance of the base class is already initialized");
  _instance = shared_pointer(new SharedMemoryInput(partitionTag,
                                                   index,
                                                   ringbuffer,
                                                   ratemeter,
                                                   loadmeter,
                                                   parent));
}

SharedMemoryInput::SharedMemoryInput(const string &partitionTag,
                                     int index,
                                     RingBuffer<CASSEvent>& ringbuffer,
                                     Ratemeter &ratemeter,
                                     Ratemeter &loadmeter,
                                     QObject *parent)
  : InputBase(ringbuffer,ratemeter,loadmeter,parent),
    _partitionTag(partitionTag),
    _index(index),
    _convert(*FormatConverter::instance())
{
  load();
}

void SharedMemoryInput::load()
{
  _convert.loadSettings(0);
}

void SharedMemoryInput::run()
{
  _status = lmf::PausableThread::running;
  Log::add(Log::DEBUG0,"SharedMemoryInput::run(): starting shared memory in put with partition Tag: '" +
      _partitionTag + "' and Client Index " + toString(_index));
  Pds::XtcMonitorClient::run(_partitionTag.c_str(),_index,_index);
  Log::add(Log::DEBUG0,"SharedMemoryInput::run(): shared memory input is closing down");
}

void SharedMemoryInput::end()
{
  Log::add(Log::DEBUG0,"SharedMemoryInput::end(): got signal to close");
  _control = _quit;
  Log::add(Log::DEBUG0,"SharedMemoryInput::end(): wait for 5 s that shared memory shuts down");
  if(!wait(5000))
  {
    Log::add(Log::DEBUG0,string("SharedMemoryInput::end(): time has elapsed. So we") +
        " probably lost connection to the shared memory. Therefore we will" +
        " terminate the thread");
    terminate();
  }
  else
  {
    Log::add(Log::DEBUG0,"SharedMemoryInput::end(): Ok. Shared Memory input thread has shut down within 5 s");
  }
}

int SharedMemoryInput::processDgram(Pds::Dgram* datagram)
{
  pausePoint();

  //check if it just timed out, if so return
  if(!datagram)
    return (_control == _quit);

  //retrieve a new element from the ringbuffer//
  rbItem_t rbItem(_ringbuffer.nextToFill());

  //read the datagram to the ringbuffer//
  CASSEvent::buffer_t& buf(rbItem->element->datagrambuffer());
  buf.assign(reinterpret_cast<CASSEvent::buffer_t::value_type*>(datagram),
             reinterpret_cast<CASSEvent::buffer_t::value_type*>(datagram)+(sizeof(Pds::Dgram)+datagram->xtc.sizeofPayload()));
  if (datagram->xtc.sizeofPayload() > static_cast<int>(DatagramBufferSize))
  {
    Log::add(Log::WARNING,string("SharedMemoryInput::processDgram(): Datagram size is bigger ") +
             "than the maximum buffer size of " + toString(DatagramBufferSize/1024/1024) +
             " MB. Something is wrong. Skipping the datagram");
    return  _control == _quit;
  }
//  memcpy(dg.xtc.payload(),datagram+1,datagram->xtc.sizeofPayload());

  //now convert the datagram to a cassevent//
  const bool isGood = _convert(rbItem->element);

  //tell the buffer that we are done, but also let it know whether it is a good event//
  _ringbuffer.doneFilling(rbItem,isGood);

  //for ratemeter purposes send a signal that we added a new event//
  newEventAdded(rbItem->element->datagrambuffer().size());

  //return the quit code//
  return  _control == _quit;
}
