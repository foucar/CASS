// CASS TCP server
//
// Copyright (C) 2010 Jochen Küpper

#include "tcpserver.h"


TCPserver::TCPserver(std::unary_function<event_parameter&, Event> event,
                     std::unary_function<histogram_parameter&, Histogram> hist)
  : get_event(event), get_histogram(hist)
{
}




// Local Variables:
// coding: utf-8
// mode: C++
// c-file-offsets: ((c . 0) (innamespace . 0))
// c-file-style: "Stroustrup"
// fill-column: 100
// End:
