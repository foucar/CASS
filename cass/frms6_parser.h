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
    void run();
  };
}
#endif
