// Copyright (C) 2013 Lutz Foucar

/**
 * @file main_window.h contains the main window of jocassview
 *
 * @author Lutz Foucar
 */

#ifndef _MAIN_WINDOW_H_
#define _MAIN_WINDOW_H_

#include <QtCore/QMap>
#include <QtCore/QString>

#include <QtGui/QMainWindow>

class QComboBox;
class QStringList;

namespace cass
{
class Histogram0DFloat;
class Histogram1DFloat;
class Histogram2DFloat;
}//end namespace cass

namespace jocassview
{
class StatusLED;
class DataViewer;

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

  /** set the items in the combobox
   *
   * @return pointer to the combobox
   */
  void setDisplayableItems(QStringList itemNames);

  /** set the items in the combobox
   *
   * @return pointer to the combobox
   */
  void setDisplayedItem(QString item);

  /** display a 0d Item
   *
   * @param histogram the 0d histogram that should be displayed
   */
  void displayItem(cass::Histogram0DFloat * histogram);

  /** display a 1d Item
   *
   * @param histogram the 1d histogram that should be displayed
   */
  void displayItem(cass::Histogram1DFloat * histogram);

  /** display a 2d Item
   *
   * @param histogram the 2d histogram that should be displayed
   */
  void displayItem(cass::Histogram2DFloat * histogram);

private slots:
  /** display about this box */
  void on_about_triggered();

  /** open file dialog and emit filname */
  void on_load_triggered();

  /** open file dialog and emit filname */
  void on_save_as_triggered();

private:
  /** the combobox that contains the contents that can be displayed */
  QComboBox *_attachId;

  /** the status LED */
  StatusLED * _statusLED;

  /** the container for all opened viewers */
  QMap<QString,DataViewer*> _viewers;
};

}//end namspace jocassview

#endif
