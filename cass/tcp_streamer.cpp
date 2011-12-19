// Copyright (C) 2011 Lutz Foucar

/**
 * @file tcp_streamer.cpp contains the base class for all tcp streamers
 *
 * @author Lutz Foucar
 */

#include <stdexcept>

#include "tcp_streamer.h"

using namespace cass;
using namespace std;
using namespace std::tr1;

TCPStreamer::shared_pointer TCPStreamer::_instance;

TCPStreamer &TCPStreamer::instance()
{
  if(!_instance)
    throw runtime_error("");
  return *_instance.get();
}

TCPStreamer &TCPStreamer::instance(const string &type)
{
//  if (type == "xtc")
//    ptr = shared_pointer(new XtcReader());
//  else if (type == "lma")
//    ptr = shared_pointer(new LmaReader());
//  else if (type == "sss")
//    ptr = shared_pointer(new RAWSSSReader());
//  else if (type == "frms6")
//    ptr = shared_pointer(new Frms6Reader());
//  else if (type == "txt")
//    ptr = shared_pointer(new TxtReader());
//  else
  {
    throw invalid_argument("TCPStreamer::instance: streamer of type '"+ type +
                           "' is unknown.");
  }
  return *_instance;
}

