// Copyright (C) 2011 Lutz Foucar

/**
 * @file lma_reader.h contains class to read lma files
 *
 * @author Lutz Foucar
 */

#ifndef _LMAREADER_H_
#define _LMAREADER_H_

#include <tr1/memory>
#include <fstream>
#include <string>

#include "file_reader.h"
#include "acqiris_device.h"

namespace cass
{
class CASSEvent;
class FormatConverter;

/** class for reading lma files
 *
 * read an event in lma file format from a given position in a file stream and
 * put the contents in the cassvent after conversion.
 *
 * @author Lutz Foucar
 */
class LmaReader : public FileReader
{
public:
  /** constructor */
  LmaReader();

  /** read the lma file contents put them into cassevent
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
   * @param state the state that the _newFile flag should be set to
   */
  void newFile(bool state=true) {_newFile=state;}

private:
  /** flag to tell whether there is a new file */
  bool _newFile;

  /** acqiris device where we store the file header information */
  ACQIRIS::Instrument _instrument;

  /** bitmask describing which channels are active in the instrument */
  uint32_t _usedChannelBitmask;
};
}//end namespace cass
#endif
