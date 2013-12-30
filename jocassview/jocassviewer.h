// Copyright (C) 2013 Lutz Foucar

/**
 * @file jocassview.h contains the jocassviewer class
 *
 * @author Lutz Foucar
 */

#ifndef _JOCASSVIEW_
#define _JOCASSVIEW_

#include <QtCore/QObject>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QTimer>

#include "tcpclient.h"

namespace jocassview
{
class MainWindow;
class DataViewer;

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
   * create a main window and set up the connections
   *
   * @param parent the parent widget. Default is 0
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

private slots:
  /** react on when an item in the list has been checked
   *
   * @param key the name of the checked item
   * @param state the state of the checked item
   */
  void on_displayitem_checked(QString key,bool state);

  /** remove the viewer from the container when it has been destroyed
   *
   * @param obj The viewer object that has been destroyed
   */
  void on_viewer_destroyed(DataViewer *obj);

  /** update the contents within the viewers */
  void update_viewers();

  /** react on the when something related to auto update changed */
  void on_autoupdate_changed();

  /** react on when "save" has been triggered
   *
   * go through all opend windows and generate an appropriate filename with the
   * date and the key of the data. Then use the saveFile(QString fname, QString key)
   * slot to acutally save the data.
   */
  void on_autosave_triggered() const;

  /** save a data from key to the given filename
   *
   * save the data with the given key to a given file. If no key is given and the
   * file is not a container type (e.g. hdf5) file, then ask the user which of
   * the available keys should be written. Write the data of all opened windows
   * in the container type file.
   *
   * @param filename The name of the file
   * @param key The key of the window whos contens should be written
   */
  void saveFile(const QString &filename, const QString &key="") const;

private:
  /** the main window of the jocassviewer */
  MainWindow *_mw;

  /** the current data filename */
  QString _filename;

  /** the container for all opened viewers */
  QMap<QString,DataViewer*> _viewers;

  /** timer for the auto update function */
  QTimer _updateTimer;

  /** the client to connect to the cass server */
  TCPClient _client;
};
}//end namspace jocassview

#endif
