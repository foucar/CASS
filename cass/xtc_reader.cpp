// Copyright (C) 2011 Lutz Foucar

/**
 * @file xtc_reader.cpp contains the class to read xtc files
 *
 * @author Lutz Foucar
 */

#include <tr1/memory>

#include "xtc_reader.h"

#include "cass_event.h"
#include "format_converter.h"
#include "cass_settings.h"
#include "pdsdata/xtc/Dgram.hh"

using namespace cass;
using namespace std;
using std::tr1::shared_ptr;

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
  while(1)
  {
    const streampos eventstartpos(file.tellg());
    Pds::Dgram& dg
        (*reinterpret_cast<Pds::Dgram*>(event->datagrambuffer()));
    file.read(event->datagrambuffer(),sizeof(Pds::Dgram));
    if (dg.seq.service() != Pds::TransitionId::L1Accept)
    {
      file.read(dg.xtc.payload(), dg.xtc.sizeofPayload());
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
  Pds::Dgram& dg
      (*reinterpret_cast<Pds::Dgram*>(event.datagrambuffer()));
  file.read(event.datagrambuffer(),sizeof(dg));
  if (dg.xtc.sizeofPayload() > static_cast<int>(DatagramBufferSize))
  {
    throw runtime_error(string("XtcReader::operator (): Datagram size is '" + toString(dg.xtc.sizeofPayload()/1024/1024) +"MB', therefore it is bigger ") +
                        "than the maximum buffer size of " + toString(DatagramBufferSize/1024/1024) +
                        " MB. Something is wrong. Skipping the datagram");
  }
  file.read(dg.xtc.payload(), dg.xtc.sizeofPayload());
  return (_convert(&event));
}
