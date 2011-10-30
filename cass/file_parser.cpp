// Copyright (C) 2011 Lutz Foucar

/**
 * @file file_parser.cpp contains the base class for all file parsers
 *
 * @author Lutz Foucar
 */

#include <sstream>
#include <stdexcept>
#include <iostream>

#include "file_parser.h"

#include "lma_parser.h"
#include "raw_sss_parser.h"
#include "frms6_parser.h"
#include "txt_parser.h"
#include "file_reader.h"

using namespace cass;
using namespace std;
using namespace std::tr1;

FileParser::FileParser(const filereaderpointerpair_t readerpointerpair,
                       event2positionreaders_t &event2posreader,
                       QReadWriteLock &lock)
  :QThread(),
    _readerpointerpair(readerpointerpair),
    _event2posreader(event2posreader),
    _lock(lock)
{}

FileParser::~FileParser()
{}

FileParser::shared_pointer FileParser::instance(const std::string type,
                                                const filereaderpointerpair_t readerpointerpair,
                                                event2positionreaders_t &event2posreader,
                                                QReadWriteLock &lock)
{
  shared_pointer ptr;
  if (type == "lma")
    ptr = shared_pointer(new LMAParser(readerpointerpair,event2posreader, lock));
  else if (type == "sss")
    ptr = shared_pointer(new RAWSSSParser(readerpointerpair,event2posreader, lock));
  else if (type == "frms6")
    ptr = shared_pointer(new Frms6Parser(readerpointerpair,event2posreader, lock));
  else if (type == "txt")
    ptr = shared_pointer(new TxtParser(readerpointerpair,event2posreader, lock));
  else
    throw invalid_argument("FileParser::instance: file extension '"+ type +"' is unknown.");
  return ptr;
}

void FileParser::savePos(const std::streampos& eventStartPos, const uint64_t eventId)
{
  _readerpointerpair.second._pos = eventStartPos;
  if (!eventId)
  {
    cout <<"FileParser::savePos: WARNING EventId '"<<eventId
        <<"' from parser type '"<< type()
         <<"' seems to be wrong skipping event." <<endl;
    return;
  }
  QWriteLocker lock(&_lock);
  _event2posreader[eventId].push_back(_readerpointerpair);
}
