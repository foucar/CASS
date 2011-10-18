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

#include "file_reader.h"

namespace cass
{
class CASSEvent;

/** class for reading txt files
 *
 * first line has to give the names of the varibles, which are then ordered
 * line by line.
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

  /** this tells when a new file is opened.
   *
   * ignore this message
   */
  void newFile(bool state=true){_newfile = state;}

private:
  /** flag to tell whether its a new file */
  bool _newfile;
};
}//end namespace cass
#endif
