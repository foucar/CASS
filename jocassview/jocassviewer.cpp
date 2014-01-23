// Copyright (C) 2013 Lutz Foucar

/**
 * @file jocassviewer.cpp contains the jocassviewer
 *
 * @author Lutz Foucar
 */

#include <QtCore/QFileInfo>
#include <QtCore/QDebug>
#include <QtCore/QSettings>
#include <QtCore/QDir>
#include <QtCore/QDateTime>
#include <QApplication>

#include <QtGui/QMessageBox>
#include <QtGui/QInputDialog>
#include <QtGui/QMenuBar>
#include <QtGui/QMenu>
#include <QtGui/QToolBar>
#include <QtGui/QLineEdit>
#include <QtGui/QSpinBox>
#include <QtGui/QDoubleSpinBox>
#include <QtGui/QLabel>
#include <QtGui/QStatusBar>
#include <QtGui/QListWidget>
#include <QtGui/QFileDialog>

#include "jocassviewer.h"

#include "histogram.h"
#include "file_handler.h"
#include "zero_d_viewer.h"
#include "one_d_viewer.h"
#include "two_d_viewer.h"
#include "status_led.h"
#include "data_source.h"
#include "data.h"
#include "tcpclient.h"
#include "data_source_manager.h"

using namespace jocassview;
using namespace cass;
using namespace std;


