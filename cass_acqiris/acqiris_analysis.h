//Copyright (C) 2009,2010 Lutz Foucar
//Copyright (C) 2009 Jochen Küpper


#ifndef _ACQIRIS_ANALYSIS_H_
#define _ACQIRIS_ANALYSIS_H_

#include "cass_acqiris.h"
#include "analysis_backend.h"

namespace cass 
{
  class CASSEvent;

  namespace ACQIRIS
  {
//    class WaveformAnalyzerBackend;
//    class DetectorAnalyzerBackend;
//
//    class CASS_ACQIRISSHARED_EXPORT Parameter : public cass::ParameterBackend
//    {
//    public:
//      Parameter()  {beginGroup("Acqiris");}
//      ~Parameter() {endGroup();}
//
//      void load();
//      void save();
//
//      //Device::detectors_t  _detectors; //the detector parameters (are the dets itselve)
//    };



    /** Preanalyzer for Acqiris Data.
     * This class is not needed anymore, since all calculation are done in
     * the postprocessors for the Acqiris detectors only when needed.
     * @author Lutz Foucar
     */
    class CASS_ACQIRISSHARED_EXPORT Analysis : public cass::AnalysisBackend
    {
    public:
      /** default constructor (does nothing)*/
      Analysis();
      /** needs to be overwritten, since its pure virtual in base (does nothing)*/
      void loadSettings() {/*_param.load();*/}
      /** needs to be overwritten, since its pure virtual in base (does nothing)*/
      void saveSettings() {/*_param.save();*/}
      /** should evalutate acqiris part of the cassevent, now its an empty stub
        which does nothing*/
      void operator()(cass::CASSEvent*);

//    private:
//      typedef std::map<DetectorAnalyzers, DetectorAnalyzerBackend*> detectoranalyzer_t;
//      typedef std::map<WaveformAnalyzers, WaveformAnalyzerBackend*> waveformanalyzer_t;
//
//    private:
//      waveformanalyzer_t  _waveformanalyzer;
//      detectoranalyzer_t  _detectoranalyzer;
//      Parameter           _param;
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
