// Copyright (C) 2011 Lutz Foucar

/**
 * @file lma_parser.h contains class to parse a lma file
 *
 * @author Lutz Foucar
 */

#ifndef _LMAPARSER_H_
#define _LMAPARSER_H_

#include "file_parser.h"

namespace cass
{
  /** Will parse a lma file
   *
   * @author Lutz Foucar
   */
  class LMAParser : public FileParser
  {
  public:
    void run();
  };
}
#endif
