//Copyright (C) 2010 Lutz Foucar

/**
 * @file acqiris_detectors_helper.cpp file contains definition of classes that
 *                                    extract information of acqiris detectors.
 *
 * @author Lutz Foucar
 */
#include <tr1/functional>
#include <string>

#include "acqiris_detectors_helper.h"

#include "cass_settings.h"
#include "detector_backend.h"
#include "detector_analyzer_backend.h"
#include "convenience_functions.h"
#include "log.h"

using namespace cass::ACQIRIS;
using namespace std;
using std::tr1::bind;
using std::tr1::placeholders::_1;

//initialize static members//
HelperAcqirisDetectors::helperinstancesmap_t HelperAcqirisDetectors::_instances;
QMutex HelperAcqirisDetectors::_mutex;

HelperAcqirisDetectors::shared_pointer HelperAcqirisDetectors::instance(const helperinstancesmap_t::key_type& detector)
{
  QMutexLocker lock(&_mutex);
  if (_instances.find(detector) == _instances.end())
  {
    Log::add(Log::DEBUG0,string("HelperAcqirisDetectors::instance(): creating an") +
               " instance of the Acqiris Detector Helper for detector '" + detector + "'");
    _instances[detector] = shared_pointer(new HelperAcqirisDetectors(detector));
  }
  return _instances[detector];
}

void HelperAcqirisDetectors::releaseDetector(const id_type &id)
{
  QMutexLocker lock(&_mutex);
  for (helperinstancesmap_t::iterator it(_instances.begin()); it != _instances.end(); ++it)
    it->second->release(id);
}

const HelperAcqirisDetectors::helperinstancesmap_t& HelperAcqirisDetectors::instances()
{
  QMutexLocker lock(&_mutex);
  return _instances;
}

HelperAcqirisDetectors::HelperAcqirisDetectors(const helperinstancesmap_t::key_type& detname)
{
  CASSSettings s;
  s.beginGroup("AcqirisDetectors");
  s.beginGroup(QString::fromStdString(detname));
  _dettype = (static_cast<DetectorType>(s.value("DetectorType",ToF).toUInt()));
  s.endGroup();
  s.endGroup();
  for (size_t i=0; i<NbrOfWorkers+2;++i)
    _detectorList.push_back(make_pair(0,DetectorBackend::instance(_dettype,detname)));
  _lastEntry = _detectorList.begin();
  Log::add(Log::DEBUG0,string("AcqirisDetectorHelper::constructor: ") +
           "we are responsible for det '" + detname +  "', which is of type " + toString(_dettype));
}

HelperAcqirisDetectors::iter_type HelperAcqirisDetectors::findId(const id_type &id)
{
  return (find_if(_detectorList.begin(), _detectorList.end(),
                  bind(equal_to<id_type>(),id,
                       bind<id_type>(&KeyDetPair_t::first,_1))));
}

void HelperAcqirisDetectors::release(const id_type &id)
{
  iter_type it(findId(id));
  if (it != _detectorList.end())
    it->first = 0;
}

DetectorBackend& HelperAcqirisDetectors::validate(const CASSEvent &evt)
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
  return *it->second;
}

void HelperAcqirisDetectors::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("AcqirisDetectors");
  detectorList_t::iterator it(_detectorList.begin());
  detectorList_t::const_iterator end(_detectorList.end());
  for (;it != end ;++it)
    it->second->loadSettings(s);
  s.endGroup();
}
