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
