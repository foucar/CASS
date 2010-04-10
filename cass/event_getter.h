//Copyright (C) 2010 Lutz Foucar

#ifndef __EVENT_GETTER_H__
#define __EVENT_GETTER_H__

#include <string>

#include "cass.h"
#include "cass_event.h"
#include "ringbuffer.h"

namespace cass
{
  /** Event retrievel parameters
  @author Jochen Küpper
  */
  struct EventParameter
  {
    EventParameter(size_t _what, unsigned _t1, unsigned _t2)
      : what(_what), t1(_t1), t2(_t2)
    {};

    size_t what;
    unsigned int t1, t2;
  };


  /*! Retrieve a CASSEvent from the ringbuffer

    class that will retrieve an cassevent from the ringbuffer
    using the requested arguments
   @author Lutz Foucar
   */
  class CASSSHARED_EXPORT EventGetter
  {
  public:
    /** constructor
      @param ringbuffer Reference to the Ringbuffer to retrieve the events from
      */
    EventGetter(RingBuffer<CASSEvent, RingBufferSize>&ringbuffer);
    /** function to retrieve the event using the event parameters*/
    const std::string operator()(const EventParameter&) const;

  protected:
    RingBuffer<CASSEvent,RingBufferSize>  &_ringbuffer; //!< the ringbuffer
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
