//Copyright (C) 2011 Lutz Foucar

/**
 * @file pixel_detector_helper.cpp contains classes that extract and add
 *                                 information of pixel detectors.
 *
 * @author Lutz Foucar
 */

#include<tr1/functional>

#include "pixel_detector_helper.h"

#include "cass_settings.h"
#include "cass_event.h"
#include "advanced_pixeldetector.h"

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
    VERBOSEOUT(std::cout << "DetectorHelper::instance(): creating an"
               <<" instance of the Pixel Detector Helper for detector '"<<detector
               <<"'"
               <<std::endl);
    _instances[detector] = DetectorHelper::shared_pointer(new DetectorHelper(detector));
  }
  return _instances[detector];
}

DetectorHelper::DetectorHelper(const instancesmap_t::key_type& detname)
{
  for (size_t i=0; i<NbrOfWorkers*2;++i)
    _detectorList.push_front(make_pair(0,new AdvancedDetector(detname)));
  VERBOSEOUT(std::cout << "DetectorHelper::constructor: "
             << "we are responsible for pixel det '"<<detname <<"'"
             <<std::endl);
}


DetectorHelper::AdvDet_sptr DetectorHelper::detector(const CASSEvent &evt)
{
  QMutexLocker lock(&_helperMutex);
  detectorList_t::iterator it
    (find_if(_detectorList.begin(), _detectorList.end(),
             bind<bool>(equal_to<uint64_t>(),evt.id(),
                        bind<uint64_t>(&detectorList_t::value_type::first,_1))));
  if(_detectorList.end() == it)
  {
    DetectorHelper::AdvDet_sptr det(_detectorList.back().second);
    det->associate(evt);
    detectorList_t::value_type newPair(make_pair(evt.id(),det));
    _detectorList.push_front(newPair);
    _detectorList.pop_back();
    it = _detectorList.begin();
  }
  return it->second;
}

void DetectorHelper::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("PixelDetectors");
  /** @todo use for each with bind */
  detectorList_t::iterator it(_detectorList.begin());
  detectorList_t::const_iterator end(_detectorList.end());
  for (; it != end; ++it)
    it->second->loadSettings(s);
//  for_each(_detectorList.begin(),_detectorList.end(),
//           bind<void>(&AdvancedDetector::loadSettings,
//                      bind<AdvancedDetector*>(&AdvDet_sptr::get,
//                                              bind<AdvDet_sptr>(&detectorList_t::value_type::second,_1)),
//                      s));
  s.endGroup();
}
