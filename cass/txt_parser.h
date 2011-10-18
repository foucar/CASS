// Copyright (C) 2011 Lutz Foucar

/**
 * @file txt_parser.h contains class to parse a txt ascii file
 *
 * @author Lutz Foucar
 */

#ifndef _TXTPARSER_H_
#define _TXTPARSER_H_

#include "file_parser.h"

namespace cass
{
/** Will parse a txt files
 *
 * @author Lutz Foucar
 */
class TxtParser : public FileParser
{
public:
  /** constructor
   *
   * @param filename the file to parse
   * @param eventmap reference to the map of events
   * @param lock reference to the protector of the eventlist map
   */
  TxtParser (const std::string &filename,
               eventmap_t &eventmap,
               QReadWriteLock &lock)
    :FileParser(filename,eventmap,lock)
  {}

  /** parse the frms6 file
   *
   * detailed description
   */
  void run();
};
}//end namespace cass
#endif
