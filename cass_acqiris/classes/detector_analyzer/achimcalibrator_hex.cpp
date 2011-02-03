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


/** @todo make sure that the given standart parameters are ok */
HexCalibrator::HexCalibrator()
  :DetectorAnalyzerBackend(),
   _tsum_calibrator(new sum_walk_calibration_class(49,true,150,0.1)),
   _scalefactor_calibrator(new scalefactors_calibration_class(true,150,0,1,1,1))
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
  //	const double tsU = d.GetTsu();
  //	const double tsV = d.GetTsv();
  //	const double tsW = d.GetTsw();
  //
  //	const double u1 = (u1d.size())?u1d[0]:0;
  //	const double u2 = (u2d.size())?u2d[0]:0;
  //	const double v1 = (v1d.size())?v1d[0]:0;
  //	const double v2 = (v2d.size())?v2d[0]:0;
  //	const double w1 = (w1d.size())?w1d[0]:0;
  //	const double w2 = (w2d.size())?w2d[0]:0;
  //	const double mcp = (mcpd.size())?mcpd[0]:0;
  //
  //	const double u_ns = u1-u2;
  //	const double v_ns = v1-v2;
  //	const double w_ns = w1-w2;
  //
  //	const bool csu = (TMath::Abs( u1+u2-2.*mcp) < 10) && u1d.size() && u2d.size() && mcpd.size();
  //	const bool csv = (TMath::Abs( v1+v2-2.*mcp) < 10) && v1d.size() && v2d.size() && mcpd.size();
  //	const bool csw = (TMath::Abs( w1+w2-2.*mcp) < 10) && w1d.size() && w2d.size() && mcpd.size();
  //
  //	if (csu && csv && csw)
  //	{
  //		fSfc->feed_calibration_data(u_ns,v_ns,w_ns,w_ns-d.GetWOffset());
  //		rm.fill2d(fHiOff+kNonLinearityMap, fSfc->binx,fSfc->biny, fSfc->detector_map_fill);
  //	}
  //	fSwc->fill_sum_histograms();
  return hits;
}

void HexCalibrator::loadSettings(CASSSettings& s, DelaylineDetector &d)
{
}
