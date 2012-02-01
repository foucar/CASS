// Copyright (C) 2011 Lutz Foucar

/**
 * @file file_reader.h contains base class for all file readers
 *
 * @author Lutz Foucar
 */

#ifndef _FILEREADER_H_
#define _FILEREADER_H_

#include <tr1/memory>
#include <fstream>
#include <string>

namespace cass
{
class CASSEvent;

/** base class for all file readers
 *
 * @author Lutz Foucar
 */
class FileReader
{
public:
  /** typedef the shared pointer of this */
  typedef std::tr1::shared_ptr<FileReader> shared_pointer;

  /** virtual destructor */
  virtual ~FileReader() {}

  /** create an instance of the requested type
   *
   * @return a shared pointer to the requested type
   * @param type the reqested type
   */
  static shared_pointer instance(const std::string &type);

  /** read the file contents
   *
   * should read the file contents and convert them so that they can be put
   * into the cassevent
   *
   * @return true when the workers should work on the filled cassevent,
   *         false if not.
   * @param file the file that contains the data to be put into the cassevent
   * @param event the CASSEvent where the data will be put into
   */
  virtual bool operator()(std::ifstream &file, CASSEvent& event)=0;

  /** load the settings of the reader */
  virtual void loadSettings()=0;

  /** read the file header
   *
   * @param file the filestream to the header information of the file
   */
  virtual void readHeaderInfo(std::ifstream &/*file*/) {}

  /** tell the reader the name of the file it is working on
   *
   * @param filename the filename of the file that this reader is working on
   */
  virtual void setFilename(const std::string& /*filename*/) {}

protected:
  /** only inheritants can create this */
  FileReader() {}
};
} //end namespace cass
#endif
