// Copyright (C) 2013 Lutz Foucar

/**
 * @file jocassviewer.h contains the jocassviewer class
 *
 * @author Lutz Foucar
 */

#ifndef _JOCASSVIEW_
#define _JOCASSVIEW_

#include <QtCore/QObject>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QTimer>

namespace cass
{
class HistogramBackend;
}//end namespace cass


namespace jocassview
{
class MainWindow;
class DataViewer;
class DataSource;

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

  /** start the viewer
   *
   * uses on_autoupdate_changed() to initalize the autoupdate parameters.
   */
  void startViewer();

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

  /** update the contents within the viewers in the map */
  void update_viewers();

  /** react on the when something related to auto update changed
   *
   * change the interval. If the autoupdate button is checked, start the timer.
   * Otherwise just stop the timer.
   */
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

  /** react on when refresh list has been triggered
   *
   * retrieve the list of possible displayable items from the current source and
   * the list.
   */
  void on_refresh_list_triggered();

  /** react when print has been triggered
   *
   * ask the user which window should be printed, and then call print for the
   * requested window.
   */
  void on_print_triggered();

private:
  /** convenience function to create a viewer thats appropriate for a given type
   *  of data
   *
   * @param view iterator to the item in the viewer list where the viewer needs
   *        to be created.
   * @param hist pointer to the histogram that hold the data
   */
  void createViewerForType(QMap<QString,DataViewer*>::iterator view,
                           cass::HistogramBackend *hist);

  /** retrieve the current active source
   *
   * @return pointer to the current active source
   */
  DataSource* currentSource();

private:
  /** the main window of the jocassviewer */
  MainWindow *_mw;

  /** the container for all opened viewers */
  QMap<QString,DataViewer*> _viewers;

  /** container for all data sources */
  QMap<QString,DataSource*> _sources;

  /** key of the currently active data source */
  QString _currentSourceType;

  /** timer for the auto update function it is used as singleshot timer */
  QTimer _updateTimer;

  /** flag to tell whether an update is in progess */
  bool _updateInProgress;
};
}//end namspace jocassview

#endif
