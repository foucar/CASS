// Copyright (C) 2009,2010 LMF

#include <QMutexLocker>

#include <iostream>
#include <iomanip>
#include "sharedmemory_input.h"
#include "format_converter.h"
#include "pdsdata/xtc/Dgram.hh"


cass::SharedMemoryInput::SharedMemoryInput(char * partitionTag,
                                           cass::RingBuffer<cass::CASSEvent,cass::RingBufferSize>& ringbuffer,
                                           QObject *parent)
                                             :QThread(parent),
                                             _ringbuffer(ringbuffer),
                                             _partitionTag(partitionTag),
                                             _quit(false),
                                             _converter(cass::FormatConverter::instance()),
                                             _pause(false),
                                             _paused(false)
{
}

cass::SharedMemoryInput::~SharedMemoryInput()
{
  std::cout<<"deleting shared memory input"<<std::endl;
  _converter->destroy();
  std::cout<<"shared memory input is deleted"<<std::endl;
}


void cass::SharedMemoryInput::loadSettings(size_t what)
{
  //pause yourselve//
  std::cout << "Shared Memory Input: Load Settings: suspend before laoding settings"
      <<std::endl;
  suspend();
  //load settings//
  std::cout << "Shared Memory Input: Load Settings: suspended. Now loading Settings"
      <<std::endl;
  _converter->loadSettings(what);
  //resume yourselve//
  std::cout << "Shared Memory Input: Load Settings: Done loading Settings. Now Resuming Thread"
      <<std::endl;
  resume();
  std::cout << "Shared Memory Input: Load Settings: thread is resumed"
      <<std::endl;
}

void cass::SharedMemoryInput::suspend()
{
  _pause=true;
  //wait until you are paused//
  waitUntilSuspended();
}

void cass::SharedMemoryInput::resume()
{
  //if the thread has not been paused return here//
  if(!_pause)
    return;
  //reset the pause flag;
  _pause=false;
  //tell run to resume via the waitcondition//
  _pauseCondition.wakeOne();
}

void cass::SharedMemoryInput::waitUntilSuspended()
{
  //if it is already paused then retrun imidiatly//
  if(_paused)
    return;
  //otherwise wait until the conditions has been called//
  QMutex mutex;
  QMutexLocker lock(&mutex);
  _waitUntilpausedCondition.wait(&mutex);
}

void cass::SharedMemoryInput::run()
{
  //start the xtcmonitorclient//
  //this eventqueue will subscripe to a partitiontag//
  std::cout << "starting shared memory in put with partition Tag: \""
      <<_partitionTag <<"\""<<std::endl;
  Pds::XtcMonitorClient::run(_partitionTag);
  std::cout << "shared memory input is closing down"<<std::endl;
}

void cass::SharedMemoryInput::end()
{
  std::cout << "shared memory input got signal to close"<<std::endl;
  //tell the loop that it should quit
  _quit=true;
  //wait until we have finished, but only for 2 seconds//
  //if we were not finished by that time the we want to terminate//
  //ourselves//
  std::cout << "wait for 2 s that shared memory shuts down"<<std::endl;
  if(!wait(2000))
  {
    std::cout << "time has elapsed. So we probably lost connection to"
        <<"the shared memory. Therefore we will terminate the thread"<<std::endl;
    terminate();
  }
  std::cout << "Ok. Shared Memory input thread has shut down within 2 s"<<std::endl;
}

int cass::SharedMemoryInput::processDgram(Pds::Dgram* datagram)
{
  //pause execution if suspend has been called//
  if (_pause)
  {
    //lock the mutex to prevent that more than one thread is calling pause//
    _pauseMutex.lock();
    //set the status flag to paused//
    _paused=true;
    //tell the wait until paused condtion that we are now pausing//
    _waitUntilpausedCondition.wakeOne();
    //wait until the condition is called again
    _pauseCondition.wait(&_pauseMutex);
    //set the status flag//
    _paused=false;
    //unlock the mutex, such that others can work again//
    _pauseMutex.unlock();
  }

  //make a pointer to a element in the ringbuffer//
  cass::CASSEvent *cassevent;

  //retrieve a new element from the ringbuffer//
  _ringbuffer.nextToFill(cassevent);

  //read the datagram to the ringbuffer//
  Pds::Dgram& dg = *reinterpret_cast<Pds::Dgram*>(cassevent->datagrambuffer());
  memcpy(&dg,datagram,sizeof(Pds::Dgram));
  if (datagram->xtc.sizeofPayload() > static_cast<int>(cass::DatagramBufferSize))
    std::cout << "datagram size is bigger than the maximum buffer size of "
        <<cass::DatagramBufferSize/1024/1024<<" MB. Something is wrong"<<std::endl;
  memcpy(dg.xtc.payload(),datagram+1,datagram->xtc.sizeofPayload());

  //now convert the datagram to a cassevent//
  const bool isGood = _converter->processDatagram(cassevent);

  //tell the buffer that we are done, but also let it know whether it is a good event//
  _ringbuffer.doneFilling(cassevent,isGood);

  //for ratemeter purposes send a signal that we added a new event//
  emit newEventAdded();

  //return the quit code//
  return _quit;
}


// Local Variables:
// coding: utf-8
// mode: C++
// c-file-offsets: ((c . 0) (innamespace . 0))
// c-file-style: "Stroustrup"
// fill-column: 100
// End:
