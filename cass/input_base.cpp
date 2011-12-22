// Copyright (C) 2011 Lutz Foucar

/**
 * @file input_base.cpp contains the base class for all input modules
 *
 * @author Lutz Foucar
 */

#include "input_base.h"

#include "ratemeter.h"

using namespace cass;
using namespace std;

// ============define static members==============
InputBase::shared_pointer InputBase::_instance;
QMutex InputBase::_mutex;

InputBase::shared_pointer InputBase::instance()
{
  QMutexLocker lock(&_mutex);
  if(!_instance)
    throw logic_error("InputBase::instance(): is not created yet");
  return _instance;
}

InputBase::shared_pointer::element_type& InputBase::reference()
{
  QMutexLocker lock(&_mutex);
  if(!_instance)
    throw logic_error("InputBase::reference():is not created yet");
  return *_instance;
}
//===================================================

void InputBase::loadSettings(size_t)
{
  pause(true);
  load();
  resume();
}

void InputBase::newEventAdded()
{
  _ratemeter.count();
}
