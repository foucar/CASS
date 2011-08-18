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
    void run();
  };
}
#endif
