// Copyright (C) 2013 Lutz Foucar

/**
 * @file file_handler.h contains a file handler
 *
 * @author Lutz Foucar
 */

#ifndef _FILE_HANDLER_
#define _FILE_HANDLER_

#include "data_source.h"

namespace cass
{
class HistogramBackend;
}//end namespace cass

class QString;
class QImage;
class QStringList;

namespace jocassview
{

/** read and write data to files
 *
 * @author Lutz Foucar
 */
class FileHandler : public DataSource
{
public:
  /** read data from a file
   *
   * @return the container for the data
   * @param filename The file to read the data from
   * @param key in case of a hdf5 file the key to the requested data. Default is ""
   */
  static cass::HistogramBackend * getData(const QString &filename, const QString &key="");

  /** return the list of keys in the file
   *
   * in case this is not a container file (e.g. h5) this will just return the
   * filename without path and suffix.
   *
   * @return a list of keys
   * @param filename The name of the file whos keys should be read
   */
  static QStringList getKeyList(const QString &filename);

  /** return the basename of the filename
   *
   * @return basename of filename
   * @param filename the filename whos basename should be extracted
   */
  static QString getBaseName(const QString &filename);

  /** return whether the file is a container file
   *
   * @return true when the file is a container file (e.g. hdf5 file)
   * @param filename the name of the file
   */
  static bool isContainerFile(const QString &filename);

  /** save data to a given file
   *
   * @param filename The filename to save the data to
   * @param data the data to save
   */
  static void saveData(const QString &filename, cass::HistogramBackend* data);

  /** save data to a given container file
   *
   * @param filename The filename to save the data to
   * @param data the data to save
   */
  static void saveDataToContainer(const QString &filename, cass::HistogramBackend* data);

  /** retrieve an result from the file
   *
   * @param key the key of the result
   * @param id the eventid of the result
   */
  cass::HistogramBackend* result(const QString &key, quint64 id=0);

  /** retrieve the list of names that an be displayed from the file
   *
   * @return list of names of displayable items
   */
  QStringList resultNames();

  /** retrieve the type of source
   *
   * @return "File"
   */
  QString type()const;

  /** set the filename
   *
   * @param filename the filename to work on
   */
  void setFilename(const QString &filename);

private:
  /** read the data from an image file
   *
   * @return the image
   * @param filename the name of the file that contains the image data
   */
  QImage loadImage(const QString &filename);

  /** save image data
   *
   * @param filename the name of the file to save the image in
   * @param image the image to save
   */
  void saveImage(const QString &filename,const QImage &image);

  /** read the data from a hist file
   *
   * @return pointer to the data
   * @param filename the name of the file
   */
  cass::HistogramBackend * loadDataFromHist(const QString &filename);

  /** save the data from a hist file
   *
   * @param filename the name of the file
   * @param data pointer to the data to save
   */
  void saveDataToHist(const QString &filename, cass::HistogramBackend *data);

  /** read the data from a CSV file
   *
   * @return pointer to the data
   * @param filename the name of the file
   */
  cass::HistogramBackend * loadDataFromCSV(const QString &filename);

  /** save the data from a CSV file
   *
   * @param filename the name of the file
   * @param data pointer to the data to save
   */
  void saveDataToCSV(const QString &filename, cass::HistogramBackend *data);

  /** read the data from a hdf5 file
   *
   * @return pointer to the data
   * @param filename the name of the file
   * @param key in case of a hdf5 file the key to the requested data. Default is ""
   */
  cass::HistogramBackend * loadDataFromH5(const QString &filename, const QString &key="");

  /** write the data to a hdf5 file
   *
   * The key under which name the data is saved in the file is given provided as
   * part of the data.
   *
   * @sa hdf5::Handler::open for options how the file will be opened
   *
   * @param filename the name of the file
   * @param pointer to the data
   * @param mode The mode in which the file will be opened. Default is "w".
   */
  void saveDataToH5(const QString &filename, cass::HistogramBackend *data, const QString& mode="w");

private:
  /** the filename of the file to work on */
  QString _filename;
};
}//end namespace jocassview
#endif
