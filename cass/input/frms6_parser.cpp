// Copyright (C) 2011 Lutz Foucar

/**
 * @file cass_pixeldetector/frms6_parser.cpp contains class to parse a frms6
 *                     file created by Xonline.
 *
 * @author Lutz Foucar
 */

#include <fstream>
#include <sstream>
#include <stdexcept>

#include "frms6_parser.h"
#include "hlltypes.hpp"

using namespace cass;
using namespace std;
using namespace std::tr1;
using Streaming::operator >>;

void Frms6Parser::run()
{
  ifstream &file(*(_readerpointerpair.second._filestream));
  hllDataTypes::Frms6FileHeader fileHead;
  file >> fileHead;
  size_t frameWidth_bytes(fileHead.the_width*sizeof(hllDataTypes::pixel));

  hllDataTypes::FrameHeader frameHead;
  while (!file.eof())
  {
    const streampos eventstartpos;
    file >> frameHead;
    savePos(eventstartpos,frameHead.external_id);
    file.seekg(frameWidth_bytes*frameHead.the_height, std::ios_base::cur);
  }
}
