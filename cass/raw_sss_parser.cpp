// Copyright (C) 2011 Lutz Foucar

/**
 * @file raw_sss_parser.cpp contains class to parse a file containing the
 *                          commercial ccd images created by Per Johnsonns
 *                          program.
 *
 * @author Lutz Foucar
 */

#include <fstream>
#include <sstream>
#include <stdexcept>

#include "raw_sss_parser.h"

using namespace cass;
using namespace std;
using namespace std::tr1;

void RAWSSSParser::run()
{
  ifstream &file(*(_filepointer._filestream));

  const uint32_t width(FileStreaming::retrieve<uint32_t>(file));
  const uint32_t height(FileStreaming::retrieve<uint32_t>(file));
  const uint32_t nImages(FileStreaming::retrieve<uint32_t>(file));
  const uint32_t imagesize(width*height);

  for (uint32_t iImage(0); iImage < nImages; ++iImage)
  {
    uint32_t eventId(FileStreaming::peek<uint32_t>(file));
    savePos(eventId);
    eventId = (FileStreaming::retrieve<uint32_t>(file));
    const size_t curPos(file.tellg());
    const size_t offset(curPos + imagesize);
    file.seekg(offset);
    uint32_t heightCompare(FileStreaming::retrieve<uint32_t>(file));

    if (heightCompare != height)
    {
      stringstream ss;
      ss << "RAWSSSParser(): The read height '"<<heightCompare
         <<"' does not match to the height given in the header '"<<height<<"'";
      throw runtime_error(ss.str());
    }
  }
  if (!file.eof())
  {
    stringstream ss;
    ss <<"we read all images, but we are not at the end of the file.";
    throw runtime_error(ss.str());
  }
}
