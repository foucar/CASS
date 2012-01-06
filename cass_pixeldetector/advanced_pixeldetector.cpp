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
#include "common_data.h"
#include "frame_processor_base.h"
#include "pixel_finder_base.h"

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
  CASSEvent::devices_t::const_iterator devIt(evt.devices().find(CASSEvent::PixelDetectors));
  if (devIt == evt.devices().end())
    throw invalid_argument("AdvancedDetector::associate(): Device 'PixelDetectors'" +
                           string("' does not exist in CASSEvent"));
  /** @todo use reference to device here to safe space in line below */
  const Device &dev (dynamic_cast<const Device&>(*(devIt->second)));
  Device::detectors_t::const_iterator detIt(dev.dets().find(_detector));
  if (detIt == dev.dets().end())
    throw invalid_argument("AdvancedDetector::associate(): Detector '" + toString(_detector) +
                           "' does not exist in Device 'PixelDetectors' within the CASSEvent");
  const Detector &det(detIt->second);
  _frame.columns = det.columns();
  _frame.rows = det.rows();
  _frame.data = det.frame();
  _common->createMaps(_frame);
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
  _common = CommonData::instance(_name); /** @todo do we need this here since we already got it in the constructor?*/
  _common->detectorId = _detector;
  _common->loadSettings(s);
  s.endGroup();
}
