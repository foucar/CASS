// Copyright (C) 2011 Lutz Foucar

/**
 * @file lma_reader.cpp contains the class to read lma files
 *
 * @author Lutz Foucar
 */

#include "lma_reader.h"

#include "cass_event.h"
#include "acqiris_device.h"
#include "cass_settings.h"

using namespace cass;
using namespace std;

LmaReader::LmaReader()
  :_newFile(true)
{}

void LmaReader::loadSettings()
{
}

bool LmaReader::operator ()(ifstream &file, CASSEvent& event)
{
  return true;
}
