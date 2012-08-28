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
{"ERROR ","WARNING ","INFO ","VERBOSEINFO ","DEBUG ","DEBUG1 ","DEBUG2 ","DEBUG3 ","DEBUG4 "};

void Log::add(Level level, const std::string& line)
{
  if (_loggingLevel < level)
    return;
  QMutexLocker locker(&_lock);
  if (!_instance)
    _instance = std::tr1::shared_ptr<Log>(new Log());
  _instance->addline(level,line);
}

void Log::loadSettings()
{
  QMutexLocker locker(&_lock);
  if (!_instance)
    _instance = std::tr1::shared_ptr<Log>(new Log());
  _instance->load();
}

Log::Log()
{
  load();
}

void Log::load()
{
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
  QString tmpfilename("casslog_" +
                   QDateTime::currentDateTime().toString("yyyyMMdd") +
                   ".log");
  QString filename(s.value("Filename",tmpfilename).toString());
  QFileInfo fileinfo(directory,filename);
  if(fileinfo.filePath().toStdString() != _filename)
  {
    if (_log.is_open())
      _log.close();
    _log.open(fileinfo.filePath().toUtf8().data(), ios_base::out | ios_base::app);
    _filename = fileinfo.filePath().toStdString();
  }
}

Log::~Log()
{
  _log.close();
}

void Log::addline(Level level, const string &line)
{
  _log << QDateTime::currentDateTime().toString("yyyy/MM/dd_hh:mm:ss:z ").toStdString()
       << _level2string[level]
       << line
       << endl << flush;
}
