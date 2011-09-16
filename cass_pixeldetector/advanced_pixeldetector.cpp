// Copyright (C) 2011 Lutz Foucar

/**
 * @file pixel_detector_new.h file more advanced pixeldetectors
 *
 * @author Lutz Foucar
 */

#include <sstream>
#include <stdexcept>

#include "pixel_detector_new.h"

#include "coalescing_base.h"
#include "cass_settings.h"
#include "cass_event.h"
#include "pixeldetector_device.h"

using namespace cass;
using namespace std;

const PixelDetectorContainer::hitlist_t& PixelDetectorContainer::hits()
{
  if (!_hitListCreated)
  {
    CoalescingBase & coalesce (*_coalesce);
    coalesce(*this, _hits);
    _hitListCreated = true;
  }
  return _hits;
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
  _pixelslist = _pixeldetector->pixellist();
  _hits.clear();
  _hitListCreated = false;
}

void PixelDetectorContainer::loadSettings(CASSSettings &s)
{
  s.beginGroup(QString::fromStdString(_name));
  _device = s.value("Device",0).toUInt();
  _detector = s.value("Detector",0).toUInt();
  string type(s.value("CoalescingFunctionType","simple").toString().toStdString());
  _coalesce = CoalescingBase::instance(type);
  _coalesce->loadSettings(s);
  s.endGroup();
}
