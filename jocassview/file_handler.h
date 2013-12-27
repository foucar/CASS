// Copyright (C) 2013 Lutz Foucar

/**
 * @file file_handler.h contains a file handler
 *
 * @author Lutz Foucar
 */

#ifndef _FILE_HANDLER_
#define _FILE_HANDLER_

namespace cass
{
class HistogramBackend;
}//end namespace cass

class QString;
class QImage;

namespace jocassview
{

/** read and write data to files
 *
 * @author Lutz Foucar
 */
class FileHandler
{
public:
  /** read data from a file
   *
   * @return the container for the data
   * @param filename The file to read the data from
   * @param key in case of a hdf5 file the key to the requested data. Default is ""
   */
  static cass::HistogramBackend * getData(const QString &filename, const QString &key="");

private:
  /** read the data from an image file
   *
   * @return the image
   * @param filename the name of the file that contains the image data
   */
  QImage loadImage(const QString &filename);

  /** read the data from a hist file
   *
   * @return pointer to the data
   * @param filename the name of the file
   */
  cass::HistogramBackend * loadDataFromHist(const QString &filename);

  /** read the data from a CSV file
   *
   * @return pointer to the data
   * @param filename the name of the file
   */
  cass::HistogramBackend * loadDataFromCSV(const QString &filename);

  /** read the data from a hdf5 file
   *
   * @return pointer to the data
   * @param filename the name of the file
   * @param key in case of a hdf5 file the key to the requested data. Default is ""
   */
  cass::HistogramBackend * loadDataFromH5(const QString &filename, const QString &key="");
};
}//end namespace jocassview
#endif
