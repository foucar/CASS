// CASS TCP server
//
// Copyright (C) 2010 Jochen Küpper

#include "tcpserver.h"


namespace cass
{

TCPserver::TCPserver(std::unary_function<event_parameter&, const SerializedEvent *> event,
                     std::unary_function<histogram_parameter&, const SerializedHistogram *> hist)
  : get_event(event), get_histogram(hist)
{
}


}



// Local Variables:
// coding: utf-8
// mode: C++
// c-file-offsets: ((c . 0) (innamespace . 0))
// c-file-style: "Stroustrup"
// fill-column: 100
// End:
