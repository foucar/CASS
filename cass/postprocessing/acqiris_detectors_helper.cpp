//Copyright (C) 2010 Lutz Foucar

/**
 * @file acqiris_detectors_helper.cpp file contains definition of classes that
 *                                    extract information of acqiris detectors.
 *
 * @author Lutz Foucar
 */

#include "acqiris_detectors_helper.h"

#include "delayline_detector_analyzer_simple.h"
#include "delayline_detector.h"
#include "tof_analyzer_simple.h"
#include "tof_detector.h"
#include "com.h"
#include "cfd.h"
#include "cass_settings.h"


//initialize static members//
cass::ACQIRIS::HelperAcqirisDetectors::helperinstancesmap_t
    cass::ACQIRIS::HelperAcqirisDetectors::_instances;
QMutex cass::ACQIRIS::HelperAcqirisDetectors::_mutex;

cass::ACQIRIS::HelperAcqirisDetectors*
    cass::ACQIRIS::HelperAcqirisDetectors::instance
    (cass::ACQIRIS::HelperAcqirisDetectors::helperinstancesmap_t::key_type dettype)
{
  //lock this//
  QMutexLocker lock(&_mutex);
  //check if an instance of the helper class already exists//
  //return it, otherwise create one and return it//
  if (0 == _instances[dettype])
  {
    VERBOSEOUT(std::cout << "HelperAcqirisDetectors::instance(): creating an"
               <<" instance of the Acqiris Detector Helper for detector type "<<dettype
               <<std::endl);
    _instances[dettype] = new HelperAcqirisDetectors(dettype);
  }
  return _instances[dettype];
}

void cass::ACQIRIS::HelperAcqirisDetectors::destroy()
{
  QMutexLocker lock(&_mutex);
  std::map<ACQIRIS::Detectors,HelperAcqirisDetectors*>::iterator itdm
      (_instances.begin());
  for (;itdm!=_instances.end();++itdm)
    delete itdm->second;
}

cass::ACQIRIS::HelperAcqirisDetectors::HelperAcqirisDetectors(cass::ACQIRIS::Detectors dettype)
{
  VERBOSEOUT(std::cout << "AcqirisDetectorHelper::constructor: we are responsible for det type "<<dettype
             <<", which name is ");
  //create the detector
  //create the detector list with twice the amount of elements than workers
  switch(dettype)
  {
  case HexDetector:
    {
      VERBOSEOUT(std::cout <<"HexDetector"<<std::endl);
      for (size_t i=0; i<NbrOfWorkers*2;++i)
        _detectorList.push_front(std::make_pair(0,new DelaylineDetector(Hex,"HexDetector")));
    }
    break;
  case QuadDetector:
    {
      VERBOSEOUT(std::cout <<"QuadDetector"<<std::endl);
      for (size_t i=0; i<NbrOfWorkers*2;++i)
        _detectorList.push_front(std::make_pair(0,new DelaylineDetector(Quad,"QuadDetector")));
    }
    break;
  case VMIMcp:
    {
      VERBOSEOUT(std::cout <<"VMIMcp"<<std::endl);
      for (size_t i=0; i<NbrOfWorkers*2;++i)
        _detectorList.push_front(std::make_pair(0,new TofDetector("VMIMcp")));
    }
    break;
  case FELBeamMonitor:
    {
      VERBOSEOUT(std::cout <<"Beamdump"<<std::endl);
      for (size_t i=0; i<NbrOfWorkers*2;++i)
        _detectorList.push_front(std::make_pair(0,new TofDetector("FELBeamMonitor")));
    }
    break;
  case YAGPhotodiode:
    {
      VERBOSEOUT(std::cout <<"YAGPhotodiode"<<std::endl);
      for (size_t i=0; i<NbrOfWorkers*2;++i)
        _detectorList.push_front(std::make_pair(0,new TofDetector("YAGPhotodiode")));
    }
    break;
  case FsPhotodiode:
    {
      VERBOSEOUT(std::cout <<"FsPhotodiode"<<std::endl);
      for (size_t i=0; i<NbrOfWorkers*2;++i)
        _detectorList.push_front(std::make_pair(0,new TofDetector("FsPhotodiode")));
    }
    break;
  default: throw std::invalid_argument("HelperAcqirisDetectors::constructor: no such detector is present");
  }
}

cass::ACQIRIS::HelperAcqirisDetectors::~HelperAcqirisDetectors()
{
  for (detectorList_t::iterator it=_detectorList.begin();
       it != _detectorList.end();
       ++it)
    delete it->second;
}

void cass::ACQIRIS::HelperAcqirisDetectors::loadSettings(size_t)
{
  VERBOSEOUT(std::cout << "HelperAcqirisDetectors::loadSettings(): loading parameters of detector "<< _detector->name()<<std::endl);
  CASSSettings par;
  par.beginGroup("AcqirisDetectors");

  for (detectorList_t::iterator it=_detectorList.begin();
       it != _detectorList.end();
       ++it)
    it->second->loadSettings(&par);
  VERBOSEOUT(std::cout << "HelperAcqirisDetectors::loadSettings(): done loading for "<< _detector->name()<<std::endl);
}
