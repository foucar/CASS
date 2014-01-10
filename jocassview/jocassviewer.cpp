// Copyright (C) 2013 Lutz Foucar

/**
 * @file jocassviewer.cpp contains the jocassviewer
 *
 * @author Lutz Foucar
 */

#include <utility>
#include <vector>
#include <iostream>

#include <QtCore/QFileInfo>
#include <QtCore/QDebug>
#include <QtCore/QSettings>
#include <QtCore/QDir>
#include <QtCore/QDateTime>
#include <QApplication>

#include <QtGui/QMessageBox>
#include <QtGui/QInputDialog>

#include "jocassviewer.h"

#include "main_window.h"
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


JoCASSViewer::JoCASSViewer(QObject *parent)
  : QObject(parent),
    _updateInProgress(false)
{
  /** use the timer as single shot */
  _updateTimer.setSingleShot(true);

  /** generate the main window */
  _mw = new MainWindow();

  /** create and initialize the server data sources and add them to the manager */
  TCPClient *client(new TCPClient);
  client->setServer(_mw->on_server_property_changed());
  DataSourceManager::addSource("Server",client);

  /** set up the connections and display it */
  connect(_mw,SIGNAL(load_file_triggered(QString)),this,SLOT(loadData(QString)));
  connect(_mw,SIGNAL(save_triggered()),this,SLOT(on_autosave_triggered()));
  connect(_mw,SIGNAL(save_file_triggered(QString)),this,SLOT(saveFile(QString)));
  connect(_mw,SIGNAL(print_triggered()),this,SLOT(on_print_triggered()));

  connect(_mw,SIGNAL(refresh_list_triggered()),this,SLOT(on_refresh_list_triggered()));
  connect(_mw,SIGNAL(get_data_triggered()),this,SLOT(update_viewers()));
  connect(_mw,SIGNAL(clear_histogram_triggered(QString)),client,SLOT(clearHistogram(QString)));
  connect(_mw,SIGNAL(send_command_triggered(QString,QString)),client,SLOT(sendCommandTo(QString,QString)));
  connect(_mw,SIGNAL(reload_ini_triggered()),client,SLOT(reloadIni()));
  connect(_mw,SIGNAL(broadcast_triggered(QString)),client,SLOT(broadcastCommand(QString)));
  connect(_mw,SIGNAL(quit_server_triggered()),client,SLOT(quitServer()));

  connect(_mw,SIGNAL(server_changed(QString)),client,SLOT(setServer(QString)));
  connect(_mw,SIGNAL(autoupdate_changed()),this,SLOT(on_autoupdate_changed()));
  connect(_mw,SIGNAL(item_checked(QString,bool)),this,SLOT(on_displayitem_checked(QString,bool)));

  connect(&_updateTimer,SIGNAL(timeout()),this,SLOT(update_viewers()));

  _mw->show();
}

JoCASSViewer::~JoCASSViewer()
{
  delete _mw;
}

void JoCASSViewer::loadData(QString filename, QString key)
{
  /** set the current source to file and set the filename to the source */
  DataSourceManager::addSource(filename,new FileHandler(filename));

  /** load the displayable items from the source and set the window title to
   *  to reflect the filename.
   */
  on_refresh_list_triggered();
  _mw->setWindowTitle(QFileInfo(filename).baseName());

  /** in case the key is on the list of displayable items, display it */
  if (_mw->displayableItems().contains(key))
    _mw->setDisplayedItem(key,true);
}

void JoCASSViewer::startViewer()
{
  on_autoupdate_changed();
}

void JoCASSViewer::on_displayitem_checked(QString key, bool state)
{
  qDebug()<<"on_displayable_checked"<<key<<state;
  if (state)
  {
    /** if the container already has a viewer with the requested name, exit here */
    if (_viewers.contains(key))
      return;
    /** create an entry in the viewers container with a 0 pointer and initialize
     *  the viewer based upon the type of data
     */
    _viewers[key] = 0;
    update_viewers();
  }
  else
  {
    /** if the key is on the list of viewers and the viewer has been created
     *  close it (which will delete the window, because all dataviewer windows
     *  have the delete on close flag set
     */
    if (_viewers.contains(key) && _viewers[key])
      _viewers[key]->close();
  }
}

void JoCASSViewer::on_viewer_destroyed(DataViewer *obj)
{
  QString key(obj->windowTitle());
  _viewers.remove(key);
  _mw->setDisplayedItem(key,false);
}

