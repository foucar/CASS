// Copyright (C) 2011 Lutz Foucar

/**
 * @file tcp_streamer.cpp contains the base class for all tcp streamers
 *
 * @author Lutz Foucar
 */

#include <stdexcept>

#include "tcp_streamer.h"

#include "shm_deserializer.h"
#include "agat_deserializer.h"

using namespace cass;
using namespace std;
using namespace std::tr1;

TCPStreamer::shared_pointer TCPStreamer::_instance;

TCPStreamer &TCPStreamer::instance()
{
  if(!_instance)
    throw runtime_error("TCPStreamer::instance(): The instance has not been initialized yet.");
  return *_instance.get();
}

TCPStreamer &TCPStreamer::instance(const string &type)
{
  if (type == "shm")
    _instance = shared_pointer(new pixeldetector::SHMStreamer());
  else if (type == "agat")
    _instance = shared_pointer(new ACQIRIS::AGATStreamer());
  else
  {
    throw invalid_argument("TCPStreamer::instance(type): streamer of type '"+ type +
                           "' is unknown.");
  }
  return *_instance;
}

