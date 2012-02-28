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
   * The type is determined from the filename
   *
   * @return a shared pointer to the requested type
   * @param filename the filename of the file that this reader is working on
   */
  static shared_pointer instance(const std::string &filename);

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

  /** set the filename of the instance
   *
   * @param filename the filename of the file that this reader is working on
   */
  void filename(const std::string &filename) {_filename = filename;}

  /** retrieve  the filename that this instance is working on
   *
   * @return filename the filename of the file that this reader is working on
   */
  const std::string& filename()const {return _filename;}

protected:
  /** only inheritants can create this */
  FileReader() {}

  /** the name of the file that we read the values from */
  std::string _filename;
};
} //end namespace cass
#endif
