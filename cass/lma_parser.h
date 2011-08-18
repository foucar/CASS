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
    /** parse the lma file
     *
     * first read the headers and check whether this parser is suitable for
     * reading this kind of waveforms.\n
     * Go through all events an remember the position and correlate them with
     * the even id.\n
     * since we zero substracted the waveform in the lma file, we need to
     * iterate through the waveform sinpplets called pulses and find out how
     * long they are to be able to jump to the next event.
     */
    void run();
  };
}
#endif
