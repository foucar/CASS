//Copyright (C) 2010 Lutz Foucar

/**
 * @file tof_detector.cpp file contains the definition of the class that
 *                        describes a Time Of Flight Detector.
 *
 * @author Lutz Foucar
 */

#include "tof_detector.h"

#include "cass_settings.h"

void cass::ACQIRIS::TofDetector::associate(const CASSEvent &evt)
{
  _mcp.associate(evt);
}

void cass::ACQIRIS::TofDetector::loadSettings(CASSSettings &s)
{
  s.beginGroup(_name.c_str());
  s.beginGroup("MCP");
  _mcp.loadSettings(s);
  s.endGroup();
  s.endGroup();
}
