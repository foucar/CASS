// Copyright (C) 2011 Lutz Foucar

/**
 * @file raw_sss_parser.h contains class to parse a file containing the commercial
 *                        ccd images created by Per Johnsonns program.
 *
 * @author Lutz Foucar
 */

#ifndef _RAWSSSPARSER_H_
#define _RAWSSSPARSER_H_

#include "file_parser.h"

namespace cass
{
  /** Will parse a file containing commercial ccd images.
   *
   * @author Lutz Foucar
   */
  class RAWSSSParser : public FileParser
  {
  public:
    /** constructor
     *
     * @param filename the file to parse
     * @param eventmap reference to the map of events
     * @param lock reference to the protector of the eventlist map
     */
    RAWSSSParser(const std::string &filename,
                 eventmap_t &eventmap,
                 QReadWriteLock &lock)
       :FileParser(filename,eventmap,lock)
    {}

    /** parse the raw.sss file
     *
     * detailed description
     */
    void run();
  };
}
#endif
