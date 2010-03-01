//Copyright (C) 2010 lmf

#ifndef __EVENT_GETTER_H__
#define __EVENT_GETTER_H__

#include "cass.h"
#include "cass_event.h"
#include "tcpserver.h"
#include "ringbuffer.h"

namespace cass
{

  class CASSSHARED_EXPORT EventGetter
  {
  public:
    EventGetter(lmf::RingBuffer<cass::CASSEvent,cass::RingBufferSize>&);

  public:
      void operator()(const TCPserver::event_parameter&,bufferinputiterator_t&);
  private:
    lmf::RingBuffer<cass::CASSEvent,cass::RingBufferSize>  &_ringbuffer;

  };
}//end namespace cass

#endif
