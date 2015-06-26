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
 * Uses the text reader to read all the events. See cass::TxtReader for details
 * on what parameters it uses.
 *
 * @author Lutz Foucar
 */
class TxtParser : public FileParser
{
public:
  /** constructor
   *
   * @param readerpointerpair the filereader the will read the event from files
   * @param event2posreader reference to container that maps events to the
   *                        position in file, reader pair vector
   * @param lock reference to the protector of the eventlist map
   */
  TxtParser(const filereaderpointerpair_t readerpointerpair,
            event2positionreaders_t &event2posreader,
            QReadWriteLock &lock)
    : FileParser(readerpointerpair,event2posreader,lock)
  {}

  /** parse the frms6 file
   *
   * detailed description
   */
  void run();

  /** @return the type of file parser */
  virtual const std::string type() {return "txt";}
};
}//end namespace cass
#endif
