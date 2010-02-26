// Copyright (C) 2010 lmf

#include "acqiris_analysis.h"
#include "acqiris_device.h"
#include "cass_event.h"
#include "com.h"
#include "cfd.h"
#include "delayline_detector_analyzer_simple.h"
#include "delayline_detector.h"


void cass::ACQIRIS::Parameter::load()
{
  //string for the container index//
  QString s;
  //sync before loading//
  sync();

  //delete the previous detector parameters//
  _detectors.clear();

  //the detector parameters//
  beginGroup("DetectorContainer");
  for (size_t i = 0; i < value("size",1).toUInt();++i)
  {
    beginGroup(s.setNum(static_cast<uint32_t>(i)));
    //find out which type the detector is//
    DetectorType dettype = static_cast<DetectorType>(value("DetectorType",Delayline).toInt());
    //create a new detector according to the detectortype//
    switch(dettype)
    {
    case Delayline : _detectors.push_back(DelaylineDetector()); break;
    default:
      std::cerr<<"Acqris Analyzer: Detectortype \""<<dettype<<"\" is unknown"<<std::endl;
      endGroup();
      continue;
      break;
    }
    //load the parameters of the detector//
    _detectors[i].loadParameters(this);
    endGroup(); //QString(i)
  }
  endGroup();//detectorcontainer
}

void cass::ACQIRIS::Parameter::save()
{
  //string for the container index//
  QString s;
  //the detector parameters//
  beginGroup("DetectorContainer");
  //how many detectors are there//
  setValue("size",static_cast<uint32_t>(_detectors.size()));
  for (size_t i = 0; i < _detectors.size();++i)
  {
    beginGroup(s.setNum(static_cast<uint32_t>(i)));
    _detectors[i].saveParameters(this);
    endGroup();
  }
  endGroup();//detectorcontainer
}











cass::ACQIRIS::Analysis::Analysis()
{
  //create the map with the waveform analyzers//
  _waveformanalyzer[cfd8]  = new CFD8Bit();
  _waveformanalyzer[cfd16] = new CFD16Bit();
  _waveformanalyzer[com8]  = new CoM8Bit();
  _waveformanalyzer[com16] = new CoM16Bit();

  //create the map with the detector analyzers//
  _detectoranalyzer[DelaylineSimple] = new DelaylineDetectorAnalyzerSimple(&_waveformanalyzer);

  //load the settings//
  loadSettings();
}


void cass::ACQIRIS::Analysis::operator()(cass::CASSEvent* cassevent)
{
  //get the remievent from the cassevent//
  AcqirisDevice* dev =
      dynamic_cast<AcqirisDevice*>(cassevent->devices()[cass::CASSEvent::Acqiris]);

  //ignore event if it is not initialized//
  if (dev->channels().size())
  {
    //copy the parameters to the event//
    dev->detectors() = _param._detectors;

    //analyze the detectors//
    //this has to be done for each detektor individually//
    for (size_t i=0; i<dev->detectors().size();++i)
    {
      //retrieve reference to the detector//
      DetectorBackend &det = dev->detectors()[i];
      //analyze the detector using the requested analyzer//
      _detectoranalyzer[det.analyzerType()]->analyze(det, dev->channels());
    }
  }
}



// Local Variables:
// coding: utf-8
// mode: C++
// c-file-offsets: ((c . 0) (innamespace . 0))
// c-file-style: "Stroustrup"
// fill-column: 100
// End:
