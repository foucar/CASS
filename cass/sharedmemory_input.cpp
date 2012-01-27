// Copyright (C) 2009,2010 Lutz Foucar

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


using namespace cass;
using namespace std;

void SharedMemoryInput::instance(const string &partitionTag,
                                 int index,
                                 RingBuffer<CASSEvent,RingBufferSize>& ringbuffer,
                                 Ratemeter &ratemeter,
                                 QObject *parent)
{
  if(_instance)
    throw logic_error("SharedMemoryInput::instance(): The instance of the base class is already initialized");
  _instance = shared_pointer(new SharedMemoryInput(partitionTag,
                                                   index,
                                                   ringbuffer,
                                                   ratemeter,
                                                   parent));
}

SharedMemoryInput::SharedMemoryInput(const string &partitionTag,
                                     int index,
                                     RingBuffer<CASSEvent,RingBufferSize>& ringbuffer,
                                     Ratemeter &ratemeter,
                                     QObject *parent)
  :InputBase(ringbuffer,ratemeter,parent),
    _partitionTag(partitionTag),
    _index(index),
    _convert(*FormatConverter::instance())
{
//  loadSettings(0);
  load();
}

void SharedMemoryInput::load()
{
  _convert.loadSettings(0);
}

void SharedMemoryInput::run()
{
  _status = lmf::PausableThread::running;
  VERBOSEOUT(cout << "starting shared memory in put with partition Tag: \""
      <<_partitionTag <<"\""
      << " and Client Index "<< _index<<endl);
  Pds::XtcMonitorClient::run(_partitionTag.c_str(),_index,_index,2);
  VERBOSEOUT(cout << "shared memory input is closing down"<<endl);
}

void SharedMemoryInput::end()
{
  VERBOSEOUT(cout << "SharedMemoryInput::end(): got signal to close"<<endl);
  _control = _quit;
  VERBOSEOUT(cout << "SharedMemoryInput::end(): wait for 5 s that shared memory"
             << " shuts down"<<endl);
  if(!wait(5000))
  {
    VERBOSEOUT(cout << "SharedMemoryInput::end(): time has elapsed. So we"
        <<" probably lost connection to the shared memory. Therefore we will"
        <<" terminate the thread"<<endl);
    terminate();
  }
  else
  {
    VERBOSEOUT(cout << "Ok. Shared Memory input thread has shut down within 5 s"<<endl);
  }
}

int SharedMemoryInput::processDgram(Pds::Dgram* datagram)
{
  pausePoint();

  //check if it just timed out, if so return
  if(!datagram)
    return (_control == _quit);

  //make a pointer to a element in the ringbuffer//
  CASSEvent *cassevent(0);

  //retrieve a new element from the ringbuffer//
  _ringbuffer.nextToFill(cassevent);

  //read the datagram to the ringbuffer//
  Pds::Dgram& dg = *reinterpret_cast<Pds::Dgram*>(cassevent->datagrambuffer());
  memcpy(&dg,datagram,sizeof(Pds::Dgram));
  if (datagram->xtc.sizeofPayload() > static_cast<int>(DatagramBufferSize))
  {
    cout << "datagram size is bigger than the maximum buffer size of "
         <<DatagramBufferSize/1024/1024
         <<" MB. Something is wrong. Skipping the datagram"
         <<endl;
    return  _control == _quit;
  }
  memcpy(dg.xtc.payload(),datagram+1,datagram->xtc.sizeofPayload());

  //now convert the datagram to a cassevent//
  const bool isGood = _convert(cassevent);

  //tell the buffer that we are done, but also let it know whether it is a good event//
  _ringbuffer.doneFilling(cassevent,isGood);

  //for ratemeter purposes send a signal that we added a new event//
  newEventAdded();

  //return the quit code//
  return  _control == _quit;
}


// Local Variables:
// coding: utf-8
// mode: C++
// c-file-offsets: ((c . 0) (innamespace . 0))
// c-file-style: "Stroustrup"
// fill-column: 100
// End:
