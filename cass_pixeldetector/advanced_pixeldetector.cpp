// Copyright (C) 2011 Lutz Foucar

/**
 * @file advanced_pixeldetector.h file more advanced pixeldetectors
 *
 * @author Lutz Foucar
 */

#include <sstream>
#include <stdexcept>

#include "advanced_pixeldetector.h"

#include "coalescing_base.h"
#include "cass_settings.h"
#include "cass_event.h"

using namespace cass;
using namespace pixeldetector;
using namespace std;

const AdvancedDetector::hits_t& AdvancedDetector::hits()
{
  if (!_hitListCreated)
  {
    CoalescingBase & coalesce (*_coalesce);
    //make sure that frame and pixels are created//
    frame();
    pixels();
    coalesce(_frame,_pixels,_hits);
    _hitListCreated = true;
  }
  return _hits;
}

void AdvancedDetector::associate(const CASSEvent &evt)
{
  if (evt.devices().find(CASSEvent::PixelDetectors) == evt.devices().end())
  {
    stringstream ss;
    ss << "AdvancedDetector::associate(): Device 'PixelDetectors'"
        <<"' does not exist in CASSEvent";
    throw invalid_argument(ss.str());
  }
  const Device &dev (dynamic_cast<const Device&>(*(evt.devices().find(CASSEvent::PixelDetectors)->second)));
  if (dev.dets().find(_detector) == dev.dets().end())
  {
    stringstream ss;
    ss << "AdvancedDetector::associate(): Detector '"<<_detector
        <<"' does not exist in the CASSEvent";
    throw invalid_argument(ss.str());
  }
  const Detector &det(dev.dets().find(_detector)->second);
  /** @todo copy info from cassevent to this container */
  _frameExtracted = false;
  _pixels.clear();
  _pixellistCreated = false;
  _hits.clear();
  _hitListCreated = false;
}

void AdvancedDetector::loadSettings(CASSSettings &s)
{
  s.beginGroup(QString::fromStdString(_name));
  _detector = s.value("Detector",0).toUInt();
  string type(s.value("CoalescingFunctionType","simple").toString().toStdString());
  _coalesce = CoalescingBase::instance(type);
  _coalesce->loadSettings(s);
  s.endGroup();
}
