//Copyright (C) 2011 Lutz Foucar

/**
 * @file pnccd_analysis_new.cpp the pnccd correction in a new way
 *
 * @author Lutz Foucar
 */

#include "pnccd_analysis_new.h"

#include "pnccd_device.h"
#include "pixel_detector.h"
#include "cass_event.h"

using namespace cass;
using namespace pnCCD;

NewAnalysis::NewAnalysis()
{

}

void NewAnalysis::loadSettings()
{

}

void NewAnalysis::saveSettings()
{

}

void NewAnalysis::operator ()(CASSEvent *evt)
{
  //retrieve the right device
  pnCCDDevice &dev (*dynamic_cast<pnCCDDevice*>(evt->devices()[CASSEvent::pnCCD]));

  const size_t detId = 0;
  PixelDetector &det = (*dev.detectors())[detId];


}
