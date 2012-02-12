// Copyright (C) 2012 Lutz Foucar

/**
 * @file log.cpp contains logger for cass
 *
 * @author Lutz Foucar
 */

#include <QtCore/QDateTime>
#include <QtCore/QString>
#include <QtCore/QFileInfo>
#include <QtCore/QDir>
#include <QtCore/QMutexLocker>

#include "log.h"

#include "cass_settings.h"

using namespace cass;
using namespace std;
using namespace tr1;

shared_ptr<Log> Log::_instance;
QMutex Log::_singletonLock;

void Log::add(Level level, const std::string& line)
{
  QMutexLocker locker(&_lock);
  if (!_instance)
    _instance = std::tr1::shared_ptr<Log>(new Log());
  _instance->addline(level,line);
}

Log::Log()
{
  CASSSettings s;
  s.beginGroup("Log");
  QDir directory(s.value("Directory",QDir::currentPath()).toString);
  QString filename(QDateTime::currentDateTime().toString("casslog_yyyyMMdd.log"));
  _log.open(QFileInfo(directory,filename).filePath(), ios::out | ios::app);
}

Log::~Log()
{
  _log.close();
}

void Log::addline(Level level, const string &line)
{
  static const char* _level2string[] =
  {"ERROR ","WARNING ","INFO ","DEBUG ","DEBUG1 ","DEBUG2 ","DEBUG3 ","DEBUG4 "};

  _log << QDateTime::currentDateTime().toString("yyyyMMdd-hhmmss-z: ").toStdString()
       << _level2string[level]
       << line
       << endl;
}
