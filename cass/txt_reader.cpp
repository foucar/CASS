// Copyright (C) 2011 Lutz Foucar

/**
 * @file txt_reader.cpp contains class to read txt ascii files
 *
 * @author Lutz Foucar
 */

#include "txt_reader.h"

#include "cass_event.h"
#include "cass_settings.h"

using namespace cass;
using namespace std;

TxtReader::TxtReader()
{}

void TxtReader::loadSettings()
{
}

bool TxtReader::operator ()(ifstream &file, CASSEvent& event)
{
  return (true);
}
