// Copyright (C) 2009 Jochen Küpper
// Copyright (C) 2009 Nils Kimmel
// Copyright (C) 2009, 2010 Lutz Foucar
// Copyright (C) 2009, 2010  Nicola Coppola

#ifndef PNCCDANALYSIS_H
#define PNCCDANALYSIS_H

#include <QtCore/QPoint>
#include <QtCore/QMutex>
#include <QtCore/QMutexLocker>
#include <QtCore/QWriteLocker>

#include <vector>
#include <stdlib.h>
#include "cass_pnccd.h"
#include "analysis_backend.h"
#include "parameter_backend.h"
#include "pixel_detector.h"


namespace cass
{
  //forward declaration
  class CASSEvent;
  namespace pnCCD
  {

    class CASS_PNCCDSHARED_EXPORT DetectorParameter
    {
    public:
      typedef std::vector<double> correctionmap_t;
    public:
      size_t          _nbrDarkframes;     //the number of fills for each detector//
      /** User settable parameters via CASS.ini
       *  each of the following is defined for each pnCCD detector and
       *  may be defined in a different way
       *  RebinFactor:
       *        The rebinfactor for rebinning of the frame accectable values are 1,2,4 and any power of 2
       *  MaxNoise:
       *        The max allowed noise level before a pixel is mask off, the limit is interpreted as
       *        3 x std dev, if the pixel displays a noise level largen than 3x_max_noise than it is removed
       *  SigmaMultiplier:
       *        The number of std deviation a pixel must be above noise to be selected as a photon
       *  Adu2ev
       *        The adu to eV conversion constant
       *  CreatePixelList
       *        true of false, if true and if _doOffsetCorrection==true
       *        (irregardless of the value of _useCommonMode) a list of Photons will be created
       *        for each pixel with value larger that _sigmaMultiplier*noise(of the pixel)
       *  DoOffsetCorrection
       *        true of false, if true the darkcalibration maps will be used to subtract the ADC
       *        offset, to correct the raw frame
       *  useCommonMode
       *       true of false, useful only if _doOffsetCorrection==true
       *            if true the CommonMode correction will be calculated and applied to the pixels
       *            each "row" of 128-pixel is separately considered: the pixels that have
       *            value below _sigmaMultiplier*noise(of the pixel), once their offset is removed,
       *            and that are not marked as BAD are used, the arithmetical mean is calculated and
       *            subtracted from all the pixels
       *  IntegralOverThres (here called _thres_for_integral )
       *       any Integer>=0 is accepted, in case the value is >0 than a second integral over the
       *            corrected frame is calculated, this time using only those pixel that have a
       *            value>_thres_for_integral
       *  DarkCalibrationFileName
       *       if set the darkframe calibrations will be taken out of the named file
       *       PLEASE DO not set unless you know what you are using,
       *       darkframe files created via Xonline/Raccoon will be ignored as they have
       *       a completely different structure
       *  DarkCalibrationSaveFileName
       *       it contains the name of the files where darkframe calibrations will be written into
       *       the values are ANYWAY overwritten by the saveParameter function to avoid
       *       overwriting of previously saved darkframe files
       **/
      correctionmap_t _offset;            //!< offsetmap
      correctionmap_t _noise;             //!< noise map
      uint32_t        _rebinfactor;       //!< the rebinfactor for rebinning
      double          _max_noise;         //!< pixels with noise larger than will be masked
      double          _sigmaMultiplier;   //!< how big is above noise
      double          _adu2eV;            //!< conversion from adu to eV
      bool            _createPixellist;   //!< flag to switch pixellist on / off
      bool            _doOffsetCorrection;//!< flag to switch offsetcorrection on / off
      bool            _useCommonMode;     //!< flag to switch a common mode subtraction scheme
      uint32_t        _thres_for_integral;//!< the thresold for special integral
      std::string     _darkcalfilename;   //!< filename of file containing dark & noisemap
      std::string     _savedarkcalfilename;// Dark frame calibration save file names for each detector//
      cass::detROI_   _detROI;

      cass::ROI::ROImask_t _ROImask;//The ROI mask
      cass::ROI::ROImask_t _ROImask_converter;
      cass::ROI::ROIiterator_t _ROIiterator;//The ROI iterators
      cass::ROI::ROIiterator_t _ROIiterator_converter;
    };

    class CASS_PNCCDSHARED_EXPORT Parameter : public cass::ParameterBackend
    {
    public:
      /** constructor creates group "pnCCD" */
      Parameter() {beginGroup("pnCCD");}
      /** constructor closes group "pnCCD" */
      ~Parameter() {endGroup();}
      /** load the parameters from cass.ini*/
      void load();
      /** save the parameters to cass.ini*/
      void save();
      void loadDetectorParameter(size_t DetectorIndex);

    public:
      typedef std::vector<DetectorParameter> detparameters_t;
    public:
      detparameters_t _detectorparameters;  //!< the parameters of the detector

      /**
       *  The following is a user settable parameter
       *  it decide for both pnCCD detectors if the following frames are
       *  to be used to calculate the offset corrections and noise map
       *  that are needed to calculate a corrected frame from the raw one
       *
       *    After 200 Frames have been seen a warning will be printed out,
       *    the user can then send a "Save ini" command to CASS to save the
       *    calibration constants to files. The calculation will proceed anyway
       *    even after the user has save the darkcal-frames to file
       *    allowing for a still improved statistical accuracy, in case needed.
       *
       *    if IsDarkFrames is set to true all the previously described parameters
       *    are NOT active as also any ROI that may be defined in the CASS.ini file
       *
       *    unfortunately there is a similar parameter for the commercial CCD
       *    and they can be set to opposite values
      **/
      bool            _isDarkframe;         //!< switch telling whether we are collecting darkframes right now
      //flag to set the dark/not-dark run condition
      bool            _This_is_a_dark_run;
    };



    class pnCCDDevice;

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
      void operator() (CASSEvent*);

    private:
      void createOffsetAndNoiseMap(cass::pnCCD::pnCCDDevice&);
      void rebin(cass::pnCCD::pnCCDDevice&,size_t DetectorIndex);

    private:
      QReadWriteLock              _RWlock; //a mutex to lock write operations but allow read
      QMutex                      _mutex; //a mutex to lock write operations
      Parameter                   _param; //!< the parameters used to analyze the pnccd detectors
      cass::PixelDetector::frame_t  _tmp; //temporary storage for rebinning frames
    };
  } // end of scope of namespace pnCCD
} // end of scope of namespace cass

#endif
