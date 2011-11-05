// Copyright (C) 2011 Lutz Foucar

/**
 * @file xtc_reader.h contains class to read xtc files
 *
 * @author Lutz Foucar
 */

#ifndef _XTCREADER_H_
#define _XTCREADER_H_

#include <tr1/memory>
#include <fstream>
#include <string>

#include "file_reader.h"

namespace cass
{
  class CASSEvent;
  class FormatConverter;

  /** class for reading xtc files
   *
   * it uses the format converter to the xtc datagrams to cassevent
   *
   * @author Lutz Foucar
   */
  class XtcReader : public FileReader
  {
  public:
    /** constructor */
    XtcReader();

    /** read the xtc file contents put them into cassevent
     *
     * read datagrams until the return value of the conversion is true, which
     * indicates
     *
     * @return true when the workers should work on the filled cassevent,
     *         false if not.
     * @param file the file that contains the data to be put into the cassevent
     * @param event the CASSEvent where the data will be put into
     */
    bool operator()(std::ifstream &file, CASSEvent& event);

    /** load the settings of the reader */
    void loadSettings();

    /** read the file header
     *
     * read datagram by datagram until the first L1 transition appears
     *
     * @param file the filestream to the header information of the file
     */
    void readHeaderInfo(std::ifstream &file);

  private:
    /** a reference to the format converter functor */
    FormatConverter &_convert;
  };
}
#endif
