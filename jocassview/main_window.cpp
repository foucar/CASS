// Copyright (C) 2013 Lutz Foucar

/**
 * @file main_window.cpp contains the jocassview main window
 *
 * @author Lutz Foucar
 */

#include <QtCore/QSettings>
#include <QtCore/QSize>
#include <QtCore/QDir>
#include <QtCore/QString>
#include <QtCore/QStringList>

#include <QtGui/QMenuBar>
#include <QtGui/QMenu>
#include <QtGui/QToolBar>
#include <QtGui/QLineEdit>
#include <QtGui/QSpinBox>
#include <QtGui/QDoubleSpinBox>
#include <QtGui/QWidget>
#include <QtGui/QComboBox>
#include <QtGui/QCheckBox>
#include <QtGui/QLabel>
#include <QtGui/QStatusBar>
#include <QtGui/QDockWidget>
#include <QtGui/QApplication>
#include <QtGui/QMessageBox>
#include <QtGui/QAction>
#include <QtGui/QRadioButton>
#include <QtGui/QFileDialog>
#include <QtGui/QListWidget>
#include <QtGui/QMoveEvent>
#include <QtGui/QResizeEvent>
#include <QtGui/QInputDialog>

#include "main_window.h"

#include "status_led.h"

using namespace jocassview;

