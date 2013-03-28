// Copyright (C) 2011 Lutz Foucar

/**
 * @file cass_pixeldetector/raw_sss_parser.h contains class to parse a file
 *        containing the commercial ccd images created by Per Johnsonns program.
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
   * @param readerpointerpair the filereader the will read the event from files
   * @param event2posreader reference to container that maps events to the
   *                        position in file, reader pair vector
   * @param lock reference to the protector of the eventlist map
   */
  RAWSSSParser(const filereaderpointerpair_t readerpointerpair,
               event2positionreaders_t &event2posreader,
               QReadWriteLock &lock)
    :FileParser(readerpointerpair,event2posreader,lock)
  {}

  /** parse the raw.sss file
   *
   * read the header information and then the individual frames timestamp. Save
   * the position where the timestamp is found then skip the frame data to read
   * the next frames timestamp. Do this until the number of frames reported in
   * the header have been read. Check if we are at the end of the file, if not
   * throw an error.
   */
  void run();

  /** @return the type of file parser */
  virtual const std::string type() {return "sss";}
};
}//end namespace cass
#endif
