// Copyright (C) 2009 Jochen Küpper , Nils Kimmel,lmf, Nicola Coppola

#ifndef PNCCDANALYSIS_H
#define PNCCDANALYSIS_H

#include <QtCore/QPoint>
#include <QtCore/QMutex>
#include <QtCore/QMutexLocker>
#include <map>
#include <string>
#include <vector>
#include "cass_pnccd.h"
#include "analysis_backend.h"
#include "parameter_backend.h"
#include "pixel_detector.h"


namespace cass
{
  class CASSEvent;
  namespace pnCCD
  {

    class CASS_PNCCDSHARED_EXPORT DetectorParameter
    {
    public:
      typedef std::vector<double> correctionmap_t;
    public:
      size_t          _nbrDarkframes;     //the number of fills for each detector//
      correctionmap_t _offset;            //offsetmap
      correctionmap_t _noise;             //noise map
      uint32_t        _rebinfactor;       //the rebinfactor for rebinning
      double          _sigmaMultiplier;   //how big is above noise
      double          _adu2eV;            //conversion from adu to eV
      bool            _createPixellist;   //flag to switch pixellist on / off
      bool            _doOffsetCorrection;//flag to switch offsetcorrection on / off
      bool            _useCommonMode;     //need to know if I need to use a common mode subtraction scheme//
      uint32_t        _thres_for_integral;   //the thresold for special integral
      std::string     _darkcalfilename;   //filename of file containing dark & noisemap
      std::string     _savedarkcalfilename;// Dark frame calibration save file names for each detector//
      cass::detROI_   _detROI;

      cass::ROI::ROImask_t _ROImask;//The ROI mask
      //cass::ROI_t::ROImask_t _ROImask_converter;
      cass::ROI::ROIiterator_t _ROIiterator;//The ROI iterators
      //cass::ROI_t::ROIiterator_t _ROIiterator_converter;
    };

    class CASS_PNCCDSHARED_EXPORT Parameter : public cass::ParameterBackend
    {
    public:
      Parameter() {beginGroup("pnCCD");}
      ~Parameter()    {endGroup();}
      void load();
      void save();
      void loadDetectorParameter(size_t DetectorIndex);

    public:
      typedef std::vector<DetectorParameter> detparameters_t;
    public:
      detparameters_t _detectorparameters;  //the parameters of the detector
      bool            _isDarkframe;         //switch telling whether we are collecting darkframes right now
      //flag to set the dark/not-dark run condition
      bool            _This_is_a_dark_run;
    };



    //class pnCCDDevice;

    class CASS_PNCCDSHARED_EXPORT Analysis : public cass::AnalysisBackend
    {
    public:
      Analysis(); // {loadSettings();}
      //~Analysis();
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
      //void createOffsetAndNoiseMap(const pnCCDDevice&) {}
      //<>void createOffsetAndNoiseMap(cass::pnCCD::pnCCDDevice&);
      void createOffsetAndNoiseMap() {;}
      void rebin(){}

    private:
      QMutex                      _mutex; //a mutex to lock write operations
      Parameter                   _param; //the parameters used to analyze the pnccd detectors
      cass::PixelDetector::frame_t  _tmp;   //temporary storage for rebinning frames
    };
  } // end of scope of namespace pnCCD
} // end of scope of namespace cass

#endif
