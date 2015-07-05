// Copyright (C) 2013, 2014 Lutz Foucar

/**
 * @file data_source_manager.cpp contains singleton class to manage the data sources
 *
 * @author Lutz Foucar
 */

#include <QtCore/QDebug>
#include <QtCore/QSignalMapper>

#if QT_VERSION >= 0x050000
#include <QtWidgets/QMenu>
#include <QtWidgets/QActionGroup>
#else
#include <QtGui/QMenu>
#include <QtGui/QActionGroup>
#endif

#include "data_source_manager.h"

#include "data_source.h"

using namespace jocassview;

DataSourceManager* DataSourceManager::_instance(0);

DataSourceManager::DataSourceManager()
  : _sourceMenu(0),
    _actionGroup(0),
    _mapper(0)
{

}

DataSourceManager* DataSourceManager::instance()
{
  if(!_instance)
  {
    //qDebug()<<"create instance";
    _instance = new DataSourceManager();
  }
  return _instance;
}

DataSource* DataSourceManager::source(const QString &sourcename)
{
  //qDebug()<<"get source"<<sourcename<<currentSourceName();
  if (sourcename.isEmpty())
    return instance()->_sources[currentSourceName()];
  else if(!instance()->_sources.contains(sourcename))
    return 0;
  return instance()->_sources.value(sourcename);
}

QString DataSourceManager::currentSourceName()
{
  QString name;
  if (instance()->_actionGroup && instance()->_actionGroup->checkedAction())
    name = instance()->_actionGroup->checkedAction()->text();
  //qDebug()<<"current source name "<<name;
  return name;
}

QStringList DataSourceManager::sourceNames()
{
  return instance()->_sources.keys();
}

void DataSourceManager::setMenu(QMenu *menu)
{
  //qDebug()<<"add menu";
  instance()->_sourceMenu = menu;
  instance()->_actionGroup = new QActionGroup(menu);
  instance()->_mapper = new QSignalMapper(menu);
  connect(instance()->_mapper,SIGNAL(mapped(QString)),instance(),SIGNAL(sourceChanged(QString)));
}

void DataSourceManager::addSource(const QString &sourcename, DataSource *source,
                                  bool setActive)
{
  //qDebug()<<"add Source"<< sourcename<<source;
  instance()->_sources[sourcename] = source;
  QAction *act(instance()->_sourceMenu->addAction(sourcename));
  act->setCheckable(true);
  connect(act,SIGNAL(triggered()),instance()->_mapper,SLOT(map()));
  instance()->_mapper->setMapping(act,sourcename);
  instance()->_actionGroup->addAction(act);
  if (setActive)
  {
    act->setChecked(true);
    emit instance()->sourceChanged(sourcename);
  }
}
