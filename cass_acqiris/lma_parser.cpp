// Copyright (C) 2011 Lutz Foucar

/**
 * @file lma_parser.cpp contains class to parse a lma file
 *
 * @author Lutz Foucar
 */

#include <fstream>
#include <sstream>
#include <iostream>
#include <stdexcept>

#include "lma_parser.h"
#include "agattypes.h"
#include "log.h"

using namespace cass;
using namespace ACQIRIS;
using namespace std;
using Streaming::operator >>;

void LMAParser::run()
{
  ifstream &file(*(_readerpointerpair.second._filestream));

  lmaHeader::General header;
  file >> header;

  if (header.nbrBits != 16)
    throw runtime_error("LMAParser():run: The lma file seems to contain 8-bit wavefroms '"
                        + toString(header.nbrBits) + "'. Currently this is not supported.");

  Log::add(Log::VERBOSEINFO,"LMAParser(): File contains instrument with '" +
           toString(header.nbrChannels) + "' channels:");

  for (int16_t i(0) ; i < header.nbrChannels ;++i)
  {
    Log::add(Log::VERBOSEINFO,"LMAParser(): Channel '" + toString(i) + "' is " +
             ((header.usedChannelBitmask & (0x1<<i))?"":"not") + " recorded!");
    if (header.usedChannelBitmask & (0x1<<i))
      file.seekg(sizeof(lmaHeader::Channel),ios_base::cur);
  }

  lmaHeader::Event evtHead;
  while(!file.eof())
  {
    const streampos eventStartPos(file.tellg());
    file >> evtHead;
    savePos(eventStartPos,evtHead.id);

    for (int16_t i=0; i<header.nbrChannels;++i)
    {
      if (header.usedChannelBitmask & (0x1<<i))
      {
        int16_t nbrPulses(Streaming::retrieve<int16_t>(file));
        for (int16_t i(0); i < nbrPulses; ++i)
        {
          lmaHeader::Puls pulsHead;
          file >> pulsHead;
          const size_t dataSize(pulsHead.length * 2);
          file.seekg(dataSize,ios_base::cur);
        }
      }
    }
  }

}
