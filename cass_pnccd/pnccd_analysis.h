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
#include "cass_settings.h"
#include "pixel_detector.h"


namespace cass
{
  //forward declaration
  class CASSEvent;
  namespace pnCCD
  {
    /**
     *  User settable parameters via CASS.ini
     *  each of the following is defined for each pnCCD detector and
     *  may be defined in a different way on different pnCCD detectors
     *  @cassttng
     *  RebinFactor\n
     *        The rebinfactor for rebinning of the frame accectable values are 1,2,4 and any power of 2.\n
     *        By default the value is set to 1.
     *  @cassttng
     *  MaxNoise\n
     *        The max allowed noise level before a pixel is mask off, the limit is interpreted as
     *        std dev, if the pixel displays a noise level largen than _max_noise than it is removed.\n
     *        By default the value is set to 4000.
     *  @cassttng
     *  SigmaMultiplier\n
     *        The number of std deviation a pixel must be above noise to be selected as a photon.\n
     *        By default the value is set to 4.\n
     *  @cassttng
     *  Adu2ev\n
     *        The adu to eV conversion constant.\n
     *        By default the value is set to 1.\n
     *  @cassttng
     *  CreatePixelList\n
     *        True or false, if true and if _doOffsetCorrection==true
     *            (irregardless of the value of _useCommonMode) a list of Photons will be created
     *            for each pixel with value larger that _sigmaMultiplier*noise(of the pixel).\n
     *        By default the value is set to false.\n
     *  @cassttng
     *  DoOffsetCorrection\n
     *        True or false, if true the darkcalibration maps will be used to subtract the ADC
     *            offset, to correct the raw frame.\n
     *        By default the value is set to false.\n
     *  @cassttng
     *  useCommonMode\n
     *       True or false, useful only if _doOffsetCorrection==true,\n
     *            if true the CommonMode correction will be calculated and applied to the pixels
     *            each "row" of 128-pixel is separately considered: the pixels that have
     *            value below _sigmaMultiplier*noise(of the pixel), once their offset is removed,
     *            and that are not marked as BAD are used, the arithmetical mean is calculated and
     *            subtracted from all the pixels.\n
     *        By default the value is set to false.
     *  @cassttng
     *  useCTECorr\n
     *       True or false,\n
     *            use 'useCTECorrection' as key in ini-file\n
     *            if true the Charge Correction Efficiency correction will be applied to the pixels.\n
     *        By default the value is set to false.
     *  @cassttng
     *  useGAINCorr\n
     *       True or false,\n
     *            use 'useGAINCorrection' as key in ini-file\n
     *            if true the Gain correction will be applied to the pixels.\n
     *        By default the value is set to false.
     *  @cassttng
     *  UseDarkcalBadPixelInfo\n
     *       True or false, by default UseDarkcalBadPixelInfo==false,\n
     *            useful only if IsDarkFrames==false,\n
     *            if true the pixel(s) that have a noise larger than MaxNoise are masked, regardless
     *            of the fact that the offset correction and/or the common mode correction are
     *            performed;\n
     *            if false, no extra pixel will be masked on base of its noise level!
     *  @cassttng
     *  autoSaveDarkCals\n
     *       True or false, by default autoSaveDarkCals==false,
     *            useful only if IsDarkFrames==true.\n
     *            If true when exactly 200 frames (per detector) have been collected, the Darkcal 
     *            filemap(s) is(are) saved and the value of _isDarkframe is set to false.\n
     *            The program is NOT able to reload the setting-file as the file itself has not been changed.\n
     *            The user should reload the cass-settings, or restart cass, only after he/she is sure that
     *            the cassini file has been modified in the proper way!\n
     *            If false the program will continue indefinetly to add frames to the darkcal arrays,
     *            and the user needs to give from jocassview (or any client application that would perform
     *            similar tasks) the "Write Darkcal File" command to save the calibrations into file(s).
     *  @cassttng
     *  IntegralOverThres (internally called _thres_for_integral )\n
     *       Any Integer>=0 is accepted, in case the value is >0 than a second integral over the
     *            corrected frame is calculated, this time using only those pixel that have a
     *            value>_thres_for_integral.\n
     *        By default the value is set to 0.\n
     *  @cassttng
     *  DarkCalibrationFileName\n
     *       If set the darkframe calibrations will be taken out of the named file.\n
     *       PLEASE DO NOT SET unless you know what you are using,
     *       (for example if using offline CASS)
     *       darkframe files created via Xonline/Raccoon will be ignored as they have
     *       a completely different structure.
     *  @cassttng
     *  DarkCalibrationSaveFileName\n
     *       It contains the name of the files where darkframe calibrations will be written into
     *       the values are ANYWAY overwritten by the saveParameter function to avoid
     *       overwriting of previously saved darkframe files.
     *  @cassttng
     *  GAINCalibrationFileName\n
     *       It contains the name of the files where GAIN calibrations will be taken out.\n
     *  @cassttng
     *  useHLLFormatGainCTEReader\n
     *       Use Mirkos Parser for the CTE Gain File from HLL. Default is false
     *       which case Nicolas Parser will be used.\n
     *  @author Nicola Coppola
     *
     */
    class CASS_PNCCDSHARED_EXPORT DetectorParameter
    {
    public:
      typedef std::vector<double> correctionmap_t;
    public:
      size_t          _nbrDarkframes;     //!< the number of frames used to calculate offset/noise map, this is an internal value is not "saved" in the calfiles
      correctionmap_t _offset;            //!< offset map
      correctionmap_t _noise;             //!< noise map
      correctionmap_t _gain_ao_CTE;       //!< Gain and/or Charge Transfair correction map

