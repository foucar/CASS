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
 * @author to be determined
 */
class Frms6Parser : public FileParser
{
public:
  /** constructor
   *
   * @param readerpointerpair the filereader the will read the event from files
   * @param event2posreader reference to container that maps events to the
   *                        position in file, reader pair vector
   * @param lock reference to the protector of the eventlist map
   */
  Frms6Parser(const filereaderpointerpair_t readerpointerpair,
              event2positionreaders_t &event2posreader,
              QReadWriteLock &lock)
    :FileParser(readerpointerpair,event2posreader,lock)
  {}

  /** parse the frms6 file
   *
   * detailed description
   */
  void run();

  /** @return the type of file parser */
  virtual const std::string type() {return "frms6";}
};
}//end namespace cass
#endif
