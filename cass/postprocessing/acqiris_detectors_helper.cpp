//Copyright (C) 2010 Lutz Foucar

/**
 * @file acqiris_detectors_helper.cpp file contains definition of classes that
 *                                    extract information of acqiris detectors.
 *
 * @author Lutz Foucar
 */

#include "acqiris_detectors_helper.h"

#include "cass_settings.h"
#include "cass_event.h"
#include "detector_backend.h"
#include "detector_analyzer_backend.h"
#include "convenience_functions.h"

using namespace cass::ACQIRIS;

//initialize static members//
HelperAcqirisDetectors::helperinstancesmap_t HelperAcqirisDetectors::_instances;
QMutex HelperAcqirisDetectors::_mutex;

HelperAcqirisDetectors* HelperAcqirisDetectors::instance(const helperinstancesmap_t::key_type& detector)
{
  QMutexLocker lock(&_mutex);
  if (_instances.find(detector) == _instances.end())
  {
    VERBOSEOUT(std::cout << "HelperAcqirisDetectors::instance(): creating an"
               <<" instance of the Acqiris Detector Helper for detector \" "<<detector
               <<"\""
               <<std::endl);
    _instances[detector] = new HelperAcqirisDetectors(detector);
  }
  return _instances[detector];
}

void cass::ACQIRIS::HelperAcqirisDetectors::destroy()
{
  QMutexLocker lock(&_mutex);
  helperinstancesmap_t::iterator itdm(_instances.begin());
  for (;itdm!=_instances.end();++itdm)
    delete itdm->second;
}

HelperAcqirisDetectors::HelperAcqirisDetectors(const helperinstancesmap_t::key_type& detname)
{
  CASSSettings settings;
  settings.beginGroup("AcqirisDetectors");
  settings.beginGroup(detname.c_str());
  DetectorType dettype (static_cast<DetectorType>(settings.value("DetectorType",Delayline).toUInt()));
  settings.endGroup();
  settings.endGroup();
  for (size_t i=0; i<NbrOfWorkers*2;++i)
    _detectorList.push_front(std::make_pair(0,DetectorBackend::instance(dettype,detname)));
  VERBOSEOUT(std::cout << "AcqirisDetectorHelper::constructor: "
             << "we are responsible for det "<<detname
             << ", which name is of type " <<dettype
             <<std::endl);
}

HelperAcqirisDetectors::~HelperAcqirisDetectors()
{
  for (detectorList_t::iterator it=_detectorList.begin();
       it != _detectorList.end();
       ++it)
    delete it->second;
}

DetectorBackend * HelperAcqirisDetectors::validate(const CASSEvent &evt)
{
  QMutexLocker lock(&_helperMutex);
  detectorList_t::iterator it =
    std::find_if(_detectorList.begin(), _detectorList.end(), IsKey(evt.id()));
  if(_detectorList.end() == it)
  {
    DetectorBackend* det (_detectorList.back().second);
    det->associate(evt);
    detectorList_t::value_type newPair = std::make_pair(evt.id(),det);
    _detectorList.push_front(newPair);
    _detectorList.pop_back();
    it = _detectorList.begin();
  }
  return it->second;
}

void HelperAcqirisDetectors::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("AcqirisDetectors");
  detectorList_t::iterator it(_detectorList.begin());
  for (;it != _detectorList.end();++it)
    it->second->loadSettings(s);
  s.endGroup();
}
