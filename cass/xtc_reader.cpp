// Copyright (C) 2011 Lutz Foucar

/**
 * @file xtc_reader.cpp contains the class to read xtc files
 *
 * @author Lutz Foucar
 */

#include "xtc_reader.h"

#include "cass_event.h"
#include "format_converter.h"
#include "cass_settings.h"
#include "pdsdata/xtc/Dgram.hh"

using namespace cass;
using namespace std;

XtcReader::XtcReader()
  :_convert(*FormatConverter::instance())
{}

void XtcReader::loadSettings()
{
  _convert.loadSettings(0);
}

void XtcReader::readHeaderInfo(std::ifstream &file)
{
  CASSEvent event;
  while(1)
  {
    const streampos eventstartpos(file.tellg());
    Pds::Dgram& dg
        (*reinterpret_cast<Pds::Dgram*>(event.datagrambuffer()));
    file.read(event.datagrambuffer(),sizeof(Pds::Dgram));
    if (dg.seq.service() != Pds::TransitionId::L1Accept)
    {
      file.read(dg.xtc.payload(), dg.xtc.sizeofPayload());
      _convert(&event);
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
  file.read(dg.xtc.payload(), dg.xtc.sizeofPayload());
  return (_convert(&event));
}
