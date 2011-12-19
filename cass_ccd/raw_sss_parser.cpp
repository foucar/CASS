// Copyright (C) 2011 Lutz Foucar

/**
 * @file raw_sss_parser.cpp contains class to parse a file containing the
 *                          commercial ccd images created by Per Johnsonns
 *                          program.
 *
 * @author Lutz Foucar
 */

#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

#include "raw_sss_parser.h"

#include "raw_sss_file_header.h"

using namespace cass;
using namespace std;
using namespace std::tr1;

void RAWSSSParser::run()
{
  ifstream &file(*(_readerpointerpair.second._filestream));
  file.seekg (0, ios_base::end);
  const streampos filesize(file.tellg());
  file.seekg (0, ios_base::beg);

  /** the file header information */
  sssFile::Header header;
  file.read(reinterpret_cast<char*>(&header),sizeof(sssFile::Header));

  const uint32_t imagesize(header.width*header.height*sizeof(sssFile::image_t::value_type));

  cout <<"RAWSSSParser::run(): file contains '"<<header.nFrames<<"' images"<<endl;
  
  for (uint32_t iImage(0); iImage < header.nFrames; ++iImage)
  {
    const streampos eventStartPos(file.tellg());
    uint32_t eventId(FileStreaming::retrieve<uint32_t>(file));
    savePos(eventStartPos,eventId);
    file.seekg(imagesize,ios_base::cur);
  }
 
  if (file.tellg()<filesize)
    throw runtime_error("RAWSSSParser::run(): Read all images, but end of the file not reached.");
}
