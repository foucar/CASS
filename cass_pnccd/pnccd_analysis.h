// Copyright (C) 2009 Jochen Küpper , Nils Kimmel

#ifndef PNCCDANALYSIS_H
#define PNCCDANALYSIS_H

#include <map>
#include <string>
#include <vector>
#include "cass_pnccd.h"
#include "analysis_backend.h"
#include "parameter_backend.h"

#include <QtGui/QImage>


namespace cass
{
  class CASSEvent;
  namespace pnCCD
  {

    /*
        Dark Frame calibration results of a pnCCD pixel array:
        */
    class pnCCDFrameCalibration
    {
    public:
      pnCCDFrameCalibration(void) {}
      ~pnCCDFrameCalibration() {}
    private:
    };

    /*
        X-ray photon pulse height calibration results of a
        pnCCD pixel array:
        */
    class pnCCDPulseHeightCal
    {
    public:
      pnCCDPulseHeightCal(void) {}
      ~pnCCDPulseHeightCal() {}
    private:
    };



    class CASS_PNCCDSHARED_EXPORT Parameter : public cass::ParameterBackend
    {
      /*
      Parameters needed for the pnCCDs. CAMP will typically use
      two one-megapixel detector modules consisting of two pnCCDs
      with 512 x 1024 pixels each.
      */
    public:
      /*
      Constructor: assign the parameters with safe default values:
      */
      Parameter(void):_rebinfactor(1) {beginGroup("pnCCD");}
      ~Parameter()    {endGroup();}
      void load();
      void save();
      /*
      Get the number of detectors which are described in this
      parameter class:
      */
      int getNumpnCCDPixArrays(void)
      { return frame_calibrations.size(); }
      /*
      The results of the dark frame calibration:
      pixel offset maps , pixel noise maps , bad pixel maps
      */
      std::vector<pnCCDFrameCalibration> frame_calibrations;
      /*
      The results of the pulse height calibration:
      gain correction factors of each channel , charge transfer
      inefficiency of each channel , a map which combines both
      gain and CTI in one value for each  pixel , a factor for a
      cubic non-linearity correction:
      */
      std::vector<pnCCDPulseHeightCal>   pulse_height_calibs;
      /*
      The analysis parameters for a CCD frame, e.g. the photon hit
      detection threshold in units of the noise sigma of each pixel:
      */
      double photon_hit_thresh_sigmaunits;

      //rebin factor//
      size_t _rebinfactor;
    };






    class pnCCDFrameAnalysis;

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
      Parameter _param;
      // The frame analysis object:
      pnCCDFrameAnalysis *pnccd_analysis_;

      //temporary storage if one wants to have the picture rebinned//
      std::vector<int16_t> _tmp;
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
