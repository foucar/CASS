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
std::map<cass::ACQIRIS::Detectors,cass::ACQIRIS::HelperAcqirisDetectors*>
    cass::ACQIRIS::HelperAcqirisDetectors::_instances;
std::map<cass::ACQIRIS::DetectorAnalyzers, cass::ACQIRIS::DetectorAnalyzerBackend*>
    cass::ACQIRIS::HelperAcqirisDetectors::_detectoranalyzer;
std::map<cass::ACQIRIS::WaveformAnalyzers, cass::ACQIRIS::WaveformAnalyzerBackend*>
    cass::ACQIRIS::HelperAcqirisDetectors::_waveformanalyzer;
QMutex cass::ACQIRIS::HelperAcqirisDetectors::_mutex;

cass::ACQIRIS::HelperAcqirisDetectors* cass::ACQIRIS::HelperAcqirisDetectors::instance(cass::ACQIRIS::Detectors dettype)
{
  //lock this//
  QMutexLocker lock(&_mutex);
  //if the maps with the analyzers are empty, fill them//
  if (_waveformanalyzer.empty())
  {
    VERBOSEOUT(std::cout << "HelperAcqirisDetectors::instance(): the list of waveform analyzers is empty, we need to inflate it"<<std::endl);
    _waveformanalyzer[cfd8]  = new CFD8Bit();
    _waveformanalyzer[cfd16] = new CFD16Bit();
    _waveformanalyzer[com8]  = new CoM8Bit();
    _waveformanalyzer[com16] = new CoM16Bit();
  }
  if (_detectoranalyzer.empty())
  {
    VERBOSEOUT(std::cout << "HelperAcqirisDetectors::instance(): the list of "
               <<" detector analyzers is empty, we need to inflate it"
               <<std::endl);
    _detectoranalyzer[DelaylineSimple] =
        new DelaylineDetectorAnalyzerSimple(&_waveformanalyzer);
    _detectoranalyzer[ToFSimple] = new ToFAnalyzerSimple(&_waveformanalyzer);
  }
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
  waveformanalyzer_t::iterator itwa (_waveformanalyzer.begin());
  for (;itwa!=_waveformanalyzer.end();++itwa)
    delete itwa->second;
  detectoranalyzer_t::iterator itda (_detectoranalyzer.begin());
  for (;itda!=_detectoranalyzer.end();++itda)
    delete itda->second;
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
      _detector = new DelaylineDetector(Hex,"HexDetector");
      for (size_t i=0; i<NbrOfWorkers*2;++i)
        _detectorList.push_front(std::make_pair(0,new DelaylineDetector(Hex,"HexDetector")));
    }
    break;
  case QuadDetector:
    {
      VERBOSEOUT(std::cout <<"QuadDetector"<<std::endl);
      _detector = new DelaylineDetector(Quad,"QuadDetector");
      for (size_t i=0; i<NbrOfWorkers*2;++i)
        _detectorList.push_front(std::make_pair(0,new DelaylineDetector(Quad,"QuadDetector")));
    }
    break;
  case VMIMcp:
    {
      VERBOSEOUT(std::cout <<"VMIMcp"<<std::endl);
      _detector = new TofDetector("VMIMcp");
      for (size_t i=0; i<NbrOfWorkers*2;++i)
        _detectorList.push_front(std::make_pair(0,new TofDetector("VMIMcp")));
    }
    break;
  case FELBeamMonitor:
    {
      VERBOSEOUT(std::cout <<"Beamdump"<<std::endl);
      _detector = new TofDetector("FELBeamMonitor");
      for (size_t i=0; i<NbrOfWorkers*2;++i)
        _detectorList.push_front(std::make_pair(0,new TofDetector("FELBeamMonitor")));
    }
    break;
  case YAGPhotodiode:
    {
      VERBOSEOUT(std::cout <<"YAGPhotodiode"<<std::endl);
      _detector = new TofDetector("YAGPhotodiode");
      for (size_t i=0; i<NbrOfWorkers*2;++i)
        _detectorList.push_front(std::make_pair(0,new TofDetector("YAGPhotodiode")));
    }
    break;
  case FsPhotodiode:
    {
      VERBOSEOUT(std::cout <<"FsPhotodiode"<<std::endl);
      _detector = new TofDetector("FsPhotodiode");
      for (size_t i=0; i<NbrOfWorkers*2;++i)
        _detectorList.push_front(std::make_pair(0,new TofDetector("FsPhotodiode")));
    }
    break;
  default: throw std::invalid_argument("HelperAcqirisDetectors::constructor: no such detector is present");
  }
}

cass::ACQIRIS::HelperAcqirisDetectors::~HelperAcqirisDetectors()
{
  //delete the detectorList
  for (detectorList_t::iterator it=_detectorList.begin();
       it != _detectorList.end();
       ++it)
    delete it->second;
  //delete the detector
  delete _detector;
}

void cass::ACQIRIS::HelperAcqirisDetectors::loadSettings(size_t)
{
  VERBOSEOUT(std::cout << "HelperAcqirisDetectors::loadSettings(): loading parameters of detector "<< _detector->name()<<std::endl);
  CASSSettings par;
//  par.beginGroup("postprocessors");
  par.beginGroup("AcqirisDetectors");
  _detector->loadSettings(&par);
  VERBOSEOUT(std::cout << "HelperAcqirisDetectors::loadSettings(): done loading for "<< _detector->name()<<std::endl);
}
