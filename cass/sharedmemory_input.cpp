// Copyright (C) 2009 LMF

#include <iostream>
#include <iomanip>
#include "sharedmemory_input.h"
#include "pdsdata/xtc/Dgram.hh"

cass::SharedMemoryInput::SharedMemoryInput(char * partitionTag, lmf::RingBuffer<cass::CASSEvent,4>& ringbuffer, QObject *parent)
       :QThread(parent),
        _ringbuffer(ringbuffer),
        _partitionTag(partitionTag),
        _quit(false)
{
}

cass::SharedMemoryInput::~SharedMemoryInput()
{
}

void cass::SharedMemoryInput::run()
{
  //start the xtcmonitorclient//
  //this eventqueue will subscripe to a partitiontag//
  std::cout << "starting with partition Tag: \""<<_partitionTag <<"\""<<std::endl;
  Pds::XtcMonitorClient::run(_partitionTag);
  std::cout << "shared memory is closing down"<<std::endl;
}

void cass::SharedMemoryInput::end()
{
  std::cout << "shared memory input got signal to close"<<std::endl;
  _quit=true;
}

int cass::SharedMemoryInput::processDgram(Pds::Dgram* datagram)
{
  //make a pointer to a element in the ringbuffer//
  cass::CASSEvent *cassevent;

  //retrieve a new element from the ringbuffer//
  _ringbuffer.nextToFill(cassevent);

  //read the datagram to the ringbuffer//
  Pds::Dgram& dg = *reinterpret_cast<Pds::Dgram*>(cassevent->datagrambuffer());
  memcpy(&dg,datagram,sizeof(Pds::Dgram));
  memcpy(dg.xtc.payload(),datagram+1,datagram->xtc.sizeofPayload());

  //tell the buffer that we are done//
  _ringbuffer.doneFilling(cassevent);
  
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
