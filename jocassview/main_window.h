// Copyright (C) 2013 Lutz Foucar

/**
 * @file main_window.h contains the main window of jocassview
 *
 * @author Lutz Foucar
 */

#ifndef _MAIN_WINDOW_H_
#define _MAIN_WINDOW_H_


#include <QtGui/QMainWindow>

class QComboBox;
class QString;
class QStringList;
class QListWidgetItem;
class QLineEdit;
class QSpinBox;
class QDoubleSpinBox;
class QAction;

namespace jocassview
{
class StatusLED;

/** the main window of jocassview
 *
 * @author Lutz Foucar
 */
class MainWindow : public QMainWindow
{
Q_OBJECT
public:
  /** constructor
   *
   * set up the main window
   *
   * @param parent the parent window
   * @param flags the flags
   */
  MainWindow(QWidget *parent=0, Qt::WFlags flags =0);

  /** destructor */
  ~MainWindow();

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

  /** retrieve the auto update status
   *
   * @return true when auto update has been selected
   */
  bool autoUpdate() const;

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

signals:
  /** signal that "refresh list" was triggered */
  void refresh_list_triggered();

  /** signal that "get data" was triggered */
  void get_data_triggered();

  /** signal that "reload ini" was triggered */
  void reload_ini_triggered();

  /** signal that "quit server" was triggered */
  void quit_server_triggered();

  /** signal that load file was triggered
   *
   * @param filename The filename of the file that should be loaded
   */
  void load_file_triggered(QString filename);

  /** signal that save file was triggered
   *
   * @param filename the name of the file where the data should be saved to
   */
  void save_file_triggered(QString filename);

  /** signal that "save" was triggered */
  void save_triggered();

  /** signal that "print" was triggered */
  void print_triggered();

  /** signal that "one of the server inputs" was changed
   *
   * @param serverstring the new server string
   */
  void server_changed(QString serverstring);

  /** signal that an item in the list has been checked
   *
   * @param itemName the name of the item that has been checked
   * @param state the state of the item
   */
  void item_checked(QString itemName, bool state);

  /** signal that something related to the auto update changed*/
  void autoupdate_changed();

  /** signal the broadcast command
   *
   * @param command the command that should be broadcasted
   */
  void broadcast_triggered(QString command);

  /** signal that a command should be sent to a specific postprocessor
   *
   * @param name the name of the postprocessor to sent the signal to
   * @param command the command to sent
   */
  void send_command_triggered(QString name, QString command);

  /** signal that tells which postprocessors histogram should be cleared
   *
   * @param name the name of the postprocesor whos histogram should be cleared
   */
  void clear_histogram_triggered(QString name);

public slots:
  /** change the status displayed by the LED
   *
   * @param status the new status that should be displayed
   */
  void setLEDStatus(int status);

  /** set the items in the list
   *
   * @param itemNames list with the names of the displayable items
   */
  void setDisplayableItems(QStringList itemNames);

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

private slots:
  /** display about this box */
  void on_about_triggered();

  /** open the openfile dialog and emit filename */
  void on_load_triggered();

  /** open the getsavefile dialog and emit filename */
  void on_save_as_triggered();

  /** react on when an item in the list has been clicked
   *
   * check the state of the item and emit the name of the item together with its
   * state.
   *
   * @param item The item that has been clicked
   */
  void on_listitem_clicked(QListWidgetItem *item);

public slots:
  /** react when the server string or port has changed
   *
   * assemble the server string and port to a server address and emit
   * server_changed(); signal
   */
  QString on_server_property_changed();

private slots:
  /** react when something related to auto updates has changed
   *
   * write the current settings to the ini file and emit the autoupdate_changed()
   * signal
   */
  void on_autoupdate_changed();

  /** react when broadcast darkal has been clicked
   *
   * emit the broadcast signal with "startDarkcal" as string
   */
  void on_broadcast_darkcal_triggered();

  /** react when broadcast gaincal has been clicked
   *
   * emit the broadcast signal with "startGaincal" as string
   */
  void on_broadcast_gaincal_triggered();

  /** react when send command was triggered
   *
   * ask to whom the command should be sent to and for the command to send, then
   * emit the send_command_triggered(qstring,qstring) signal.
   */
  void on_send_command_triggered();

  /** react when clear histogram was triggered
   *
   * ask the user which postprocessors histograms should be cleared and then emit
   * the clear_histogram(QString) signal
   */
  void on_clear_histogram_triggered();

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
};

}//end namspace jocassview

#endif
