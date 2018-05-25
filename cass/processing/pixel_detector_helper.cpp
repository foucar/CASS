//Copyright (C) 2011 -2014 Lutz Foucar

/**
 * @file pixel_detector_helper.cpp contains classes that extract and add
 *                                 information of pixel detectors.
 *
 * @author Lutz Foucar
 */

#include <tr1/functional>

#include "pixel_detector_helper.h"

#include "cass_settings.h"
#include "advanced_pixeldetector.h"
#include "log.h"

using namespace cass;
using namespace pixeldetector;
using namespace std;
using std::tr1::bind;
using std::tr1::placeholders::_1;


//initialize static members//
DetectorHelper::instancesmap_t DetectorHelper::_instances;
QMutex DetectorHelper::_mutex;

DetectorHelper::shared_pointer DetectorHelper::instance(const instancesmap_t::key_type& detector)
{
  QMutexLocker lock(&_mutex);
  if (_instances.find(detector) == _instances.end())
  {
    Log::add(Log::VERBOSEINFO, string("DetectorHelper::instance(): creating an") +
             " instance of the Pixel Detector Helper for detector '" + detector + "'");
    _instances[detector] = DetectorHelper::shared_pointer(new DetectorHelper(detector));
  }
  return _instances[detector];
}

void DetectorHelper::releaseDetector(const id_type &id)
{
  QMutexLocker lock(&_mutex);
  for (instancesmap_t::iterator it(_instances.begin()); it != _instances.end(); ++it)
    it->second->release(id);
}

DetectorHelper::DetectorHelper(const instancesmap_t::key_type& detname)
{
  for (size_t i=0; i<NbrOfWorkers+2;++i)
    _detectorList.push_back(make_pair(0,AdvDet_sptr(new AdvancedDetector(detname))));
  _lastEntry = _detectorList.begin();
  Log::add(Log::VERBOSEINFO,string("DetectorHelper::constructor: we are ")+
           "we are responsible for pixel det '" + detname + "'");
}

DetectorHelper::iter_type DetectorHelper::findId(const id_type &id)
{
  return (find_if(_detectorList.begin(), _detectorList.end(),
                  std::tr1::bind(equal_to<id_type>(),id,
                       std::tr1::bind<id_type>(&KeyDetPair_t::first,_1))));
}

void DetectorHelper::release(const id_type &id)
{
  QMutexLocker lock(&_helperMutex);
  iter_type it(findId(id));
  if (it != _detectorList.end())
    it->first = 0;
}

DetectorHelper::AdvDet_sptr DetectorHelper::detector(const CASSEvent &evt)
{
  QMutexLocker lock(&_helperMutex);
  iter_type it(findId(evt.id()));
  if(_detectorList.end() == it)
  {
    while(_lastEntry->first)
    {
      ++_lastEntry;
      if (_lastEntry == _detectorList.end())
        _lastEntry = _detectorList.begin();
    }
    it = _lastEntry;
    it->first = evt.id();
    it->second->associate(evt);
  }
  return it->second;
}

void DetectorHelper::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("PixelDetectors");
  detectorList_t::iterator it(_detectorList.begin());
  detectorList_t::const_iterator end(_detectorList.end());
  for (; it != end; ++it)
    it->second->loadSettings(s);
  s.endGroup();
}