MainWindow::MainWindow(QWidget *parent, Qt::WFlags flags)
  : QMainWindow(parent,flags)
{
  QSettings settings;

  // Add a menu to the window
  QMenuBar *menu = menuBar();

  // Add file menu
  QMenu *fmenu = menu->addMenu(tr("&File"));
  fmenu->addAction(tr("Load Data"),this,SLOT(on_load_triggered()));
  fmenu->addAction(tr("Save"),this,SIGNAL(save_triggered()),QKeySequence(tr("F10")));
  fmenu->addAction(tr("Save as..."),this,SLOT(on_save_as_triggered()));
  fmenu->addAction(tr("Print"),this,SIGNAL(print_triggered()));
  fmenu->addSeparator();
  fmenu->addAction(tr("Quit"),qApp,SLOT(closeAllWindows()),QKeySequence(tr("Ctrl+q")));

  // Add control menu
  QMenu *cmenu = menu->addMenu(tr("&Control"));
  cmenu->addAction(tr("Refresh List"),this,SIGNAL(refresh_list_triggered()),QKeySequence(tr("F5")));
  cmenu->addAction(tr("Get Data"),this,SIGNAL(get_data_triggered()),QKeySequence(tr("Ctrl+i")));
  cmenu->addAction(tr("Clear Histogram"),this,SLOT(on_clear_histogram_triggered()));
  cmenu->addAction(tr("Send custom Command"),this,SLOT(on_send_command_triggered()));
  cmenu->addAction(tr("Reload ini File"),this,SIGNAL(reload_ini_triggered()),QKeySequence(tr("Ctrl+r")));
  cmenu->addAction(tr("Broadcast darkcal command"),this,SLOT(on_broadcast_darkcal_triggered()),QKeySequence(tr("Ctrl+d")));
  cmenu->addAction(tr("Broadcast gaincal command"),this,SLOT(on_broadcast_gaincal_triggered()),QKeySequence(tr("Ctrl+g")));
  cmenu->addSeparator();
  cmenu->addAction(tr("Quit Server"),this,SIGNAL(quit_server_triggered()));

  // Add help menu
  QMenu *hmenu = menu->addMenu(tr("&Help"));
  hmenu->addAction(tr("About"),this,SLOT(on_about_triggered()));
  hmenu->addAction(tr("About Qt"),qApp,SLOT(aboutQt()));

  // Add a toolbar where we can add the general tools
  QToolBar *toolBar(addToolBar(tr("Display control")));

  // Add servername and port to toolbar.
  _servername = new QLineEdit(settings.value("Servername", "server?").toString());
  _servername->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
  _servername->setToolTip(tr("Name of the server to connect to."));
  connect(_servername,SIGNAL(textEdited(QString)),this,SLOT(on_server_property_changed()));
  toolBar->addWidget(_servername);

  _serverport = new QSpinBox();
  _serverport->setKeyboardTracking(false);
  _serverport->setRange(1000, 50000);
  _serverport->setValue(settings.value("Serverport", 12321).toInt());
  _serverport->setToolTip(tr("Port of the server to connect to."));
  connect(_serverport,SIGNAL(valueChanged(int)),this,SLOT(on_server_property_changed()));
  toolBar->addWidget(_serverport);

  // Add spacer to toolbar.
  QWidget *spacer1(new QWidget());
  spacer1->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  toolBar->addWidget(spacer1);

  // Add a separator
  toolBar->addSeparator();

  // Add spacer to toolbar.
  QWidget *spacer2(new QWidget());
  spacer2->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  toolBar->addWidget(spacer2);

  // Add run control to toolbar.
  _autoUpdate = new QAction(QIcon(":images/auto_update.png"),tr("Toggle auto update"),toolBar);
  _autoUpdate->setCheckable(true);
  _autoUpdate->setChecked(settings.value("AutoUpdateOn",false).toBool());
  _autoUpdate->setToolTip(tr("If checked, continuously retrieve and display images."));
  connect(_autoUpdate,SIGNAL(triggered()),this,SLOT(on_autoupdate_changed()));
  toolBar->addAction(_autoUpdate);

  // Add status LED to toolbar.
  _statusLED = new StatusLED();
  _statusLED->setToolTip("Status indicator (green= , red= ).");
  _statusLED->setStatus(StatusLED::off);
  toolBar->addWidget(_statusLED);

  // Add rate to toolbar.
  _rate = new QDoubleSpinBox();
  _rate->setRange(0.01, 100.);
  _rate->setValue(settings.value("Rate", 10.).toDouble());
  _rate->setToolTip(tr("Image update frequency."));
  connect(_rate,SIGNAL(valueChanged(double)),this,SLOT(on_autoupdate_changed()));
  toolBar->addWidget(_rate);
  QLabel *punit = new QLabel;
  punit->setText("Hz");
  toolBar->addWidget(punit);

  // set up status bar
  statusBar()->setToolTip(tr("Actual frequency to get and display "
                          "images averaged over (n) times."));

  // Set a list item as central widget
  QListWidget *listview(new QListWidget(this));
  listview->setSelectionMode(QAbstractItemView::MultiSelection);
  connect(listview,SIGNAL(itemClicked(QListWidgetItem*)),this,SLOT(on_listitem_clicked(QListWidgetItem*)));
  setCentralWidget(listview);

  // Set the size and position of the window
  resize(settings.value("MainWindowSize",size()).toSize());
  move(settings.value("MainWindowPosition",pos()).toPoint());
}

MainWindow::~MainWindow()
{

}

double MainWindow::interval() const
{
  return (1000./rate());
}

double MainWindow::rate() const
{
  return _rate->value();
}

bool MainWindow::autoUpdate() const
{
  return _autoUpdate->isChecked();
}

QStringList MainWindow::displayableItems()const
{
  QStringList items;
  QListWidget *listwidget(dynamic_cast<QListWidget*>(centralWidget()));
  for (int i=0; i < listwidget->count(); ++i)
    items.append(listwidget->item(i)->text());
  return items;
}

QStringList MainWindow::selectedDisplayableItems()const
{
  QStringList items;
  QListWidget *listwidget(dynamic_cast<QListWidget*>(centralWidget()));
  QList<QListWidgetItem*> selected(listwidget->selectedItems());
  for (int i=0; i < selected.size(); ++i)
    items.append(selected[i]->text());
  return items;
}

void MainWindow::setLEDStatus(int status)
{
  _statusLED->setStatus(status);
}

