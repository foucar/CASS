// Copyright (C)2009 Jochen Küpper,lmf


#ifndef _ACQIRIS_ANALYSIS_H_
#define _ACQIRIS_ANALYSIS_H_

#include <string>
#include <vector>
#include "cass_acqiris.h"
#include "analysis_backend.h"
#include "parameter_backend.h"
#include "acqiris_device.h"

namespace cass 
{
  class CASSEvent;

  namespace ACQIRIS
  {
    class WaveformAnalyzerBackend;
    class DetectorAnalyzerBackend;

    class CASS_ACQIRISSHARED_EXPORT Parameter : public cass::ParameterBackend
    {
    public:
      Parameter()  {beginGroup("Acqiris");}
      ~Parameter() {endGroup();}

      void load();
      void save();

      //Device::detectors_t  _detectors; //the detector parameters (are the dets itselve)
    };




    class CASS_ACQIRISSHARED_EXPORT Analysis : public cass::AnalysisBackend
    {
    public:
      Analysis();
      ~Analysis()         {}
      void loadSettings() {_param.load();}
      void saveSettings() {_param.save();}
      //called for every event//
      void operator()(cass::CASSEvent*);

    private:
      typedef std::map<DetectorAnalyzers, DetectorAnalyzerBackend*> detectoranalyzer_t;
      typedef std::map<WaveformAnalyzers, WaveformAnalyzerBackend*> waveformanalyzer_t;

    private:
      waveformanalyzer_t  _waveformanalyzer;
      detectoranalyzer_t  _detectoranalyzer;
      Parameter           _param;
    };
  } //end namespace acqiris
} //end namespace cass

#endif



// Local Variables:
// coding: utf-8
// mode: C++
// c-file-offsets: ((c . 0) (innamespace . 0))
// c-file-style: "Stroustrup"
// fill-column: 100
// End:
