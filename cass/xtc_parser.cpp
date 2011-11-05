// Copyright (C) 2011 Lutz Foucar

/**
 * @file xtc_parser.cpp contains the class to parse xtc files
 *
 * @author Lutz Foucar
 */

#include "xtc_parser.h"

#include "cass_event.h"
#include "cass_settings.h"

#include "pdsdata/xtc/Dgram.hh"

using namespace cass;
using namespace std;

void XtcParser::run()
{
  ifstream &file(*(_readerpointerpair.second._filestream));
  Pds::Dgram datagram;
  while(!file.eof())
  {
    const streampos eventstartpos(file.tellg());
    file.read(reinterpret_cast<char*>(&datagram), sizeof(Pds::Dgram));
    file.seekg(datagram.xtc.sizeofPayload(), ios_base::cur);

    if (datagram.seq.service() == Pds::TransitionId::L1Accept)
    {
      uint64_t bunchId = datagram.seq.clock().seconds();
      bunchId = (bunchId<<32) + static_cast<uint32_t>(datagram.seq.stamp().fiducials()<<8);
      savePos(eventstartpos,bunchId);
    }
  }
}
