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

#include "hlltypes.h"

namespace cass
{
class CASSEvent;

namespace pixeldetector
{
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
  /** the frms6 file header */
  hllDataTypes::Frms6FileHeader _fileHead;

  /** header that comes before every frame */
  hllDataTypes::FrameHeader _frameHead;

  /** a buffer to not allocate the read buffer for each event */
  std::vector<hllDataTypes::pixel> _hllFrameBuffer;
};
}//end namespace pixeldetector
}//end namespace cass
#endif
