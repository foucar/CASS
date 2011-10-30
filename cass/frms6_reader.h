// Copyright (C) 2011 Lutz Foucar

/**
 * @file frms6_reader.h contains class to read frms6 files created by Xonline
 *
 * @author Lutz Foucar
 */

#ifndef _FRMS6READER_H_
#define _FRMS6READER_H_

#include <tr1/memory>
#include <fstream>
#include <string>
#include <vector>

#include "file_reader.h"

#include "frms6_file_header.h"

namespace cass
{
class CASSEvent;

/** class for reading frms6 files
 *
 * @author to be determined
 */
class Frms6Reader : public FileReader
{
public:
  /** constructor */
  Frms6Reader();

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
  frms6File::FileHeader _fileHead;

  frms6File::FrameHeader _frameHead;

  std::vector<frms6File::pixel> _hllFrameBuffer;
};
}//end namespace cass
#endif
