// Copyright (C) 2011 Lutz Foucar

/**
 * @file frms6_reader.cpp contains class to read frms6 files created by Xonline
 *
 * @author Lutz Foucar
 */

#include "frms6_reader.h"

#include "cass_event.h"
#include "cass_settings.h"

using namespace cass;
using namespace std;

Frms6Reader::Frms6Reader()
{}

void Frms6Reader::loadSettings()
{
}

bool Frms6Reader::operator ()(ifstream &file, CASSEvent& event)
{
  return (true);
}
