// Copyright (C) 2010 Jochen Küpper

/**
 * @file event_getter.h file contains definition event retriever classes
 *
 * @author Lutz Foucar
 */

#include <string>

#include "event_getter.h"
#include "serializer.h"

using namespace cass;

EventGetter::EventGetter(RingBuffer<CASSEvent,RingBufferSize>& ringbuffer)
  :_ringbuffer(ringbuffer)
{}

const std::string EventGetter::operator()(const EventParameter& ep) const
{
  //retrieve a cassevent from the ringbuffer based on the parameter//
  CASSEvent *cassevent=0;

  //if what is 0 then retrieve the last event//
  if (ep.what==0)
    _ringbuffer.nextToView(cassevent,1000);
  //if all bits of what are set then we are looking for events in the given time range//
  else if(ep.what == 0xffffffffffffffff)
  {
    //search as many elements as there are in the buffer//
    for (size_t i=0;i<cass::RingBufferSize;++i)
    {
      //reset the cassevent
      cassevent=0;
      //retrieve the next event thats viewable
      _ringbuffer.nextToView(cassevent,1000);
      //check whether we retrieved a cassevent//
      if (cassevent)
      {
        //check whether the cassevent is in the requested timerange//
        //therefore we retrieve the upper 32 bits//
        uint32_t time = (cassevent->id() & 0xffffffff00000000) >> 32;
        //if the time is in the range we are looking for, stop searching
        if (ep.t1 <= time  && time < ep.t2 )
          break;
      }
    }
  }

  //create a serializer that will serialize the cassevent//
  Serializer serializer;
  //if cassevent is 0 there has been a timeout (no cassevent has been added)
  if (cassevent)
  {
    //serialize the cassevent to the buffer//
    cassevent->serialize(serializer);
    //tell the ringbuffer that we are done with the cassevent//
    _ringbuffer.doneViewing(cassevent);
  }
  //return the buffer (std::string of the serializer)
  return serializer.buffer();
}



// Local Variables:
// coding: utf-8
// mode: C++
// c-file-offsets: ((c . 0) (innamespace . 0))
// c-file-style: "Stroustrup"
// fill-column: 100
// End:
