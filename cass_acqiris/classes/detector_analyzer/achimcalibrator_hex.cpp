// Copyright (C) 2008-2011 Lutz Foucar

/**
 * @file achimcalibrator_hex.cpp file contains class that uses achims calibration
 *                               capabilities
 *
 * @author Lutz Foucar
 */

#include "achimcalibrator_hex.h"

#include "resorter/resort64c.h"

using namespace cass::ACQIRIS;
using namespace std;


// =================define static members =================
HexCalibrator::shared_pointer HexCalibrator::_instance;
QMutex HexCalibrator::_mutex;

HexCalibrator::shared_pointer HexCalibrator::instance()
{
  QMutexLocker locker(&_mutex);
  if(!_instance)
  {
    _instance = shared_pointer(new HexCalibrator());
  }
  return _instance;
}
// ========================================================


HexCalibrator::HexCalibrator()
  :DetectorAnalyzerBackend(),
   _sorter(new sort_class())
{
//  /** @todo the below needs to be intialized at the right position. assuming that
//   *        the variables it uses have been intialized correctly
//   */
//  _tsum_calibrator =
//      tsumcalibratorPtr_t(sum_walk_calibration_class::new_sum_walk_calibration_class(_sorter.get(),49));
//  _scalefactor_calibrator =
//      scalefactorcalibratorPtr_t(new scalefactors_calibration_class(true,
//                                                                    _sorter->max_runtime*0.78,
//                                                                    0,
//                                                                    _sorter->fu,
//                                                                    _sorter->fv,
//                                                                    _sorter->fw));
}

detectorHits_t& HexCalibrator::operator()(detectorHits_t &hits)
{
  return hits;
}

void HexCalibrator::loadSettings(CASSSettings& s, DelaylineDetector &d)
{
}
