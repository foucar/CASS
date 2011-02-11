// Copyright (C) 2011 Lutz Foucar

/**
 * @file lma_reader.cpp contains the class to read xtc files
 *
 * @author Lutz Foucar
 */

#include "lma_reader.h"

#include "cass_event.h"
#include "cass_settings.h"

using namespace cass;
using namespace std;

LmaReader::LmaReader()
{}

void LmaReader::loadSettings()
{
}

bool LmaReader::operator ()(ifstream &file, CASSEvent& event)
{
  bool retval(false);
  return retval;
}
