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
#include <vector>
#include <utility>

#include <QtCore/QMutex>
#include <QtCore/QString>

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

      /** enum for accessing the vectors */
      enum {mcp, u1, u2, v1, v2, w1, w2};
      enum {u, v, w};

      /** return our singleton instance
       *
       * @param detectorname the name of the detector that the calibrator is for
       */
      static shared_pointer instance(const std::string &detectorname);

      /** the operator
       *
       * this won't extract the detector hits but rather just fill the
       * calibrators with the values that they expect.
       *
       * In order to fill the scalefactor calibrator with only points that are
       * meaningful we check first the time sum for the hit we want to include.
       *
       * After we filled we check whether we can already output the calibration
       * data. We have enough when either we are told so or when the ratio is
       * better than what the user set as limit. If so, create the a QSettings
       * object that handles the ini file that will contain the calibration data.
       * Extract the name of the .ini file from the settings for this calibrator.
       *
       * @return reference to the hit container
       * @param hits the hitcontainer
       */
      detectorHits_t& operator()(detectorHits_t &hits);

      /** load the detector analyzers settings from .ini file
       *
       * retrieve all necessary information to be able to calibrate the timesum
       * and the scalefactors. Next to this remember the the groupname of the
       * settings object, so that we later can use it to extract information
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
      static std::map<std::string,shared_pointer> _instances;

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

      /** the timesums and their width
       *
       * the order is as follows (first is always timesum and second
       * the timesumwidth):
       * - 0: u layer
       * - 1: v layer
       * - 2: w layer
       */
      std::vector<std::pair<double,double> > _timesums;

      /** the w-layer offset */
      double _wLayerOffset;

      /** the signal producers of the hex detector in a vector
       *
       * the order is as follows:
       * - 0: mcp
       * - 1: u1
       * - 2: u2
       * - 3: v1
       * - 4: v2
       * - 5: w1
       * - 6: w2
       */
      std::vector<SignalProducer*> _sigprod;

      /** the ratio to check whether the calibration can be started */
      double _ratio;

      /** the group name of the cass settings for this calibrator */
      QString _groupname;

      /** the .ini filename for the sorting information */
      std::string _calibrationFilename;

      /** the center of the image */
      std::pair<double,double> _center;
    };

  }
}
#endif
