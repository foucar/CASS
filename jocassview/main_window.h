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

signals:
  /** signal that "get data" was triggered */
  void get_data_triggered();

  /** signal that "get data" was triggered */
  void clear_histogram_triggered();

  /** signal that "get data" was triggered */
  void send_command_triggered();

  /** signal that "get data" was triggered */
  void reload_ini_triggered();

  /** signal that "get data" was triggered */
  void broadcast_darkcal_triggered();

  /** signal that "get data" was triggered */
  void broadcast_gaincal_triggered();

  /** signal that "get data" was triggered */
  void quit_server_triggered();

  /** signal that "get data" was triggered */
  void load_file_triggered(QString);

  /** signal that "get data" was triggered */
  void save_file_triggered(QString);

  /** signal that "get data" was triggered */
  void save_triggered();

  /** signal that "get data" was triggered */
  void overlay_data_triggered();

  /** signal that "get data" was triggered */
  void print_triggered();

  /** signal that "get data" was triggered */
  void server_changed(QString);

  /** signal that an item in the list has been checked
   *
   * @param itemName the name of the item that has been checked
   * @param state the state of the item
   */
  void item_checked(QString itemName,bool state);

  /** signal that "get data" was triggered */
  void runstatus_changed(int);

  /** signal that "get data" was triggered */
  void rate_changed(double);

public slots:
  /** change the status displayed by the LED
   *
   * @param status the new status that should be displayed
   */
  void change_status(int status);

  /** set the items in the list
   *
   * @param itemNames list with the names of the displayable items
   */
  void setDisplayableItems(QStringList itemNames);

  /** check an item in the list
   *
   * @param itemName the name of the item
   * @param state the state of the item
   */
  void setDisplayedItem(QString itemName, bool state);

private slots:
  /** display about this box */
  void on_about_triggered();

  /** open file dialog and emit filname */
  void on_load_triggered();

  /** open file dialog and emit filname */
  void on_save_as_triggered();

  /** react on when an item in the list has been clicked
   *
   * check the state of the item and emit the name of the item together with its
   * state.
   *
   * @param item The item that has been clicked
   */
  void on_listitem_clicked(QListWidgetItem *item);

  /** react when the server string or port has changed
   *
   * assemble the server string and port to a server address and emit
   * server_changed(); signal
   */
  void on_server_property_changed();

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
};

}//end namspace jocassview

#endif
