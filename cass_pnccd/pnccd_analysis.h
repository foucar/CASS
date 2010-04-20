// Copyright (C) 2009 Jochen Küpper , Nils Kimmel,lmf

#ifndef PNCCDANALYSIS_H
#define PNCCDANALYSIS_H

#include <QtCore/QMutex>
#include <map>
#include <string>
#include <vector>
#include "cass_pnccd.h"
#include "ccd_detector.h"
#include "analysis_backend.h"
#include "parameter_backend.h"


namespace cass
{
  class CASSEvent;
  namespace pnCCD
  {
    class pnCCDDevice;

    class CASS_PNCCDSHARED_EXPORT DetectorParameter
    {
    public:
      typedef std::vector<double> correctionmap_t;
    public:
      correctionmap_t _offset;            //offsetmap
      correctionmap_t _noise;             //noise map
      uint32_t        _rebinfactor;       //the rebinfactor for rebinning
      double          _sigmaMultiplier;   //how big is above noise
      double          _adu2eV;            //conversion from adu to eV
      bool            _createPixellist;   //flag to switch pixellist on / off
      bool            _doOffsetCorrection;//flag to switch offsetcorrection on / off
      std::string     _darkcalfilename;   //filename of file containing dark & noisemap
    };

    class CASS_PNCCDSHARED_EXPORT Parameter : public cass::ParameterBackend
    {
    public:
      Parameter(void) {beginGroup("pnCCD");}
      ~Parameter()    {endGroup();}
      void load();
      void save();
      void loadDetectorParameter(size_t DetectorIndex);

    public:
      typedef std::vector<DetectorParameter> detparameters_t;
    public:
      detparameters_t _detectorparameters;  //the parameters of the detector
      bool            _isDarkframe;         //switch telling whether we are collecting darkframes right now
    };





    class CASS_PNCCDSHARED_EXPORT Analysis : public cass::AnalysisBackend
    {
    public:
      Analysis(void);
      ~Analysis();
      /*
      initialize AnalysisBackend with new set of parameters
      */
      void loadSettings();
      void saveSettings();
      /*
      Put the pnCCDEvent object through the analysis chain. The original data
      remain unchanged, a new corrected pnCCD image is generated and X-ray
      photon hits are extracted if the user wishes to so. In addition, some
      basic parameters are reacorded, e.g. the number of detected events
      in the frame.
      */
      void operator() (cass::CASSEvent*);

    private:
      void createOffsetAndNoiseMap(const pnCCDDevice&) {}
      void rebin(){}

    private:
      QMutex                      _mutex; //a mutex to lock write operations
      Parameter                   _param; //the parameters used to analyze the pnccd detectors
      cass::CCDDetector::frame_t  _tmp;   //temporary storage for rebinning frames
    };
  } // end of scope of namespace pnCCD
} // end of scope of namespace cass

#endif



// Local Variables:
// coding: utf-8
// mode: C++
// c-file-offsets: ((c . 0) (innamespace . 0))
// c-file-style: "Stroustrup"
// fill-column: 100
// End:
