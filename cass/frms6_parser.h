// Copyright (C) 2011 Lutz Foucar

/**
 * @file frms6_parser.h contains class to parse a frms6 file created by Xonline
 *
 * @author Lutz Foucar
 */

#ifndef _FRMS6PARSER_H_
#define _FRMS6PARSER_H_

#include "file_parser.h"

namespace cass
{
  /** Will parse a frms6 file created by Xonline
   *
   * @author Lutz Foucar
   */
  class Frms6Parser : public FileParser
  {
  public:
    /** constructor
     *
     * @param filename the file to parse
     * @param eventmap reference to the map of events
     * @param lock reference to the protector of the eventlist map
     */
    Frms6Parser (const std::string &filename,
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
}
#endif
