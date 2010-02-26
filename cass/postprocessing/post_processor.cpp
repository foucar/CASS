//Copyright (C) 2010 lmf

#include <QtCore/QMutex>

#include "post_processor.h"
#include "cass_event.h"

//========================this should not be touched===========================
// define static members
cass::PostProcessor *cass::PostProcessor::_instance(0);
QMutex cass::PostProcessor::_mutex;

//create an instance of the singleton
cass::PostProcessor* cass::PostProcessor::instance(const char * OutputFileName)
{
  QMutexLocker locker(&_mutex);
  if(0 == _instance)
    _instance = new PostProcessor(OutputFileName);
  return _instance;
}

//destroy the instance of the singleton
void cass::PostProcessor::destroy()
{
  QMutexLocker locker(&_mutex);
  delete _instance;
  _instance = 0;
}
//=============================================================================


void cass::PostProcessor::postProcess(cass::CASSEvent &cassevent)
{
}