JoCASSViewer::JoCASSViewer(QWidget *parent, Qt::WFlags flags)
  : QMainWindow(parent,flags),
    _updateInProgress(false)
{
  QSettings settings;
  TCPClient *client(new TCPClient);

  /** set up the window */
  // Add a menu to the window
  QMenuBar *menu = menuBar();

  // Add file menu
  QMenu *fmenu = menu->addMenu(tr("&File"));
  fmenu->addAction(QIcon::fromTheme("document-open"),tr("Load Data"),this,
                   SLOT(openFile()),QKeySequence(QKeySequence::Open))->setShortcutContext(Qt::ApplicationShortcut);
  fmenu->addAction(QIcon::fromTheme("document-save"),tr("Save"),this,
                   SLOT(autoSave()),QKeySequence(tr("F10")))->setShortcutContext(Qt::ApplicationShortcut);
  fmenu->addAction(QIcon::fromTheme("document-save-as"),tr("Save as..."),this,
                   SLOT(saveFile()),QKeySequence(QKeySequence::SaveAs))->setShortcutContext(Qt::ApplicationShortcut);
  fmenu->addAction(QIcon::fromTheme("document-print"),tr("Print"),this,
                   SLOT(print()),QKeySequence(QKeySequence::Print))->setShortcutContext(Qt::ApplicationShortcut);
  fmenu->addSeparator();
  fmenu->addAction(QIcon::fromTheme("application-exit"),tr("Quit"),qApp,
                   SLOT(closeAllWindows()),QKeySequence(QKeySequence::Quit))->setShortcutContext(Qt::ApplicationShortcut);

  // Add control menu
  QMenu *cmenu = menu->addMenu(tr("&Control"));
  cmenu->addAction(tr("Refresh List"),this,
                   SLOT(refreshDisplayableItemsList()),QKeySequence(tr("F5")))->setShortcutContext(Qt::ApplicationShortcut);
  cmenu->addAction(tr("Get Data"),this,
                   SLOT(updateViewers()),QKeySequence(tr("Ctrl+i")))->setShortcutContext(Qt::ApplicationShortcut);
  cmenu->addAction(tr("Clear Histogram"),this,
                   SLOT(clearHistogram()));
  cmenu->addAction(tr("Send custom Command"),this,
                   SLOT(sendCustomCommand()));
  cmenu->addAction(tr("Broadcast darkcal command"),this,
                   SLOT(broadcastDarkcalCommand()),QKeySequence(tr("Ctrl+d")))->setShortcutContext(Qt::ApplicationShortcut);
  cmenu->addAction(tr("Broadcast gaincal command"),this,
                   SLOT(broadcastGaincalCommand()),QKeySequence(tr("Ctrl+g")))->setShortcutContext(Qt::ApplicationShortcut);
  cmenu->addSeparator()->setText("Server Control");
  cmenu->addAction(tr("Reload ini File"),client,
                   SLOT(reloadIni()),QKeySequence(tr("Ctrl+r")))->setShortcutContext(Qt::ApplicationShortcut);
  cmenu->addAction(tr("Quit Server"),client,SLOT(quitServer()));

  // Add the source menu
  DataSourceManager::setMenu(menu->addMenu(tr("&Sources")));

  // Add help menu
  QMenu *hmenu = menu->addMenu(tr("&Help"));
  hmenu->addAction(tr("About"),this,SLOT(about()));
  hmenu->addAction(tr("About Qt"),qApp,SLOT(aboutQt()));

  // Add a toolbar where we can add the general tools
  _serverToolBar = addToolBar(tr("Display control"));
  _serverToolBar->setContextMenuPolicy(Qt::PreventContextMenu);

  // Add servername and port to toolbar.
  _servername = new QLineEdit(settings.value("Servername", "localhost").toString());
  _servername->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
  _servername->setToolTip(tr("Name of the server to connect to."));
  connect(_servername,SIGNAL(textEdited(QString)),this,SLOT(changeServerAddress()));
  _serverToolBar->addWidget(_servername);

  _serverport = new QSpinBox();
  _serverport->setKeyboardTracking(false);
  _serverport->setRange(1000, 50000);
  _serverport->setValue(settings.value("Serverport", 12321).toInt());
  _serverport->setToolTip(tr("Port of the server to connect to."));
  connect(_serverport,SIGNAL(valueChanged(int)),this,SLOT(changeServerAddress()));
  _serverToolBar->addWidget(_serverport);

  // Add spacer to toolbar.
  QWidget *spacer1(new QWidget());
  spacer1->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  _serverToolBar->addWidget(spacer1);

  // Add a separator
  _serverToolBar->addSeparator();

  // Add spacer to toolbar.
  QWidget *spacer2(new QWidget());
  spacer2->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  _serverToolBar->addWidget(spacer2);

  // Add run control to toolbar.
  _autoUpdate = new QAction(QIcon(":images/auto_update.png"),tr("Toggle auto update"),_serverToolBar);
  _autoUpdate->setCheckable(true);
  _autoUpdate->setChecked(settings.value("AutoUpdateOn",false).toBool());
  _autoUpdate->setToolTip(tr("If checked, continuously retrieve and display images."));
  connect(_autoUpdate,SIGNAL(triggered()),
          this,SLOT(changeAutoUpdate()));
  _serverToolBar->addAction(_autoUpdate);

  // Add status LED to toolbar.
  _statusLED = new StatusLED();
  _statusLED->setToolTip("Status indicator (green = Data retrieved ok, red = communciation Problems, yellow = busy).");
  _statusLED->setStatus(StatusLED::off);
  _serverToolBar->addWidget(_statusLED);

  // Add rate to toolbar.
  _rate = new QDoubleSpinBox();
  _rate->setRange(0.01, 100.);
  _rate->setValue(settings.value("Rate", 10.).toDouble());
  _rate->setToolTip(tr("Image update frequency."));
  connect(_rate,SIGNAL(valueChanged(double)),
          this,SLOT(changeAutoUpdate()));
  _serverToolBar->addWidget(_rate);
  QLabel *punit = new QLabel;
  punit->setText("Hz");
  _serverToolBar->addWidget(punit);

  // set up status bar
  statusBar()->setToolTip(tr("Actual frequency to get and display "
                          "images averaged over (n) times."));

  // Set a list item as central widget
  QListWidget *listview(new QListWidget(this));
  listview->setSelectionMode(QAbstractItemView::MultiSelection);
  connect(listview,SIGNAL(itemClicked(QListWidgetItem*)),
          this,SLOT(changeViewers(QListWidgetItem*)));
  setCentralWidget(listview);

  /** use the timer as single shot */
  _updateTimer.setSingleShot(true);
  connect(&_updateTimer,SIGNAL(timeout()),this,SLOT(updateViewers()));

  /** initialize the server data source and add them to the manager */
  DataSourceManager::addSource("Server",client);
  changeServerAddress();
  connect(DataSourceManager::instance(),SIGNAL(sourceChanged(QString)),
          this,SLOT(on_source_changed(QString)));

  // Set the size and position of the window
  resize(settings.value("MainWindowSize",size()).toSize());
  move(settings.value("MainWindowPosition",pos()).toPoint());

  show();
}

