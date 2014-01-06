// Copyright (C) 2013 Lutz Foucar

/**
 * @file jocassview.cpp contains the jocassviewer
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

using namespace jocassview;
using namespace std;


JoCASSViewer::JoCASSViewer(QObject *parent)
  : QObject(parent)
{
  _mw = new MainWindow();

  connect(_mw,SIGNAL(load_file_triggered(QString)),this,SLOT(loadData(QString)));
  connect(_mw,SIGNAL(save_triggered()),this,SLOT(on_autosave_triggered()));
  connect(_mw,SIGNAL(save_file_triggered(QString)),this,SLOT(saveFile(QString)));
  connect(_mw,SIGNAL(item_checked(QString,bool)),this,SLOT(on_displayitem_checked(QString,bool)));
  connect(&_updateTimer,SIGNAL(timeout()),this,SLOT(update_viewers()));
  connect(_mw,SIGNAL(get_data_triggered()),this,SLOT(update_viewers()));
  connect(_mw,SIGNAL(autoupdate_changed()),this,SLOT(on_autoupdate_changed()));
  connect(_mw,SIGNAL(server_changed(QString)),&_client,SLOT(setServer(QString)));
  connect(_mw,SIGNAL(quit_server_triggered()),&_client,SLOT(quitServer()));
  connect(_mw,SIGNAL(reload_ini_triggered()),&_client,SLOT(reloadIni()));
  connect(_mw,SIGNAL(broadcast_triggered(QString)),&_client,SLOT(broadcastCommand(QString)));
  connect(_mw,SIGNAL(send_command_triggered(QString,QString)),&_client,SLOT(sendCommandTo(QString,QString)));
  connect(_mw,SIGNAL(clear_histogram_triggered(QString)),&_client,SLOT(clearHistogram(QString)));
  connect(_mw,SIGNAL(refresh_list_triggered()),this,SLOT(on_refresh_list_triggered()));

  _mw->show();

  _client.setServer(_mw->on_server_property_changed());
  on_autoupdate_changed();
}

JoCASSViewer::~JoCASSViewer()
{
  delete _mw;
}

void JoCASSViewer::loadData(QString filename, QString key)
{
  _filename = filename;
  QStringList displayableitems(FileHandler::getKeyList(filename));
  _mw->setDisplayableItems(displayableitems);
  if (key == "" || key.isEmpty())
  {
    if (!FileHandler::isContainerFile(filename) && displayableitems.size() == 1)
      _mw->setDisplayedItem(displayableitems.front(),true);
  }
  else
  {
    if (displayableitems.contains(key))
      _mw->setDisplayedItem(key,true);
  }
  _mw->setWindowTitle(FileHandler::getBaseName(filename));
}

void JoCASSViewer::on_displayitem_checked(QString key, bool state)
{
  qDebug()<<"on_displayable_checked"<<key<<state;
  if (state)
  {
    _viewers[key] = 0;
    if (!_filename.isEmpty())
    {
      cass::HistogramBackend *hist(FileHandler::getData(_filename,key));
      if (!hist)
        return;
      createViewerForType(_viewers.find(key),hist);
      _viewers[key]->setData(hist);
    }
  }
  else
  {
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
  if (_viewers.isEmpty() || !_filename.isEmpty())
    return;

  _mw->setLEDStatus(StatusLED::busy);
  qDebug()<<"update viewers";
  bool sucess(true);

  /** get an iterator to go through the map and retrieve the first item where
   *  we get the id from. Then check whether all the other histograms should have
   *  the same id (if not then set the id to 0).
   *  Remember how big the container is for error checking later on
   */
  QMap<QString,DataViewer*>::iterator view(_viewers.begin());
//  cass::HistogramBackend *hist(_client.getData(view.key()));
//  const quint64 eventID = hist && false ? hist->id() : 0;
  const quint64 eventID(0);
  const int nbrWindows(_viewers.size());
  while( view != _viewers.end())
  {
    qDebug()<<"before:"<<_viewers.size()<<nbrWindows;
    cass::HistogramBackend * hist(_client.getData(view.key(),eventID));
    qDebug()<<"after:"<<_viewers.size()<<nbrWindows;
    /** if the size of the container changed (because the user closed or opened
     *  another window) break out here, because the iterator has been invalidated
     */
    if(_viewers.size() != nbrWindows)
    {
      sucess = false;
      break;
    }
    /** if there is nothing returned, there was an error. Just try the next one */
    if (!hist)
    {
      sucess = false;
      ++view;
      continue;
    }
    /** if the viewer hasn't been initalized, initialize it now */
    if (!view.value())
      createViewerForType(view,hist);
    /** if everything was fine then set the data in the viewer */
    view.value()->setData(hist);
    ++view;
  }
  /** set the to report sucess or faliure */
  sucess ? _mw->setLEDStatus(StatusLED::ok) : _mw->setLEDStatus(StatusLED::fail);
}

void JoCASSViewer::on_autoupdate_changed()
{
  _updateTimer.setInterval(_mw->interval());
  if(_mw->autoUpdate())
  {
    on_refresh_list_triggered();
    _updateTimer.start();
  }
  else
    _updateTimer.stop();
}

void JoCASSViewer::on_autosave_triggered() const
{
  if (_viewers.isEmpty() || !_filename.isEmpty())
    return;

  QString fileNameBase(QDir::currentPath() + "/" + QDateTime::currentDateTime().toString() + "_");

  saveFile(QString(fileNameBase + "autoSave.h5"));

  QMap<QString,DataViewer*>::const_iterator view(_viewers.constBegin());
  while( view != _viewers.constEnd())
  {
    if (view.value()->type() == "0DViewer")
    {

    }
    else if (view.value()->type() == "1DViewer")
    {
      saveFile(QString(fileNameBase + view.key() +".hst"),view.key());
      saveFile(QString(fileNameBase + view.key() +".csv"),view.key());
    }
    else if (view.value()->type() == "2DViewer")
    {
      saveFile(QString(fileNameBase + view.key() +".hst"),view.key());
      saveFile(QString(fileNameBase + view.key() +".csv"),view.key());
      saveFile(QString(fileNameBase + view.key() +".png"),view.key());
    }
    ++view;
  }
}

void JoCASSViewer::saveFile(const QString &filename, const QString &key) const
{
  if (_viewers.isEmpty() || !_filename.isEmpty())
    return;

  if (FileHandler::isContainerFile(filename))
  {
    QMap<QString,DataViewer*>::const_iterator view(_viewers.constBegin());
    while( view != _viewers.constEnd())
    {
      if (view.value())
        FileHandler::saveDataToContainer(filename,view.value()->data());
      ++view;
    }
  }
  else
  {
    QString savekey(key);
    if (savekey.isEmpty() || savekey == "")
    {
      QStringList items;
      QMap<QString,DataViewer*>::const_iterator view(_viewers.constBegin());
      while( view != _viewers.constEnd())
      {
        items.append(view.key());
        ++view;
      }

      bool ok(false);
      QString item(QInputDialog::getItem(_mw, QObject::tr("Select Key"),
                                         QObject::tr("Key:"), items, 0, false, &ok));
      if (!ok)
        return;
      savekey = item;
    }

    if(_viewers[savekey])
      FileHandler::saveData(filename,_viewers[savekey]->data());
  }
}

void JoCASSViewer::on_refresh_list_triggered()
{
  _filename.clear();
  _mw->setDisplayableItems(_client.getIdList());
}

void JoCASSViewer::createViewerForType(QMap<QString,DataViewer*>::iterator view,
                                       cass::HistogramBackend *hist)
{
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
