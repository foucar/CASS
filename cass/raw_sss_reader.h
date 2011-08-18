// Copyright (C) 2011 Lutz Foucar

/**
 * @file raw_sss_reader.h contains class to read commercial ccd files created
 *                        with Per Johnsonn's program
 *
 * @author Lutz Foucar
 */

#ifndef _RAWSSSREADER_H_
#define _RAWSSSREADER_H_

#include <tr1/memory>
#include <fstream>
#include <string>

#include "file_reader.h"

namespace cass
{
  class CASSEvent;

  /** class for reading commercial ccd files
   *
   * @author Lutz Foucar
   */
  class RAWSSSReader : public FileReader
  {
  public:
    /** constructor */
    RAWSSSReader();

    /** read the xtc file contents put them into cassevent
     *
     * @return true when the workers should work on the filled cassevent,
     *         false if not.
     * @param file the file that contains the data to be put into the cassevent
     * @param event the CASSEvent where the data will be put into
     */
    bool operator()(std::ifstream &file, CASSEvent& event);

    /** load the settings of the reader */
    void loadSettings();

    /** this tells when a new file is opened.
     *
     * ignore this message
     */
    void newFile(){}
  };
}
#endif
