// Copyright (C) 2011 Lutz Foucar

/**
 * @file txt_reader.h contains class to read txt ascii files
 *
 * @author Lutz Foucar
 */

#ifndef _TXTREADER_H_
#define _TXTREADER_H_

#include <tr1/memory>
#include <fstream>
#include <string>
#include <vector>

#include "file_reader.h"

namespace cass
{
class CASSEvent;

/** class for reading txt files
 *
 * first line has to give the names of the variables, which are then ordered
 * line by line.
 *
 * @cassttng TxtReader/{Deliminator}\n
 *           The deliminator that is used to separate the values. Default is '\t',
 *           which is a tab.
 * @cassttng TxtReader/{EventIdHeader}\n
 *           The name of the Header under which the Event Id is stored. Default
 *           is "".
 *
 * @author Lutz Foucar
 */
class TxtReader : public FileReader
{
public:
  /** constructor */
  TxtReader();

  /** read the frms6 file contents put them into cassevent
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
   * @param file the filestream to the header information of the file
   */
  void readHeaderInfo(std::ifstream &file);

private:
  /** the value names */
  std::vector<std::string> _headers;

  /** the deliminator by which the values are separated in the ascii file */
  char _delim;

  /** the header name under which the event id is stored */
  std::string _eventIdhead;
};
}//end namespace cass
#endif
