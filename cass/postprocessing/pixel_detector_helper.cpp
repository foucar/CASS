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
#include "detector_backend.h"
#include "detector_analyzer_backend.h"
#include "convenience_functions.h"

using namespace cass;

//initialize static members//
HelperPixelDetectors::instancesmap_t HelperPixelDetectors::_instances;
QMutex HelperPixelDetectors::_mutex;

HelperPixelDetectors::shared_pointer HelperPixelDetectors::instance(const instancesmap_t::key_type& detector)
{
//  QMutexLocker lock(&_mutex);
//  if (_instances.find(detector) == _instances.end())
//  {
//    VERBOSEOUT(std::cout << "HelperAcqirisDetectors::instance(): creating an"
//               <<" instance of the Acqiris Detector Helper for detector '"<<detector
//               <<"'"
//               <<std::endl);
//    _instances[detector] = new HelperPixelDetectors(detector);
//  }
//  return _instances[detector];
}

void HelperPixelDetectors::destroy()
{
//  QMutexLocker lock(&_mutex);
//  helperinstancesmap_t::iterator itdm(_instances.begin());
//  for (;itdm!=_instances.end();++itdm)
//    delete itdm->second;
}

HelperPixelDetectors::HelperPixelDetectors(const instancesmap_t::key_type& detname)
{
//  CASSSettings settings;
//  settings.beginGroup("AcqirisDetectors");
//  settings.beginGroup(detname.c_str());
//  _dettype = (static_cast<DetectorType>(settings.value("DetectorType",ToF).toUInt()));
//  settings.endGroup();
//  settings.endGroup();
//  for (size_t i=0; i<NbrOfWorkers*2;++i)
//    _detectorList.push_front(std::make_pair(0,DetectorBackend::instance(_dettype,detname)));
//  VERBOSEOUT(std::cout << "AcqirisDetectorHelper::constructor: "
//             << "we are responsible for det '"<<detname
//             << "', which is of type " <<_dettype
//             <<std::endl);
}


HelperPixelDetectors::PixDetContainer_sptr HelperPixelDetectors::detector(const CASSEvent &evt)
{
//  using namespace std;
//  QMutexLocker lock(&_helperMutex);
//  detectorList_t::iterator it
//    (find_if(_detectorList.begin(), _detectorList.end(), IsKey(evt.id())));
////  cout << " DetHelp 1"<<endl;
//  if(_detectorList.end() == it)
//  {
////    cout << " DetHelp 2"<<endl;
//    DetectorBackend* det (_detectorList.back().second);
////    cout << " DetHelp 3  "<<det<<endl;
//    det->associate(evt);
////    cout << " DetHelp 4"<<endl;
//    detectorList_t::value_type newPair(make_pair(evt.id(),det));
////    cout << " DetHelp 5"<<endl;
//    _detectorList.push_front(newPair);
////    cout << " DetHelp 6"<<endl;
//    _detectorList.pop_back();
//    it = _detectorList.begin();
////    cout << " DetHelp 7"<<endl;
//  }
////  cout << " DetHelp 8 "<<it->second<<endl;
//  return it->second;
}

void HelperPixelDetectors::loadSettings(size_t)
{
//  CASSSettings s;
//  s.beginGroup("AcqirisDetectors");
//  detectorList_t::iterator it(_detectorList.begin());
//  for (;it != _detectorList.end();++it)
//    it->second->loadSettings(s);
//  s.endGroup();
}
