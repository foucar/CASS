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

#include "postprocessor.h"
#include "backend.h"
#include "acqiris_detectors_helper.h"
#include "delayline_detector.h"

class sum_walk_calibration_class;
class scalefactors_calibration_class;
class sort_class;

namespace cass
{
/** Achims resort routine calibrator
 *
 * this class will use achims resort routine capabilties to calibrate
 * the timesum shift and the scalefactors
 *
 * To set up the channel assignment for the requested detector one needs to set
 * up the detector parameters.
 * @see cass::ACQIRIS::DelaylineDetector and
 *      cass::ACQIRIS::SignalProducer
 *
 * @cassttng PostProcessor/\%name\%/{Detector}\n
 *           Name of the delayline detector that we work on. Default is "blubb"
 *           See cass::ACQIRIS::HelperAcqirisDetectors. Speciality of this
 *           PostProcessor is, that it will only work with Hex Delayline detectors
 *           and the appropriate HexSorter. Make sure that all settings of the
 *           HexSorter are correctly set. See cass::ACQIRIS::HexSorter.
 *
 * @author Lutz Foucar
 */
class HexCalibrator
    : public PostprocessorBackend
{
public:
  /** enum for accessing the vectors */
  enum {mcp, u1, u2, v1, v2, w1, w2};
  enum {u, v, w};

  /** constructor */
  HexCalibrator(PostProcessors&, const PostProcessors::key_t&);

  /** create the calibration
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
   * @param evt the event to work on
   */
  void process(const CASSEvent& evt);

  /** load the detector analyzers settings from .ini file
   *
   * retrieve all necessary information to be able to calibrate the timesum
   * and the scalefactors. Next to this remember the the groupname of the
   * settings object, so that we later can use it to extract information
   */
  void loadSettings(size_t);

private:
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
  std::tr1::shared_ptr<scalefactors_calibration_class> _scalefactor_calibrator;

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

  /** the ratio to check whether the calibration can be started */
  double _ratio;

  /** the group name of the cass settings for this calibrator */
  QString _groupname;

  /** the .ini filename for the sorting information */
  std::string _calibrationFilename;

  /** the center of the image */
  std::pair<double,double> _center;

  /** the scalefactors
   *
   * the order in the array is given by the enums
   */
  std::vector<double> _scalefactors;

  /** the maximum runtime */
  double _maxRuntime;

  /** The detector we are there for*/
  ACQIRIS::HelperAcqirisDetectors::helperinstancesmap_t::key_type _detector;

  /** flag to tell wether the calibration has been written already */
  bool _calibwritten;
};
}//end namespace cass
#endif
