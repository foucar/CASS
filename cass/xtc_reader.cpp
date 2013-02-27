// Copyright (C) 2011 Lutz Foucar

/**
 * @file xtc_reader.cpp contains the class to read xtc files
 *
 * @author Lutz Foucar
 */

#include "pdsdata/xtc/Dgram.hh"

#include <tr1/memory>

#include "xtc_reader.h"

#include "cass_event.h"
#include "format_converter.h"
#include "cass_settings.h"
#include "log.h"

using namespace cass;
using namespace std;
using std::tr1::shared_ptr;

void readDgramHeaderToBuf(ifstream &file, CASSEvent::buffer_t &buf)
{
  buf.resize(sizeof(Pds::Dgram));
  file.read(&buf.front(),sizeof(Pds::Dgram));
}

void readDgramPayloadToBuf(ifstream &file, CASSEvent::buffer_t &buf)
{
  Pds::Dgram& dg(reinterpret_cast<Pds::Dgram&>(buf.front()));
  const int payloadSize(dg.xtc.sizeofPayload());
  buf.resize(sizeof(Pds::Dgram) +  payloadSize);
  file.read(&buf[sizeof(Pds::Dgram)], payloadSize);
}

XtcReader::XtcReader()
  :_convert(*FormatConverter::instance())
{}

void XtcReader::loadSettings()
{
  _convert.loadSettings(0);
}

void XtcReader::readHeaderInfo(std::ifstream &file)
{
  shared_ptr<CASSEvent> event(new CASSEvent);
  CASSEvent::buffer_t& buf(event->datagrambuffer());
  while(1)
  {
    const streampos eventstartpos(file.tellg());
    readDgramHeaderToBuf(file,buf);
    Pds::Dgram& dg(reinterpret_cast<Pds::Dgram&>(buf.front()));
    if (dg.seq.service() != Pds::TransitionId::L1Accept)
    {
      readDgramPayloadToBuf(file,buf);
      _convert(event.get());
    }
    else
    {
      file.seekg(eventstartpos);
      break;
    }
  }
}

bool XtcReader::operator ()(ifstream &file, CASSEvent& event)
{
  CASSEvent::buffer_t& buf(event.datagrambuffer());
  readDgramHeaderToBuf(file,buf);
  Pds::Dgram& dg(reinterpret_cast<Pds::Dgram&>(buf.front()));
  if (dg.xtc.sizeofPayload() > static_cast<int>(DatagramBufferSize))
  {
    Log::add(Log::WARNING,"XtcReader::operator (): Datagram size is '" +
             toString(dg.xtc.sizeofPayload()/1024/1024) +"MB', therefore it is bigger " +
             "than the maximum buffer size of " + toString(DatagramBufferSize/1024/1024) +
             " MB. Something is wrong. Skipping the datagram");
  }
  readDgramPayloadToBuf(file,buf);
  return (_convert(&event));
}