void JoCASSViewer::update_viewers()
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
  _mw->setLEDStatus(StatusLED::busy);
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
        _mw->setDisplayedItem(view.key(),false,false);
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
        _mw->setDisplayedItem(view.key(),false,false);
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
      QList<Data*> data(view.value()->data());
      const int nbrData(data.size());
      QList<Data*>::iterator dataIt(data.begin());
      while (dataIt != data.end())
      {
        /** validate source */
        QString sourceName((*dataIt)->sourceName());
        DataSource *source(DataSourceManager::source(sourceName));
        if(!source)
          continue;
        /** validate result to update */
        if (!(*dataIt)->result())
          continue;
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
  sucess ? _mw->setLEDStatus(StatusLED::ok) : _mw->setLEDStatus(StatusLED::fail);

  /** restart the updatetimer when requested and reset the in progress flag */
  on_autoupdate_changed();
  _updateInProgress = false;
}

void JoCASSViewer::on_autoupdate_changed()
{
  qDebug()<<"on_autoupdate_changed"<<_mw->autoUpdate()<<_mw->interval();
  _updateTimer.setInterval(_mw->interval());
  if(_mw->autoUpdate())
    _updateTimer.start();
  else
    _updateTimer.stop();
}

void JoCASSViewer::on_autosave_triggered() const
{
  if (_viewers.isEmpty())
    return;
  /** generate the general automatic filename */
  QString fileNameBase(QDir::currentPath() + "/" + QDateTime::currentDateTime().toString() + "_");

  /** save all open windows data to a single container file */
  saveFile(QString(fileNameBase + "autoSave.h5"),_mw->displayedItems());

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

void JoCASSViewer::saveFile(const QString &filename, const QStringList &keys) const
{
  if (_viewers.isEmpty())
    return;

  QStringList printKeys(keys);

  /** if no keys are given, request at least one using the iteminput dialog
   *  The preselected item in the dialog should be the currently highlighted
   *  or, in case of a container file, "all"
   */
  if (printKeys.isEmpty())
  {
    QStringList items(_mw->displayedItems());
    items.prepend("**ALL**");
    QWidget *focusWiget(QApplication::focusWidget());
    int preselectItemId(items.indexOf("**ALL**"));
    if (!FileHandler::isContainerFile(filename) && (focusWiget))
      preselectItemId = items.indexOf(focusWiget->windowTitle());
    bool ok(false);
    QString item(QInputDialog::getItem(_mw, QObject::tr("Select Key"),
                                       QObject::tr("Key:"), items,
                                       preselectItemId, false, &ok));
    if (!ok)
      return;
    if (item.contains("***ALL***"))
      printKeys = _mw->displayedItems();
    else
      printKeys.append(item);
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
  for (cIt = printKeys.constBegin(); cIt != printKeys.constEnd(); ++cIt)
  {
    if (_viewers.value(*cIt))
    {
      QString fname(filename);
      if(!FileHandler::isContainerFile(filename) && printKeys.size() > 1)
        fname.insert(fname.lastIndexOf("."),"_" + *cIt);
      _viewers.value(*cIt)->saveData(fname);
    }
  }
}

void JoCASSViewer::on_refresh_list_triggered()
{
  qDebug()<<"on_refresh_list_triggered";
  if (DataSourceManager::source())
    _mw->setDisplayableItems(DataSourceManager::source()->resultNames());
}

void JoCASSViewer::createViewerForType(QMap<QString,DataViewer*>::iterator view,
                                       cass::HistogramBackend *hist)
{
  qDebug()<<"create viewer"<<view.key()<<hist->dimension();
  switch (hist->dimension())
  {
  case 0:
    view.value() = new ZeroDViewer(view.key(),_mw);
    break;
  case 1:
    view.value() = new OneDViewer(view.key(),_mw);
    break;
  case 2:
    view.value() = new TwoDViewer(view.key(),_mw);
    break;
  }
  view.value()->show();
  connect(view.value(),SIGNAL(viewerClosed(DataViewer*)),SLOT(on_viewer_destroyed(DataViewer*)));
}

void JoCASSViewer::on_print_triggered()
{
  QStringList items(_mw->displayedItems());
  QWidget *focusWiget(QApplication::focusWidget());
  QString preselectItem;
  if (focusWiget)
    preselectItem=focusWiget->windowTitle();
  bool ok(false);
  QString item(QInputDialog::getItem(_mw, QObject::tr("Select Key"),
                                     QObject::tr("Print Key:"), items,
                                     items.indexOf(preselectItem), false, &ok));
  if (!ok)
    return;

  if(!_viewers.contains(item))
    return;

  _viewers.value(item)->print();

}
