// Copyright (C) 2011 Lutz Foucar

/**
 * @file advanced_pixeldetector.cpp file more advanced pixeldetectors
 *
 * @author Lutz Foucar
 */

#include <sstream>
#include <stdexcept>

#include "advanced_pixeldetector.h"

#include "coalescing_base.h"
#include "cass_settings.h"
#include "cass_event.h"
#include "common_data.h"
#include "frame_processor_base.h"
#include "pixel_finder_base.h"
#include "cass_exceptions.h"

using namespace cass;
using namespace pixeldetector;
using namespace std;


AdvancedDetector::AdvancedDetector(const std::string &name)
  : _common(CommonData::instance(name)),
    _frameExtracted(false),
    _pixellistCreated(false),
    _hitListCreated(false),
    _name(name)
{}

const Frame& AdvancedDetector::frame()
{
  if(!_frameExtracted)
  {
    FrameProcessorBase &process(*_process);
    process(_frame);
    _frameExtracted = true;
  }
  return _frame;
}

const AdvancedDetector::pixels_t& AdvancedDetector::pixels()
{
  if(!_pixellistCreated)
  {
    frame();
    PixelFinderBase &find(*_find);
    find(_frame,_pixels);
    _pixellistCreated = true;
  }
  return _pixels;
}

const AdvancedDetector::hits_t& AdvancedDetector::hits()
{
  if (!_hitListCreated)
  {
    pixels();
    CoalescingBase & coalesce (*_coalesce);
    coalesce(_frame,_pixels,_hits);
    _hitListCreated = true;
  }
  return _hits;
}

void AdvancedDetector::associate(const CASSEvent &evt)
{
  /** validate the detector data */
  CASSEvent::devices_t::const_iterator devIt(
        evt.devices().find(CASSEvent::PixelDetectors));
  if (devIt == evt.devices().end())
    throw logic_error("AdvancedDetector::associate(): Device " +
                           string("'PixelDetectors' isn't existing in CASSEvent"));
  const Device &dev (dynamic_cast<const Device&>(*(devIt->second)));
  Device::detectors_t::const_iterator detIt(dev.dets().find(_detector));
  if (detIt == dev.dets().end())
    throw InvalidData("AdvancedDetector::associate(): Detector '" +
                      toString(_detector) + "' isn't present in Device " +
                      "'PixelDetectors' within the CASSEvent");
  const Detector &det(detIt->second);
  if (det.id() != evt.id())
    throw InvalidData("AdvancedDetector::associate(): The dataId '" +
                      toString(det.id()) + "' of detector '" + toString(_detector)+
                      "' is inconsistent with the eventId '" + toString(evt.id()) + "'");

  _frame.columns = det.columns();
  _frame.rows = det.rows();
  _frame.data = det.frame();
  _common->generateMaps(_frame);
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
  string frameprocesstype(s.value("FrameProcessorType","none").toString().toStdString());
  _process = FrameProcessorBase::instance(frameprocesstype);
  _process->loadSettings(s);
  string pixfindtype(s.value("PixelFinderType","simple").toString().toStdString());
  _find = PixelFinderBase::instance(pixfindtype);
  _find->loadSettings(s);
  string coalescetype(s.value("CoalescingFunctionType","simple").toString().toStdString());
  _coalesce = CoalescingBase::instance(coalescetype);
  _coalesce->loadSettings(s);
  _common = CommonData::instance(_name);
  _common->detectorId = _detector;
  _common->loadSettings(s);
  s.endGroup();
}
