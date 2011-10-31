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

bool XtcReader::operator ()(ifstream &file, CASSEvent& event)
{
  while(1)
  {
    Pds::Dgram& dg
        (*reinterpret_cast<Pds::Dgram*>(event.datagrambuffer()));
    file.read(event.datagrambuffer(),sizeof(dg));
    file.read(dg.xtc.payload(), dg.xtc.sizeofPayload());
    if (_convert(&event))
      break;
  }
  return (event.id());
}
