// Copyright (C) 2011 Lutz Foucar

/**
 * @file frms6_parser.cpp contains class to parse a frms6 file created by Xonline
 *
 * @author Lutz Foucar
 */

#include <fstream>
#include <sstream>
#include <stdexcept>

#include "frms6_parser.h"
#include "frms6_file_header.h"

using namespace cass;
using namespace std;
using namespace std::tr1;

void Frms6Parser::run()
{
  frms6File::FileHeader fileHead;
  ifstream &file(*(_readerpointerpair.second._filestream));
  file.read(reinterpret_cast<char*>(&fileHead), sizeof(frms6File::FileHeader));
  size_t frameWidth_bytes(fileHead.the_width*sizeof(frms6File::pixel));

  frms6File::FrameHeader frameHead;
  while (!file.eof())
  {
    const streampos eventstartpos;
    file.read(reinterpret_cast<char*>(&frameHead), sizeof(frms6File::FrameHeader) );
    savePos(eventstartpos,frameHead.external_id);
    file.seekg(frameWidth_bytes*frameHead.the_height, std::ios_base::cur);
  }
}