JoCASSViewer::~JoCASSViewer()
{

}

void JoCASSViewer::openFile(QString filename, QString key)
{
  /** if no filename is given ask for a file */
  if (filename.isEmpty())
  {
    QString filter("Data Files (*.csv *.hst *.h5 *.hdf5)");
    filename = QFileDialog::getOpenFileName(this, tr("Open File"),
                                            QDir::currentPath(), filter);
    if(filename.isEmpty())
      return;
  }

  /** add a source with the requested file and set the window title to the
   *  filename
   */
  DataSourceManager::addSource(filename,new FileHandler(filename));
  setWindowTitle(QFileInfo(filename).baseName());

  /** in case the key is on the list of displayable items, display it */
  if (displayableItems().contains(key))
    setDisplayedItem(key,true);
}

void JoCASSViewer::autoSave() const
{
  if (_viewers.isEmpty())
    return;
  /** generate the general automatic filename */
  QString fileNameBase(QDir::currentPath() + "/" + QDateTime::currentDateTime().toString() + "_");

  /** save all open windows data to a single container file */
  saveFile(QString(fileNameBase + "autoSave.h5"),displayedItems());

  /** save the individual viewers to their specific savable file types
   *  (exclude the container files type
   */
  QMap<QString,DataViewer*>::const_iterator view(_viewers.constBegin());
  while( view != _viewers.constEnd())
  {
    if (view.value())
    {
      QStringList filetypes(view.value()->dataFileSuffixes());
      QStringList::const_iterator cIt;
      for (cIt = filetypes.constBegin(); cIt != filetypes.constEnd(); ++cIt)
      {
        QString fname(fileNameBase + view.key() + "." + *cIt);
        if (!FileHandler::isContainerFile(fname))
          saveFile(fname,QStringList(view.key()));
      }
    }
    ++view;
  }
}

void JoCASSViewer::saveFile(QString filename, QStringList keys) const
{
  if (_viewers.isEmpty())
    return;

  /** if not filename was given ask for one */
  if (filename.isEmpty())
  {
    QString filter("Data Files (*.png *.csv *.hst *.h5 *.hdf5)");
    filename = QFileDialog::getSaveFileName(0, tr("Save Data to File"),
                                            QDir::currentPath(), filter);
    if(filename.isEmpty())
      return;
  }

  /** if no keys are given, request at least one using the iteminput dialog
   *  The preselected item in the dialog should be the currently highlighted
   *  window or, in case of a container file, "all"
   */
  if (keys.isEmpty())
  {
    QStringList items(displayedItems());
    items.prepend("**ALL**");
    QWidget *focusWiget(QApplication::focusWidget());
    int preselectItemId(items.indexOf("**ALL**"));
    if (!FileHandler::isContainerFile(filename) && (focusWiget))
      preselectItemId = items.indexOf(focusWiget->windowTitle());
    bool ok(false);
    QString item(QInputDialog::getItem(0, QObject::tr("Select Key"),
                                       QObject::tr("Key:"), items,
                                       preselectItemId, false, &ok));
    if (!ok || item.isEmpty())
      return;
    if (item.contains("***ALL***"))
      keys = displayedItems();
    else
      keys.append(item);
  }

  /** if the file is a container file, create the container first before adding
   *  data to it.
   */
  if (FileHandler::isContainerFile(filename))
    FileHandler::createContainer(filename);

  /** go through the list and tell the viewer to save the data
   *  Append the name of the viewer to the filaname in case this is not a
   *  container file and more that one file should be saved with data so that
   *  the files are not overwritten.
   */
  QStringList::const_iterator cIt;
  for (cIt = keys.constBegin(); cIt != keys.constEnd(); ++cIt)
  {
    if (_viewers.value(*cIt))
    {
      QString fname(filename);
      if(!FileHandler::isContainerFile(filename) && keys.size() > 1)
        fname.insert(fname.lastIndexOf("."),"_" + *cIt);
      _viewers.value(*cIt)->saveData(fname);
    }
  }
}

