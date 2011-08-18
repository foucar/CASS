// Copyright (C) 2011 Lutz Foucar

/**
 * @file lma_parser.cpp contains class to parse a lma file
 *
 * @author Lutz Foucar
 */

#include <fstream>
#include <sstream>
#include <stdexcept>

#include "lma_parser.h"
#include "lma_file_header.h"

using namespace cass;
using namespace std;
using namespace std::tr1;

void LMAParser::run()
{
  ifstream &file(*(_filepointer._filestream));

  vector<char> headerarray(sizeof(lmaFile::GeneralHeader));
  file.read(&headerarray.front(),sizeof(lmaFile::GeneralHeader));
  const lmaFile::GeneralHeader &header
      (*reinterpret_cast<lmaFile::GeneralHeader*>(&headerarray.front()));

  if (header.nbrBits != 16)
  {
    stringstream ss;
    ss<<"LMAParser():run: The lma file seems to contain 8-bit wavefroms '"
     <<header.nbrBits<<"'. Currently this is not supported.";
    throw runtime_error(ss.str());
  }

  for (int16_t i(0) ; i < header.nbrChannels ;++i)
  {
    if (header.usedChannelBitmask & (0x1<<i))
    {
      vector<char> chanheaderarray(sizeof(lmaFile::ChannelHeader));
      file.read(&chanheaderarray.front(),sizeof(lmaFile::ChannelHeader));
    }
  }

  while(!file.eof())
  {
    uint32_t eventID(FileStreaming::peek<int32_t>(file));
    savePos(eventID);

    eventID = FileStreaming::retrieve<int32_t>(file);
    double horpos(FileStreaming::retrieve<double>(file));

    for (size_t i=0; i<header.nbrChannels;++i)
    {
      if (header.usedChannelBitmask & (0x1<<i))
      {
        int16_t nbrPulses(FileStreaming::retrieve<int16_t>(file));
        for (int16_t i(0); i < nbrPulses; ++i)
        {
          int32_t wavefromOffset(FileStreaming::retrieve<int32_t>(file));
          int32_t pulslength(FileStreaming::retrieve<int32_t>(file));
          size_t dataSize(pulslength * 2);
          file.seekg(file.tellg() + dataSize);
        }
      }
    }
  }

}
