// Copyright (C) 2011 Lutz Foucar

/**
 * @file pixel_detector_container.h file container for pixeldetectors
 *
 * @author Lutz Foucar
 */

#include "pixel_detector_container.h"

#include "coalescing_base.h"
#include "cass_settings.h"

using namespace cass;
using namespace std;

const PixelDetector::pixelList_t & PixelDetectorContainer::coalescedPixels()
{
  if (!_coalescedCreated)
  {
    CoalescingBase & coalesce (*_coalesce);
    coalesce(_pixeldetector->pixellist(), _coalescedpixels);
    _coalescedCreated = true;
  }
  return _coalescedpixels;
}

void PixelDetectorContainer::associate(const CASSEvent &in)
{
//  _pixeldetector = in;
  _coalescedCreated = false;
}

void PixelDetectorContainer::loadSettings(CASSSettings &s)
{
  s.beginGroup(QString::fromStdString(_name));
  _device = s.value("Device",0).toUInt();
  _detector = s.value("Detector",0).toUInt();
  string type(s.value("CoalescingFunctionType","simple").toString().toStdString());
  _coalesce = CoalescingBase::instance(type);
  s.endGroup();
}
