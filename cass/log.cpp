// Copyright (C) 2012 Lutz Foucar

/**
 * @file log.cpp contains logger for cass
 *
 * @author Lutz Foucar
 */

#include <iostream>

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
QMutex Log::_lock;
Log::Level Log::_loggingLevel=Log::INFO;
const char* Log::_level2string[] =
{"ERROR ","WARNING ","INFO ","DEBUG ","DEBUG1 ","DEBUG2 ","DEBUG3 ","DEBUG4 "};

void Log::add(Level level, const std::string& line)
{
  if (_loggingLevel < level)
    return;
  QMutexLocker locker(&_lock);
  if (!_instance)
    _instance = std::tr1::shared_ptr<Log>(new Log());
  _instance->addline(level,line);
}

Log& Log::ref()
{
  QMutexLocker locker(&_lock);
  if (!_instance)
    _instance = std::tr1::shared_ptr<Log>(new Log());
  return *_instance;
}

Log::Log()
{
  loadSettings();
}

void Log::loadSettings()
{
//  QMutexLocker locker(&_lock);
  CASSSettings s;
  s.beginGroup("Log");
  for (int i(0); i < nbrOfLogLevel ; ++i)
  {
    if (s.value("MaxLoggingLevel","INFO").toString() + " " == _level2string[i])
    {
      _loggingLevel = static_cast<Level>(i);
      break;
    }
    _loggingLevel = INFO;
  }

  QDir directory(s.value("Directory",QDir::currentPath()).toString());
  QString filename("casslog_" +
                   QDateTime::currentDateTime().toString("yyyyMMdd") +
                   ".log");
  _log.open(QFileInfo(directory,filename).filePath().toUtf8().data(),
            ios_base::out | ios_base::app);
}

Log::~Log()
{
  _log.close();
}

void Log::addline(Level level, const string &line)
{
  _log << QDateTime::currentDateTime().toString("yyyy/MM/dd-hh:mm:ss:z: ").toStdString()
       << _level2string[level]
       << line
       << endl;
}
