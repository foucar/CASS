// Copyright (C) 2011 Lutz Foucar

/**
 * @file pixel_detector_container.h file container for pixeldetectors
 *
 * @author Lutz Foucar
 */

#include <sstream>
#include <stdexcept>

#include "pixel_detector_container.h"

#include "coalescing_base.h"
#include "cass_settings.h"
#include "cass_event.h"
#include "ccd_device.h"
#include "pnccd_device.h"

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

void PixelDetectorContainer::associate(const CASSEvent &evt)
{
  CASSEvent::Device device (static_cast<CASSEvent::Device>(_device));
  if (evt.devices().find(device) == evt.devices().end())
  {
    stringstream ss;
    ss << "PixelDetectorContainer::associate(): Device '"<<_device
        <<"' does not exist in Event";
    throw invalid_argument(ss.str());
  }
  if (evt.devices().find(device)->second->detectors()->size() <= _detector)
  {
    stringstream ss;
    ss << "PixelDetectorContainer::associate(): Detector '"<<_detector
        <<"' does not exist in Device '"<<_device
        <<"' of the Event";
    throw invalid_argument(ss.str());
  }
  _pixeldetector = &(*(evt.devices().find(device)->second->detectors()))[_detector];
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
