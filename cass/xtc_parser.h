// Copyright (C) 2011 Lutz Foucar

/**
 * @file xtc_parser.h contains class to parse xtc files
 *
 * @author Lutz Foucar
 */

#ifndef _XTCPARSER_H_
#define _XTCPARSER_H_

#include <fstream>

#include "file_parser.h"

namespace cass
{
  class CASSEvent;
  class FormatConverter;

  /** class for parsing xtc files
   *
   * goes through the xtc file and finds the events that contain data
   *
   * @author Lutz Foucar
   */
  class XtcParser : public FileParser
  {
  public:
    /** constructor
     *
     * @param readerpointerpair the filereader the will read the event from files
     * @param event2posreader reference to container that maps events to the
     *                        position in file, reader pair vector
     * @param lock reference to the protector of the eventlist map
     */
    XtcParser(const filereaderpointerpair_t readerpointerpair,
              event2positionreaders_t &event2posreader,
              QReadWriteLock &lock)
      :FileParser(readerpointerpair,event2posreader,lock)
    {}

    /** parse the frms6 file
     *
     * detailed description
     */
    void run();

  };
}
#endif
