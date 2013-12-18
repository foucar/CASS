// Copyright (C) 2013 Lutz Foucar

/**
 * @file main_window.h contains the main window of jocassview
 *
 * @author Lutz Foucar
 */

#ifndef _MAIN_WINDOW_H_
#define _MAIN_WINDOW_H_

#include<QtGui/QMainWindow>

class QComboBox;

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
   * set up the main window from the ui file and then adds the general input
   * things to the toolbar and the current rate lable to the statusbar.
   *
   * @param parent the parent window
   * @param flags the flags
   */
  MainWindow(QWidget *parent=0, Qt::WFlags flags =0);

  /** destructor */
  ~MainWindow();

public:
  /** access the combobox
   *
   * @return pointer to the combobox
   */
  QComboBox *displayableItems() {return _attachId;}

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
  void open_triggered();

  /** signal that "get data" was triggered */
  void save_triggered();

  /** signal that "get data" was triggered */
  void save_autofilename_triggered();

  /** signal that "get data" was triggered */
  void overlay_data_triggered();

  /** signal that "get data" was triggered */
  void print_triggered();

  /** signal that "get data" was triggered */
  void servername_changed(QString);

  /** signal that "get data" was triggered */
  void serverport_changed(int);

  /** signal that "get data" was triggered */
  void item_to_display_changed(QString);

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

private slots:
  /** display about this box */
  void on_about_triggered();

private:
  /** the combobox that contains the contents that can be displayed */
  QComboBox *_attachId;

  /** the status LED */
  StatusLED * _statusLED;
};

}//end namspace jocassview

#endif
