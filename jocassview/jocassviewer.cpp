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

#include <QtGui/QMessageBox>

#include "jocassviewer.h"

#include "main_window.h"
#include "histogram.h"
#include "file_handler.h"
#include "zero_d_viewer.h"
#include "one_d_viewer.h"
#include "two_d_viewer.h"

using namespace jocassview;
using namespace std;


JoCASSViewer::JoCASSViewer(QObject *parent)
  : QObject(parent)
{
  _mw = new MainWindow();

  connect(_mw,SIGNAL(load_file_triggered(QString)),this,SLOT(loadData(QString)));
  connect(_mw,SIGNAL(item_checked(QString,bool)),this,SLOT(on_displayitem_checked(QString,bool)));

  _mw->show();
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
  if (!_filename.isEmpty())
  {
    if (state)
    {
      cass::HistogramBackend *hist(FileHandler::getData(_filename,key));
      if (!hist)
        return;
      switch(hist->dimension())
      {
      case(0):
        _viewers[key] = new ZeroDViewer(key,_mw);
        _viewers[key]->setData(hist);
        break;
      case(1):
        _viewers[key] = new OneDViewer(key,_mw);
        _viewers[key]->setData(hist);
        break;
      case(2):
        _viewers[key] = new TwoDViewer(key,_mw);
        _viewers[key]->setData(hist);
        break;
      }
      connect(_viewers[key],SIGNAL(viewerClosed(DataViewer*)),SLOT(on_viewer_destroyed(DataViewer*)));
    }
    else
    {
      if (_viewers.contains(key) && _viewers[key])
        _viewers[key]->close();
    }
  }
  else
  {
    if (state)
      _viewers[key] = 0;
    else
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
