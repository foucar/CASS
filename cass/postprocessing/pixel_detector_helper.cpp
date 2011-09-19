//Copyright (C) 2011 Lutz Foucar

/**
 * @file pixel_detector_helper.cpp contains classes that extract and add
 *                                 information of pixel detectors.
 *
 * @author Lutz Foucar
 */

#include "pixel_detector_helper.h"

#include "cass_settings.h"
#include "cass_event.h"
#include "advanced_pixeldetector.h"

using namespace cass;
using namespace pixeldetector;
using namespace std;

namespace cass
{
  /** check wether the id is in the list
   *
   * checks whether the id ,which is the first part of the pair, is the id that
   * we are looking for
   *
   * @author Lutz Foucar
   */
  struct IsID
  {
    /** constructor
     *
     * @param id the id to check for
     */
    IsID(const uint64_t id):_id(id){}

    /** the operator
     *
     * @return true when id is the same as in the pair
     * @param element reference to one pair in the list
     */
    bool operator()(const DetectorHelper::detectorList_t::value_type &element)const
    { return (_id == element.first); }

    /** the id */
    const uint64_t _id;
  };
}


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
    (find_if(_detectorList.begin(), _detectorList.end(), IsID(evt.id())));
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
  detectorList_t::iterator it(_detectorList.begin());
  for (;it != _detectorList.end();++it)
    it->second->loadSettings(s);
  s.endGroup();
}
