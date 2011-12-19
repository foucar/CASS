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
namespace ACQIRIS
{
/** Will parse a lma file
 *
 * @author Lutz Foucar
 */
class LMAParser : public FileParser
{
public:
  /** constructor
   *
   * @param readerpointerpair the filereader the will read the event from files
   * @param event2posreader reference to container that maps events to the
   *                        position in file, reader pair vector
   * @param lock reference to the protector of the eventlist map
   */
  LMAParser(const filereaderpointerpair_t readerpointerpair,
            event2positionreaders_t &event2posreader,
            QReadWriteLock &lock)
    :FileParser(readerpointerpair,event2posreader,lock)
  {}

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

  /** @return the type of file parser */
  virtual const std::string type() {return "lma";}
};
}//end namespace Acqiris
}//end namespace cass
#endif
