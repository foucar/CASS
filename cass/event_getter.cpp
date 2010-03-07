#include "event_getter.h"
#include "serializer.h"


cass::EventGetter::EventGetter(lmf::RingBuffer<cass::CASSEvent,cass::RingBufferSize>& ringbuffer)
  :_ringbuffer(ringbuffer)
{
}

void cass::EventGetter::operator()(const TCP::EventParameter& ep, cass::Serializer& buffer)
{
  //retrieve a cassevent from the ringbuffer based on the parameter//
  CASSEvent *cassevent=0;
  _ringbuffer.nextToView(cassevent,1000);
  //if cassevent is 0 there has been a timeout (no cassevent has been added)
  if (cassevent)
  {
    //serialize the cassevent to the buffer//
    cassevent->serialize(buffer);
    //tell the ringbuffer that we are done with the cassevent//
    _ringbuffer.doneViewing(cassevent);
  }
}



// Local Variables:
// coding: utf-8
// mode: C++
// c-file-offsets: ((c . 0) (innamespace . 0))
// c-file-style: "Stroustrup"
// fill-column: 100
// End:
