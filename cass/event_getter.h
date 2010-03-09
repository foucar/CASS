//Copyright (C) 2010 lmf

#ifndef __EVENT_GETTER_H__
#define __EVENT_GETTER_H__

#include <string>

#include "cass.h"
#include "cass_event.h"
#include "tcpserver.h"
#include "ringbuffer.h"

namespace cass
{
  class Serializer;

  class CASSSHARED_EXPORT EventGetter
  {
  public:
    EventGetter(cass::RingBuffer<cass::CASSEvent,cass::RingBufferSize>&);

  public:
    const std::string operator()(const cass::TCP::EventParameter&);

  private:
    cass::RingBuffer<cass::CASSEvent,cass::RingBufferSize>  &_ringbuffer;
  };

} //end namespace cass

#endif



// Local Variables:
// coding: utf-8
// mode: C++
// c-file-offsets: ((c . 0) (innamespace . 0))
// c-file-style: "Stroustrup"
// fill-column: 100
// End:
