// Copyright (C) 2011 Lutz Foucar

/**
 * @file cass_ccd/raw_sss_parser.cpp contains class to parse a file containing
 *                   the commercial ccd images created by Per Johnsonns program.
 *
 * @author Lutz Foucar
 */

#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

#include "raw_sss_parser.h"

#include "raw_sss_file_header.h"
#include "file_reader.h"
#include "log.h"

using namespace cass;
using namespace CCD;
using namespace std;
using Streaming::operator >>;

void RAWSSSParser::run()
{
  ifstream &file(*(_readerpointerpair.second._filestream));
  file.seekg (0, ios_base::end);
  const streampos filesize(file.tellg());
  file.seekg (0, ios_base::beg);

  /** the file header information */
  sssFile::Header header;
  file >> header;

  const uint32_t imagesize(header.width * header.height *
                           sizeof(sssFile::image_t::value_type));

  Log::add(Log::INFO,"RAWSSSParser::run(): " + _readerpointerpair.first->filename() +
           "' contains '" + toString(header.nFrames) + "' images");
  
  for (uint32_t iImage(0); iImage < header.nFrames; ++iImage)
  {
    const streampos eventStartPos(file.tellg());
    const uint32_t eventId(Streaming::retrieve<uint32_t>(file));
    savePos(eventStartPos,eventId);
    file.seekg(imagesize,ios_base::cur);
  }
 
  if (file.tellg()<filesize)
    throw runtime_error("RAWSSSParser::run(): Read all images of '" +
                        _readerpointerpair.first->filename() +
                        "' but end of the file not reached.");
}
