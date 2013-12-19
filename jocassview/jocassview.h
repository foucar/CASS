// Copyright (C) 2013 Lutz Foucar

/**
 * @file jocassview.h contains the jocassviewer class
 *
 * @author Lutz Foucar
 */

#ifndef _JOCASSVIEW_
#define _JOCASSVIEW_

#include <QtCore/QObject>

class QString;

namespace jocassview
{
class MainWindow;

/** the jocassview class
 *
 * @author Lutz Foucar
 */
class JoCASSViewer : public QObject
{
  Q_OBJECT

public:
  /** constructor
   *
   */
  JoCASSViewer(QObject *parent=0);

  /** destructor
   *
   */
  ~JoCASSViewer();

public slots:
  /** load data from a file
   *
   * @param filename the file to load the data from
   * @param key the key of the datafield in case its an h5 file
   */
  void loadData(QString filename, QString key="");

  /** load a specific key from the given h5 file
   *
   * @param key the key to load from the file
   */
  void loadH5KeyFromFile(QString key);

private:
  /** load data from image file
   *
   * @param filename the file to load the data from
   */
  void loadDataFromImage(const QString& filename);

  /** load data from histogram binary file
   *
   * @param filename the file to load the data from
   */
  void loadDataFromHist(const QString& filename);

  /** load data from comma separated values csv file
   *
   * @param filename the file to load the data from
   */
  void loadDataFromCSV(const QString& filename);

  /** load data from image file
   *
   * @param filename the file to load the data from
   * @param key the key of the datafield in case its an h5 file
   */
  void loadDataFromH5(const QString& filename, const QString &key="");

private:
  /** the main window of the jocassviewer */
  MainWindow * _mw;

  /** the current data filename */
  QString _filename;
};
}//end namspace jocassview

#endif
