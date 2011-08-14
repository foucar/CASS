// Copyright (C) 2011 Lutz Foucar

/**
 * @file file_parser.cpp contains the base class for all file parsers
 *
 * @author Lutz Foucar
 */

#include <sstream>
#include <stdexcept>

#include "file_parser.h"

using namespace cass;
using namespace std;
using namespace std::tr1;

namespace cass
{
/** retrieve the extension of a filename
 *
 * find the last occurence of '.' after which hopefully the extension comes.
 * Funtion was inspired by 'graphitemaster' at stackoverflow:
 *
 * http://stackoverflow.com/questions/51949/how-to-get-file-extension-from-string-in-c
 *
 * @return string containing the extension
 * @param string containing the filename
 *
 * @author Lutz Foucar
 */
string extension(const string &filename)
{
  if(filename.find_last_of(".") != std::string::npos)
    return filename.substr(filename.find_last_of(".")+1);
  else
  {
    stringstream ss;
    ss <<"extension: the filename '"<<filename<<"' does not have a file extension.";
    throw invalid_argument(ss.str());
  }
}
}

FileParser::FileParser(const std::string &filename,
                       eventmap_t &eventmap,
                       QReadWriteLock &lock)
  :QThread(),
    _eventmap(eventmap),
    _lock(lock)
{
  _filepointer._filestream =
      FilePointer::filestream_t(new ifstream(filename.c_str(), std::ios::binary | std::ios::in));
}

FileParser::~FileParser()
{
}

FileParser::shared_pointer FileParser::instance(const std::string &filename,
                                                eventmap_t &eventmap,
                                                QReadWriteLock &lock)
{
  shared_pointer ptr;
  string ext(extension(filename));
//  if (ext == "lma")
//    ptr = shared_pointer(new XtcReader());
//  else if (ext == "lma")
//    ptr = shared_pointer(new LmaReader());
//  else
  {
    stringstream ss;
    ss << "FileParser::instance: file extension '"<< ext <<"' is unknown.";
    throw invalid_argument(ss.str());
  }
  return ptr;
}
