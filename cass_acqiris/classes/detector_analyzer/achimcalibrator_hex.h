// Copyright (C) 2008-2011 Lutz Foucar

/**
 * @file achimcalibrator_hex.h file contains class that uses achims calibration
 *                             capabilities
 *
 * @author Lutz Foucar
 */

#ifndef _ACHIMCALIBRATOR_HEX_H_
#define _ACHIMCALIBRATOR_HEX_H_

#include <tr1/memory>

#include <QtCore/QMutex>

#include "delayline_detector_analyzer_backend.h"
#include "delayline_detector.h"

class sum_walk_calibration_class;
class scalefactors_calibration_class;
class sort_class;

namespace cass
{
  namespace ACQIRIS
  {
    /** Achims resort routine calibrator
     *
     * this class will use achims resort routine capabilties to calibrate
     * the timesum shift and the scalefactors
     *
     * @author Lutz Foucar
     */
    class CASS_ACQIRISSHARED_EXPORT HexCalibrator
      : public DetectorAnalyzerBackend
    {
    public:
      /** typedef of this instance */
      typedef std::tr1::shared_ptr<HexCalibrator> shared_pointer;

      /** return our singleton instance */
      static shared_pointer instance();

      /** the operator
       *
       * this won't extract the detector hits but rather just fill the
       * calibrators with the values that they expect.
       *
       * @return reference to the hit container
       * @param hits the hitcontainer
       */
      detectorHits_t& operator()(detectorHits_t &hits);

      /** load the detector analyzers settings from .ini file
       *
       * retrieve all necessary information to be able to calibrate the timesum
       * and the scalefactors
       *
       * @param s the CASSSetting object
       * @param d the detector object that we are belonging to
       */
      void loadSettings(CASSSettings &s, DelaylineDetector &d);

    private:
      /** constructor
       *
       * private because this should be a singleton
       * creates and intitializes the achims routine
       */
      HexCalibrator();

      /** an instance of this */
      static shared_pointer _instance;

      /** singleton mutex */
      static QMutex _mutex;

      /** the time sum calibrator
       *
       * this will take the timesum and after a while it knows how to correct
       * the timesum to be a straight line
       */
      std::tr1::shared_ptr<sum_walk_calibration_class> _tsum_calibrator;

      /** pointer to scalfactor calibrator
       *
       * this is a class that will help finding the scalefactor and the
       * w-Layer offset of the Hex-Anode.
       */
      std::tr1::shared_ptr<scalefactors_calibration_class>  _scalefactor_calibrator;

    };

  }
}
#endif