void JoCASSViewer::startViewer()
{
  changeAutoUpdate();
}

void JoCASSViewer::about()
{
  QMessageBox::about(this, tr("About jocassview"),
                     tr("<p>The <b>JoCASSviewer</b> is a display client for the CASS software.</p>"));
}

double JoCASSViewer::interval() const
{
  return (1000./rate());
}

double JoCASSViewer::rate() const
{
  return _rate->value();
}

void JoCASSViewer::changeAutoUpdate()
{
  qDebug()<<"changeAutoUpdate: "<<interval()<<_autoUpdate->isChecked();
  _updateTimer.setInterval(interval());
  if(_autoUpdate->isChecked())
    _updateTimer.start();
  else
    _updateTimer.stop();
  QSettings settings;
  settings.setValue("Rate",rate());
  settings.setValue("AutoUpdateOn",_autoUpdate->isChecked());
}

void JoCASSViewer::updateViewers()
{
  /** if another process is still updating return here
   *  @note this can happen, because while retrieving data from the server all
   *        pending processes on the eventloop will be processed. One of them
   *        could be the user trying to retrieve data another time (while another
   *        retrieval process is still ongoing, thus resulting this function will
   *        be reentered, even though it is still working.
   */
  if (_updateInProgress)
    return;

  if (_viewers.isEmpty())
    return;

  qDebug()<<"update viewers";
  _updateInProgress = true;
  _statusLED->setStatus(StatusLED::busy);
  bool sucess(true);

  /** get an iterator to go through the map and retrieve the first item where
   *  we get the id from. Then check whether all the other histograms should have
   *  the same id (if not then set the id to 0).
   *  Remember how big the container is for validating whether nothing has changed
   *  while the data was retrieved from the source.
   */
  QMap<QString,DataViewer*>::iterator view(_viewers.begin());
//  cass::HistogramBackend *hist(_client.getData(view.key()));
//  const quint64 eventID = hist && false ? hist->id() : 0;
  const quint64 eventID(0);
  const int nbrWindows(_viewers.size());
  while( view != _viewers.end())
  {
    if (!view.value())
    {
      /** check if current source is available, if remove the viewer from the
       *  list and quit updating
       */
      QString sourceName(DataSourceManager::currentSourceName());
      DataSource *source(DataSourceManager::source());
      if (!source)
      {
        setDisplayedItem(view.key(),false,false);
        _viewers.remove(view.key());
        sucess = false;
        break;
      }
      /** if the viewer hasn't been initalized, initialize it with new result
       *  from the current active source.
       */
      HistogramBackend * result(source->result(view.key(),eventID));
      /** validate container consistency */
      if(_viewers.size() != nbrWindows)
      {
        sucess = false;
        break;
      }
      /** validate result.
       *  If the viewer can't be initialzed, remove it from the list
       */
      if (!result)
      {
        qDebug()<<"result is empty "<<view.key();
        setDisplayedItem(view.key(),false,false);
        _viewers.remove(view.key());
        break;
      }
      /** Set the result to the data of the viewer and let the data now what
       *  source type it has been filled with
       */
      createViewerForType(view,result);
      /** validate data */
      if (!view.value()->data().isEmpty())
      {
        view.value()->data().front()->setResult(result);
        view.value()->data().front()->setSourceName(sourceName);
      }
    }
    else
    {
      /** otherwise retrieve all the data containers from a viewer and update
       *  them with the latest data
       */
      qDebug()<<"update existing viewer"<<view.key();
      QList<Data*> data(view.value()->data());
      const int nbrData(data.size());
      QList<Data*>::iterator dataIt(data.begin());
      while (dataIt != data.end())
      {
        /** validate source */
        QString sourceName((*dataIt)->sourceName());
        DataSource *source(DataSourceManager::source(sourceName));
        if(!source)
        {
          qDebug()<<"source doesnt exist"<<sourceName;
          continue;
        }
        /** validate result to update */
        if (!(*dataIt)->result())
        {
          qDebug()<<"result is empty"<<sourceName;
          continue;
        }
        const QString key(QString::fromStdString((*dataIt)->result()->key()));
        HistogramBackend * result(source->result(key,eventID));
        /** validate container consistency */
        if(_viewers.size() != nbrWindows || data.size() != nbrData)
        {
          sucess = false;
          break;
        }
        (*dataIt)->setResult(result);
        ++dataIt;
      }
    }
    /** tell the viewer the data has changed */
    view.value()->dataChanged();
    ++view;
  }
  /** set the report to sucess or failure */
  sucess ? _statusLED->setStatus(StatusLED::ok) :
           _statusLED->setStatus(StatusLED::fail);

  /** restart the updatetimer when requested and reset the in progress flag */
  changeAutoUpdate();
  _updateInProgress = false;
}

