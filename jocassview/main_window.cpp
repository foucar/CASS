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

#include "main_window.h"

#include "status_led.h"
#include "histogram.h"
#include "zero_d_viewer.h"
#include "one_d_viewer.h"
#include "two_d_viewer.h"

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
  fmenu->addAction(tr("Overlay Data"),this,SIGNAL(overlay_data_triggered()));
  fmenu->addAction(tr("Save"),this,SIGNAL(save_triggered()),QKeySequence(tr("F10")));
  fmenu->addAction(tr("Save as..."),this,SLOT(on_save_as_triggered()));
  fmenu->addAction(tr("Print"),this,SIGNAL(print_triggered()));
  fmenu->addSeparator();
  fmenu->addAction(tr("Quit"),qApp,SLOT(closeAllWindows()),QKeySequence(tr("Ctrl+q")));

  // Add control menu
  QMenu *cmenu = menu->addMenu(tr("&Control"));
  cmenu->addAction(tr("Get Data"),this,SIGNAL(get_data_triggered()),QKeySequence(tr("Ctrl+i")));
  cmenu->addAction(tr("Clear Histogram"),this,SIGNAL(clear_histogram_triggered()));
  cmenu->addAction(tr("Send custom Command"),this,SIGNAL(send_command_triggered()));
  cmenu->addAction(tr("Reload ini File"),this,SIGNAL(reload_ini_triggered()),QKeySequence(tr("Ctrl+r")));
  cmenu->addAction(tr("Broadcast darkcal command"),this,SIGNAL(broadcast_darkcal_triggered()),QKeySequence(tr("Ctrl+d")));
  cmenu->addAction(tr("Broadcast gaincal command"),this,SIGNAL(broadcast_gaincal_triggered()),QKeySequence(tr("Ctrl+g")));
  cmenu->addSeparator();
  cmenu->addAction(tr("Quit Server"),this,SIGNAL(quit_server_triggered()));

  // Add help menu
  QMenu *hmenu = menu->addMenu(tr("&Help"));
  hmenu->addAction(tr("About"),this,SLOT(on_about_triggered()));
  hmenu->addAction(tr("About Qt"),qApp,SLOT(aboutQt()));

  // Add a toolbar where we can add the general tools
  QToolBar *toolBar(addToolBar(tr("Display control")));

  // Add servername and port to toolbar.
  QLineEdit *servername(new QLineEdit(settings.value("Servername", "server?").toString()));
  servername->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
  servername->setToolTip(tr("Name of the server to connect to."));
  connect(servername,SIGNAL(textEdited(QString)),this,SIGNAL(servername_changed(QString)));
  toolBar->addWidget(servername);

  QSpinBox *serverport(new QSpinBox());
  serverport->setKeyboardTracking(false);
  serverport->setRange(1000, 50000);
  serverport->setValue(settings.value("Serverport", 12321).toInt());
  serverport->setToolTip(tr("Port of the server to connect to."));
  connect(serverport,SIGNAL(valueChanged(int)),this,SIGNAL(serverport_changed(int)));
  toolBar->addWidget(serverport);

  // Add spacer to toolbar.
  QWidget *spacer1(new QWidget());
  spacer1->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  toolBar->addWidget(spacer1);

  // Add Attachment identifier to toolbar.
  _attachId = new QComboBox();
  _attachId->setToolTip(tr("Attachment identifier."));
  _attachId->setDuplicatesEnabled(false);
  _attachId->setInsertPolicy(QComboBox::InsertAlphabetically);
  _attachId->setEditable(false);
  _attachId->installEventFilter(this);
  _attachId->setSizeAdjustPolicy(QComboBox::AdjustToContents);
  connect(_attachId,SIGNAL(currentIndexChanged(QString)),this,SIGNAL(item_to_display_changed(QString)));
  toolBar->addWidget(_attachId);

  // Add spacer to toolbar.
  QWidget *spacer2(new QWidget());
  spacer2->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  toolBar->addWidget(spacer2);

  // Add run control to toolbar.
  QCheckBox *running(new QCheckBox());
  running->setCheckState(Qt::Unchecked);
  running->setToolTip(tr("If checked, continuously retrieve and display images."));
  connect(running,SIGNAL(stateChanged(int)),this,SIGNAL(runstatus_changed(int)));
  toolBar->addWidget(running);

  // Add status LED to toolbar.
  _statusLED = new StatusLED();
  _statusLED->setToolTip("Status indicator (green= , red= ).");
  _statusLED->setStatus(StatusLED::off);
  toolBar->addWidget(_statusLED);

  // Add rate to toolbar.
  QDoubleSpinBox *rate(new QDoubleSpinBox());
  rate->setRange(0.01, 100.);
  rate->setValue(settings.value("Rate", 10.).toDouble());
  rate->setToolTip(tr("Image update frequency."));
  connect(rate,SIGNAL(valueChanged(double)),this,SIGNAL(rate_changed(double)));
  toolBar->addWidget(rate);
  QLabel *punit = new QLabel;
  punit->setText("Hz");
  toolBar->addWidget(punit);

  // set up status bar
  statusBar()->setToolTip(tr("Actual frequency to get and display "
                          "images averaged over (n) times."));

  // Add the data viewers
  _0DView = new ZeroDViewer(this);
  _1DView = new OneDViewer(this);
  _2DView = new TwoDViewer(this);

  // Set the size of the window
  QSize winsize(settings.value("WindowSize",QSize(800,800)).toSize());
  resize(winsize);
}

MainWindow::~MainWindow()
{

}

void MainWindow::on_about_triggered()
{
  QMessageBox::about(this, tr("About jocassview"), tr(
      "<p>The <b>joCASSview</b> is a display client for the CASS software.</p>"));
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

void MainWindow::change_status(int status)
{
  _statusLED->setStatus(status);
}

void MainWindow::setDisplayedItem(QString item)
{
  int itemIdx = _attachId->findText(item);
 _attachId->setCurrentIndex(itemIdx);
}

void MainWindow::setDisplayableItems(QStringList itemNames)
{
  _attachId->clear();
  _attachId->addItems(itemNames);
}

void MainWindow::displayItem(cass::Histogram0DFloat *histogram)
{
  _0DView->setData(histogram);
  if (centralWidget() != _0DView)
    setCentralWidget(_0DView);
}

void MainWindow::displayItem(cass::Histogram1DFloat *histogram)
{
  _1DView->setData(histogram);
  if (centralWidget() != _1DView)
    setCentralWidget(_1DView);
}

void MainWindow::displayItem(cass::Histogram2DFloat *histogram)
{
  _2DView->setData(histogram);
  if (centralWidget() != _2DView)
    setCentralWidget(_2DView);
}
