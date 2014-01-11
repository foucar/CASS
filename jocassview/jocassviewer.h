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
#include <QtCore/QStringList>

#include <QtGui/QMainWindow>

namespace cass
{
class HistogramBackend;
}//end namespace cass

class QLineEdit;
class QSpinBox;
class QDoubleSpinBox;
class QListWidgetItem;

namespace jocassview
{
class StatusLED;
class DataViewer;
class DataSource;

/** the jocassview class
 *
 * @author Lutz Foucar
 */
class JoCASSViewer : public QMainWindow
{
  Q_OBJECT

public:
  /** constructor
   *
   * create a main window, initialize the privates and set up the connections
   *
   * @param parent the parent widget. Default is 0
   * @param flags the flags
   */
  JoCASSViewer(QWidget *parent=0, Qt::WFlags flags=0);

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
  void openFile(QString filename=QString(), QString key=QString());

private slots:
  /** save the data displayed by all windows in the possible files
   *
   * generate the correct filenames and call saveFile() to save the data.
   *
   * @sa saveFile();
   */
  void autoSave() const;

  /** save a data from specific viewer or all viewers to the given filename
   *
   * @param filename The name of the file
   * @param keys The list of keys of the window whos contens should be written
   */
  void saveFile(QString filename=QString(),
                QStringList keys=QStringList()) const;

public slots:
  /** start the viewer
   *
   * uses on_autoupdate_changed() to initalize the autoupdate parameters.
   */
  void startViewer();

private slots:
  /** display about this box */
  void about();

  /** retrieve the rate as interval in ms
   *
   * convert the rate in hz to an interval in ms (1000/rate()) and return it.
   *
   * @return the interval in ms
   */
  double interval() const;

  /** retrieve the user set rate in Hz
   *
   * @return the rate in Hz
   */
  double rate() const;

  /** change the autoupdate based upon what the user set
   *
   * change the interval. If the autoupdate button is checked, start the timer.
   * Otherwise just stop the timer.
   */
  void changeAutoUpdate();

  /** update the contents within the viewers in the map */
  void updateViewers();

  /** react on when an item in the list has been checked
   *
   * @param item the item that has changed and by which the viewers will change
   */
  void changeViewers(QListWidgetItem *item);

  /** remove the viewer from the container when it has been destroyed
   *
   * @param obj The viewer object that has been destroyed
   */
  void removeViewer(DataViewer *obj);

  /** refresh the items on the displayable list
   *
   * retrieve the list of possible displayable items from the current source and
   * set them in the list.
   */
  void refreshDisplayableItemsList();

  /** check an item in the list
   *
   * find the listwidget item by the itemname. In case there is no or more than
   * one item with the requested name return without doing anything. Otherwise
   * change the checked state of the item to the requested state and call the
   * slot on_listitem_clicked() to ensure that it is the same behavious as if the
   * user clicked the item in the gui.
   *
   * @param itemName the name of the item
   * @param state the state of the item
   * @param emit Emit the clicked signal
   */
  void setDisplayedItem(QString itemName, bool state, bool simulateClickedSignal=true);

  /** retrieve a list with all items
   *
   * @return list with all items
   */
  QStringList displayableItems() const;

  /** retrieve a list with the selected items
   *
   * @return list with all selected items
   */
  QStringList displayedItems() const;

  /** react when print has been triggered
   *
   * ask the user which window should be printed, and then call print for the
   * requested window.
   */
  void print();

  /** react on when the source has been changed
   *
   * hide the server toolbar when the source is not the server, show it otherwise
   * Then refresh the list by calling refreshDisplayableItemsList()
   *
   * @param newSource the name of source that has been activated
   */
  void on_source_changed(QString newSource);

  /** retrieve the server address
   *
   * assemble the server string and port to a server address, save them in the
   * ini file and return and emit serverChanged signal
   */
  void changeServerAddress()const;

  /** broadcast the darkcal command */
  void broadcastDarkcalCommand()const;

  /** broadcast the gaincal command */
  void broadcastGaincalCommand()const;

  /** send a custom command
   *
   * ask to whom the command should be sent to and for the command to send and
   * tell the server to send the command.
   */
  void sendCustomCommand()const;

  /** send clear histograms
   *
   * ask the user which postprocessors histograms should be cleared and tell
   * the server to clear the requested histogram
   */
  void clearHistogram()const;

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

protected:
  /** receive move events to store the current position to the settings
   *
   * @param event the move event
   */
  void moveEvent(QMoveEvent *event);

  /** receive resize events to store the current size to the settings
   *
   * @param event the resize event
   */
  void resizeEvent(QResizeEvent *event);

private:
  /** the status LED */
  StatusLED * _statusLED;

  /** the servername input widget */
  QLineEdit *_servername;

  /** the server port input widget */
  QSpinBox *_serverport;

  /** the rate input */
  QDoubleSpinBox *_rate;

  /** the auto update input */
  QAction *_autoUpdate;

  /** the toolbar with the server options */
  QToolBar *_serverToolBar;

  /** the container for all opened viewers */
  QMap<QString,DataViewer*> _viewers;

  /** timer for the auto update function it is used as singleshot timer */
  QTimer _updateTimer;

  /** flag to tell whether an update is in progess */
  bool _updateInProgress;
};
}//end namspace jocassview

#endif
