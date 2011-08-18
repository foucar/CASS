// Copyright (C) 2011 Lutz Foucar

/**
 * @file raw_sss_reader.cpp contains the class to read commercial ccd files
 *                          created with Per Johnsonn's program
 *
 * @author Lutz Foucar
 */

#include "raw_sss_reader.h"

#include "cass_event.h"
#include "cass_settings.h"

using namespace cass;
using namespace std;

RAWSSSReader::RAWSSSReader()
{}

void RAWSSSReader::loadSettings()
{
}

bool RAWSSSReader::operator ()(ifstream &file, CASSEvent& event)
{
  return (true);
}