void JoCASSViewer::changeViewers(QListWidgetItem *item)
{
  bool state(item->isSelected());
  QString name(item->text());

  if (state)
  {
    /** if the container already has a viewer with the requested name, exit here */
    if (_viewers.contains(name))
      return;
    /** create an entry in the viewers container with a 0 pointer and initialize
     *  the viewer based upon the type of data using update_viewers();
     */
    _viewers[name] = 0;
    updateViewers();
  }
  else
  {
    /** if the key is on the list of viewers and the viewer has been created
     *  close it (which will delete the window, because all dataviewer windows
     *  have the delete on close flag set)
     */
    if (_viewers.contains(name) && _viewers[name])
      _viewers[name]->close();
  }
}

void JoCASSViewer::removeViewer(DataViewer *obj)
{
  /** retrieve the window title from the dataviewer that is beeing destroyed
   *  (because its the key in the list of viewers) and remove the key from the
   *  list. Then set the entry in the list as not highlighted.
   */
  QString key(obj->windowTitle());
  _viewers.remove(key);
  setDisplayedItem(key,false);
}

void JoCASSViewer::refreshDisplayableItemsList()
{
  qDebug()<<"on_refresh_list_triggered";
  DataSource *source(DataSourceManager::source());
  if (!source)
    return;
  QListWidget *listwidget(dynamic_cast<QListWidget*>(centralWidget()));
  QStringList selectedItems(displayedItems());
  listwidget->clear();
  listwidget->addItems(source->resultNames());
  listwidget->sortItems();
  for (int i=0; i < selectedItems.size(); ++i)
    setDisplayedItem(selectedItems[i],true,false);
}

void JoCASSViewer::setDisplayedItem(QString item,bool state, bool simulateClickedSignal)
{
  QListWidget *listwidget(dynamic_cast<QListWidget*>(centralWidget()));
  QList<QListWidgetItem*> listwidgetitems(listwidget->findItems(item,Qt::MatchExactly));
  if (listwidgetitems.empty() || listwidgetitems.size() > 1)
    return;
  QListWidgetItem *listwidgetitem(listwidgetitems.front());
  listwidgetitem->setSelected(state);
  if(simulateClickedSignal)
    changeViewers(listwidgetitem);
}

QStringList JoCASSViewer::displayableItems()const
{
  QStringList items;
  QListWidget *listwidget(dynamic_cast<QListWidget*>(centralWidget()));
  for (int i=0; i < listwidget->count(); ++i)
    items.append(listwidget->item(i)->text());
  return items;
}

QStringList JoCASSViewer::displayedItems()const
{
  QStringList items;
  QListWidget *listwidget(dynamic_cast<QListWidget*>(centralWidget()));
  QList<QListWidgetItem*> selected(listwidget->selectedItems());
  for (int i=0; i < selected.size(); ++i)
    items.append(selected[i]->text());
  return items;
}