      uint32_t        _rebinfactor;       //!< the rebinfactor for rebinning
      double          _max_noise;         //!< pixels with noise larger _max_noise than will be masked
      double          _sigmaMultiplier;   //!< how big is "above noise"
      double          _adu2eV;            //!< conversion from adu to eV
      double          _CTE;               //!< single step Charge Transfer Efficiency
      bool            _createPixellist;   //!< flag to switch pixellist on / off
      bool            _doOffsetCorrection;//!< flag to switch offsetcorrection on / off
      bool            _useCommonMode;     //!< flag to switch a common mode subtraction scheme
      bool            _useCTECorr;        //!< flag to switch Charge transport efficiency Correction
      bool            _useGAINCorr;       //!< flag to switch Gain Correction
      bool            _auto_saveDarkframe;//!< flag to automatically save darkframe(s) to files as soon as a preper number of frames is reached.
      bool            _mask_BadPixel     ;//!< flag to mask or not bad pixels based on noise levels from darkframe(s)
      int64_t         _thres_for_integral;//!< the thresold for special integral
      std::string     _darkcalfilename;   //!< filename of file containing dark & noisemap
      std::string     _savedarkcalfilename;//!< Dark frame calibration save file names for each detector, it is automatically generated//
      std::string     _gainfilename;       //!< filename of file containing Gain and/or CTE values
      cass::detROI_   _detROI;

      cass::ROI::ROImask_t _ROImask;//!< The ROI mask
      cass::ROI::ROImask_t _ROImask_converter;
      cass::ROI::ROIiterator_t _ROIiterator;//!< The ROI iterators
      cass::ROI::ROIiterator_t _ROIiterator_converter;

      /** use Mirkos HLL Format reader */
      bool _useHLLFormatGainCTEReader;
    };

    /** parameters of the pnccd analysis
     *
     *  @cassttng
     *  IsDarkFrames\n
     *    True or false, default is false
     *  The following is a user settable parameter
     *  it decide for both pnCCD detectors if the following frames are
     *  to be used to calculate the offset corrections and noise map
     *  that are needed to calculate a corrected frame from the raw one.\n
     *
     *    After 200 Frames have been seen a warning will be printed out,
     *    the user can then send a "Save ini" command to CASS to save the
     *    calibration constants to files. The calculation will proceed anyway
     *    even after the user has save the darkcal-frames to file
     *    allowing for a still improved statistical accuracy, in case needed.\n
     *
     *    If IsDarkFrames is set to true all the previously described parameters
     *    are NOT active as also any ROI that may be defined in the CASS.ini file.\n
     *
     *    Unfortunately there is a similar parameter for the commercial CCD
     *    and they can be set to opposite values.
     *  @author Nicola Coppola
     */
    class CASS_PNCCDSHARED_EXPORT Parameter : public cass::CASSSettings
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
      bool            _isDarkframe;         //!< switch telling whether we are collecting darkframes right now
      bool            _This_is_a_dark_run;  //!< flag to set the dark/not-dark run condition
    };



    class pnCCDDevice;

    /** pre analysis of the pnCCD
     *
     * Put the pnCCDEvent object through the analysis chain.
     * A new corrected pnCCD image is generated and X-ray
     * photon hits are extracted if the user wishes to so. In addition, some
     * basic parameters are recorded, e.g. The integral over the corrected
     * frame, the integral over the corrected frame using only the pixel over
     * a certain threshold, the max value of the pixel over the frame.
     *
     * @author Nicola Coppola
     */
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
      Put the pnCCDEvent object through the analysis chain. 
      A new corrected pnCCD image is generated and X-ray
      photon hits are extracted if the user wishes to so. In addition, some
      basic parameters are recorded, e.g. The integral over the corrected frame,
      the integral over the corrected frame using only the pixel over a certain threshold,
      the max value of the pixel over the frame.
      */
      void operator() (CASSEvent*);

    private:
      void createOffsetAndNoiseMap(cass::pnCCD::pnCCDDevice&);
      void rebin(cass::pnCCD::pnCCDDevice&,size_t DetectorIndex);
      bool readGainCTE(DetectorParameter &dp);

    private:
      QReadWriteLock              _RWlock;//!< a mutex to lock write operations but allow read
      QMutex                      _mutex; //!< a mutex to lock write operations
      Parameter                   _param; //!< the parameters used to analyze the pnccd detectors
      cass::PixelDetector::frame_t  _tmp; //!< temporary storage for rebinning frames
    };
  } // end of scope of namespace pnCCD
} // end of scope of namespace cass

#endif
