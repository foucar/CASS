//Copyright (C) 2010 Lutz Foucar

/**
 * @file acqiris_detectors_helper.cpp file contains definition of classes that
 *                                    extract information of acqiris detectors.
 *
 * @author Lutz Foucar
 */
#include <tr1/functional>

#include "acqiris_detectors_helper.h"

#include "cass_settings.h"
#include "cass_event.h"
#include "detector_backend.h"
#include "detector_analyzer_backend.h"
#include "convenience_functions.h"

using namespace cass::ACQIRIS;
using std::cout;
using std::endl;
using std::make_pair;
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
    VERBOSEOUT(cout << "HelperAcqirisDetectors::instance(): creating an"
               <<" instance of the Acqiris Detector Helper for detector '"<<detector
               <<"'"
               <<endl);
    _instances[detector] = shared_pointer(new HelperAcqirisDetectors(detector));
  }
  return _instances[detector];
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
  s.beginGroup(detname.c_str());
  _dettype = (static_cast<DetectorType>(s.value("DetectorType",ToF).toUInt()));
  s.endGroup();
  s.endGroup();
  for (size_t i=0; i<NbrOfWorkers*2;++i)
    _detectorList.push_front(make_pair(0,DetectorBackend::instance(_dettype,detname)));
  VERBOSEOUT(cout << "AcqirisDetectorHelper::constructor: "
             << "we are responsible for det '"<<detname
             << "', which is of type " <<_dettype
             <<endl);
}

HelperAcqirisDetectors::Det_sptr HelperAcqirisDetectors::validate(const CASSEvent &evt)
{
  using namespace std;
  QMutexLocker lock(&_helperMutex);
  detectorList_t::iterator it
    (find_if(_detectorList.begin(),_detectorList.end(),
             bind<bool>(equal_to<uint64_t>(),evt.id(),
                        bind<uint64_t>(&detectorList_t::value_type::first,_1))));
  if(_detectorList.end() == it)
  {
    Det_sptr det(_detectorList.back().second);
    det->associate(evt);
    detectorList_t::value_type newPair(make_pair(evt.id(),det));
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
  detectorList_t::const_iterator end(_detectorList.end());
  for (;it != end ;++it)
    it->second->loadSettings(s);
  s.endGroup();
}