void JoCASSViewer::print()
{
  QStringList items(displayedItems());
  QWidget *focusWiget(QApplication::focusWidget());
  QString preselectItem;
  if (focusWiget)
    preselectItem=focusWiget->windowTitle();
  bool ok(false);
  QString item(QInputDialog::getItem(this, QObject::tr("Select Key"),
                                     QObject::tr("Print Key:"), items,
                                     items.indexOf(preselectItem), false, &ok));
  if (!ok)
    return;

  if(!_viewers.contains(item))
    return;

  _viewers.value(item)->print();
}

void JoCASSViewer::on_source_changed(QString newSource)
{
  qDebug()<<"new source"<<newSource;
  _serverToolBar->setVisible(newSource == "Server");
  refreshDisplayableItemsList();
  QString sourceDisplayName(newSource == "Server" ?
                              newSource :  QFileInfo(newSource).baseName());
  setWindowTitle(sourceDisplayName);
}

void JoCASSViewer::changeServerAddress()const
{
  QString servername(_servername->text());
  QString serverport(QString::number(_serverport->value()));
  QSettings settings;
  settings.setValue("Servername",servername);
  settings.setValue("Serverport",serverport);
  QString serveraddress(servername + ":" + serverport);
  DataSource *source(DataSourceManager::source("Server"));
  if (source)
    dynamic_cast<TCPClient*>(source)->setServer(serveraddress);
}

void JoCASSViewer::broadcastDarkcalCommand()const
{
  DataSource *source(DataSourceManager::source("Server"));
  if (source)
    dynamic_cast<TCPClient*>(source)->broadcastCommand("startDarkcal");
}

void JoCASSViewer::broadcastGaincalCommand()const
{
  DataSource *source(DataSourceManager::source("Server"));
  if (source)
    dynamic_cast<TCPClient*>(source)->broadcastCommand("startGaincal");
}

void JoCASSViewer::sendCustomCommand()const
{
  DataSource *source(DataSourceManager::source("Server"));
  if (!source)
    return;
  QStringList items(displayableItems());
  if (items.empty())
    return;
  bool ok(false);
  QString key(QInputDialog::getItem(0, QObject::tr("Select Key"),
                                    QObject::tr("Key:"), items, 0, false, &ok));
  if (!ok)
    return;
  QString command = QInputDialog::getText(0,tr("Command"),tr("Type command:"),
                                          QLineEdit::Normal,tr("Type command here"),&ok);
  if (!ok)
    return;
  dynamic_cast<TCPClient*>(source)->sendCommandTo(key,command);
}

void JoCASSViewer::clearHistogram()const
{
  DataSource *source(DataSourceManager::source("Server"));
  if (!source)
    return;
  QStringList items(displayableItems());
  if (items.empty())
    return;
  bool ok(false);
  QString key(QInputDialog::getItem(0, QObject::tr("Select Key"),
                                    QObject::tr("Key:"), items, 0, false, &ok));
  if (!ok)
    return;
  dynamic_cast<TCPClient*>(source)->clearHistogram(key);
}

void JoCASSViewer::createViewerForType(QMap<QString,DataViewer*>::iterator view,
                                       cass::HistogramBackend *hist)
{
  qDebug()<<"create viewer"<<view.key()<<hist->dimension();
  switch (hist->dimension())
  {
  case 0:
    view.value() = new ZeroDViewer(view.key(),this);
    break;
  case 1:
    view.value() = new OneDViewer(view.key(),this);
    break;
  case 2:
    view.value() = new TwoDViewer(view.key(),this);
    break;
  }
  view.value()->show();
  connect(view.value(),SIGNAL(viewerClosed(DataViewer*)),
          this,SLOT(removeViewer(DataViewer*)));
}

void JoCASSViewer::moveEvent(QMoveEvent *event)
{
  QSettings settings;
  settings.setValue("MainWindowPosition",event->pos());
}

void JoCASSViewer::resizeEvent(QResizeEvent *event)
{
  QSettings settings;
  settings.setValue("MainWindowSize",event->size());
}