void MainWindow::setDisplayableItems(QStringList itemNames)
{
  QListWidget *listwidget(dynamic_cast<QListWidget*>(centralWidget()));
  QStringList selectedItems(selectedDisplayableItems());
  listwidget->clear();
  listwidget->addItems(itemNames);
  listwidget->sortItems();
  for (int i=0; i < selectedItems.size(); ++i)
    setDisplayedItem(selectedItems[i],true,false);
}

void MainWindow::setDisplayedItem(QString item,bool state, bool simulateClickedSignal)
{
  QListWidget *listwidget(dynamic_cast<QListWidget*>(centralWidget()));
  QList<QListWidgetItem*> listwidgetitems(listwidget->findItems(item,Qt::MatchExactly));
  if (listwidgetitems.empty() || listwidgetitems.size() > 1)
    return;
  QListWidgetItem *listwidgetitem(listwidgetitems.front());
  listwidgetitem->setSelected(state);
  if(simulateClickedSignal)
    on_listitem_clicked(listwidgetitem);
}

void MainWindow::on_about_triggered()
{
  QMessageBox::about(this, tr("About jocassview"),
                     tr("<p>The <b>joCASSview</b> is a display client for the CASS software.</p>"));
}

void MainWindow::on_load_triggered()
{
  QString filter("Data Files (*.png *.tiff *.jpg *.jpeg *.gif *.bmp *.csv *.hst *.h5 *.hdf5)");
  QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), QDir::currentPath(), filter);
  if(!fileName.isEmpty())
    emit load_file_triggered(fileName);
}

void MainWindow::on_save_as_triggered()
{
  QString filter("Data Files (*.png *.tiff *.jpg *.jpeg *.gif *.bmp *.csv *.hst *.h5 *.hdf5)");
  QString fileName = QFileDialog::getSaveFileName(this, tr("Save Data to File"), QDir::currentPath(), filter);
  if(!fileName.isEmpty())
    emit save_file_triggered(fileName);
}

void MainWindow::on_listitem_clicked(QListWidgetItem *item)
{
  bool state(item->isSelected());
  QString name(item->text());
  emit item_checked(name,state);
}

QString MainWindow::on_server_property_changed()
{
  QString servername(_servername->text());
  QString serverport(QString::number(_serverport->value()));
  QSettings settings;
  settings.setValue("Servername",servername);
  settings.setValue("Serverport",serverport);
  QString serveraddress(servername + ":" + serverport);
  emit server_changed(serveraddress);
  return serveraddress;
}

void MainWindow::on_autoupdate_changed()
{
  QSettings settings;
  settings.setValue("Rate",rate());
  settings.setValue("AutoUpdateOn",autoUpdate());
  emit autoupdate_changed();
}

void MainWindow::on_broadcast_darkcal_triggered()
{
  emit broadcast_triggered("startDarkcal");
}

void MainWindow::on_broadcast_gaincal_triggered()
{
  emit broadcast_triggered("startGaincal");
}

void MainWindow::on_send_command_triggered()
{
  QStringList items(displayableItems());
  if (items.empty())
    return;
  bool ok(false);
  QString key(QInputDialog::getItem(this, QObject::tr("Select Key"),
                                    QObject::tr("Key:"), items, 0, false, &ok));
  if (!ok)
    return;
  QString command = QInputDialog::getText(this,tr("Command"),tr("Type command:"),
                                          QLineEdit::Normal,tr("Type command here"),&ok);
  if (!ok)
    return;
  emit send_command_triggered(key,command);
}

void MainWindow::on_clear_histogram_triggered()
{
  QStringList items(displayableItems());
  if (items.empty())
    return;
  bool ok(false);
  QString key(QInputDialog::getItem(this, QObject::tr("Select Key"),
                                    QObject::tr("Key:"), items, 0, false, &ok));
  if (!ok)
    return;
  emit clear_histogram_triggered(key);
}

void MainWindow::moveEvent(QMoveEvent *event)
{
  QSettings settings;
  settings.setValue("MainWindowPosition",event->pos());
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
  QSettings settings;
  settings.setValue("MainWindowSize",event->size());
}
