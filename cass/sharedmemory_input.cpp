// Copyright (C) 2009 LMF

#include <iostream>
#include <iomanip>
#include "sharedmemory_input.h"
#include "format_converter.h"
#include "pdsdata/xtc/Dgram.hh"


cass::SharedMemoryInput::SharedMemoryInput(char * partitionTag, lmf::RingBuffer<cass::CASSEvent,4>& ringbuffer, QObject *parent)
       :QThread(parent),
        _ringbuffer(ringbuffer),
        _partitionTag(partitionTag),
        _quit(false),
        _converter(cass::FormatConverter::instance())
{
}

cass::SharedMemoryInput::~SharedMemoryInput()
{
}

void cass::SharedMemoryInput::run()
{
  //start the xtcmonitorclient//
  //this eventqueue will subscripe to a partitiontag//
  std::cout << "starting shared memory in put with partition Tag: \""<<_partitionTag <<"\""<<std::endl;
  Pds::XtcMonitorClient::run(_partitionTag);
  std::cout << "shared memory input is closing down"<<std::endl;
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
  if (datagram->xtc.sizeofPayload() > cass::DatagramBufferSize)
    std::cout << "datagram size is bigger than the maximum buffer size of 10 MB. Something is wrong"<<std::endl;
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
