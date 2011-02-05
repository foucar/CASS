// Copyright (C) 2008-2011 Lutz Foucar

/**
 * @file achimcalibrator_hex.cpp file contains class that uses achims calibration
 *                               capabilities
 *
 * @author Lutz Foucar
 */

#include <cmath>

#include "achimcalibrator_hex.h"

#include "resorter/resort64c.h"
#include "cass_settings.h"

using namespace cass::ACQIRIS;
using namespace std;
using namespace std::tr1;

namespace cass
{
  namespace ACQIRIS
  {
    namespace AchimCalibrator
    {

      /** write the profile part of calibration data into an ini file
       *
       * extract the calibration data from the calibrators and write them into
       * the requested ini file which will be handled by the QSettings object
       *
       * extract the profile data from the layer sum profiles, that later
       * correct the timesum.
       *
       * @param s the QSettings object that handles the .ini file
       * @param profile the profile that whos stuff needs to be written to the
       *                .ini file
       *
       * @author Lutz Foucar
       */
      void writeProfileData(QSettings &s, profile_class& profile)
      {
//        tsum_calibrator->generate_sum_walk_profiles();
//        if (tsum_calibrator->sumu_profile)
        {
          s.beginWriteArray("SumUCorrectionPoints");
          const int size (profile.number_of_columns);
          for (int i = 0; i < size; ++i)
          {
            s.setArrayIndex(i);
            const double pos
                (profile.get_bin_center_x(static_cast<double>(i)));
            const double cor (profile.get_y(i));
            s.setValue("Position",pos );
            s.setValue("Correction",cor );
          }
          s.endArray();
        }
      }

      /** write all calibration data into an ini file
       *
       * use the calibrators to extract the calibration data and then write that
       * into the .ini file that is handled by a QSettings Object.
       *
       * First generate the time sum profiles and write the correction data that
       * is contained in the profiles to the .ini file.
       * Then use the created histogram of the detector to get the scalefactors
       * and the w layer offset.
       *
       * @param s the QSettings object that handles the sorter/calibrator .ini
       *          file
       * @param tsum_calibrator the timesum calibrator that hold infomration
       *                        about the timesum calibration data
       * @param scalefactor_calibrator the scalefactor calibrator that holds
       *                               information about the scalefactor
       *                               calibration data.
       *
       * @author Lutz Foucar
       */
      void writeCalibData(QSettings &s,
                          shared_ptr<sum_walk_calibration_class> tsum_calibrator,
                          shared_ptr<scalefactors_calibration_class>  scalefactor_calibrator)
      {
        tsum_calibrator->generate_sum_walk_profiles();
        if (tsum_calibrator->sumu_profile)
        {
          s.beginWriteArray("SumUCorrectionPoints");
          writeProfileData(s,*tsum_calibrator->sumu_profile);
          s.endArray();
        }
        if (tsum_calibrator->sumv_profile)
        {
          s.beginWriteArray("SumVCorrectionPoints");
          writeProfileData(s,*tsum_calibrator->sumv_profile);
          s.endArray();
        }
        if (tsum_calibrator->sumw_profile)
        {
          s.beginWriteArray("SumWCorrectionPoints");
          writeProfileData(s,*tsum_calibrator->sumw_profile);
          s.endArray();
        }
        scalefactor_calibrator->
            do_auto_calibration(s.value("WLayerOffset",0).toDouble());
        s.setValue("ScalefactorV",scalefactor_calibrator->best_fv);
        s.setValue("ScalefactorW",scalefactor_calibrator->best_fw);
        s.setValue("WLayerOffset",scalefactor_calibrator->best_w_offset);
      }
    }
  }
}
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
   _scalefactor_calibrator(new scalefactors_calibration_class(true,150,0,1,1,1)),
   _timesums(3,make_pair(0,0))

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
  const double mcp (_sigprod[0]->firstGood());
  const double u1 (_sigprod[1]->firstGood());
  const double u2 (_sigprod[2]->firstGood());
  const double v1 (_sigprod[3]->firstGood());
  const double v2 (_sigprod[4]->firstGood());
  const double w1 (_sigprod[5]->firstGood());
  const double w2 (_sigprod[6]->firstGood());

  const double u_ns (u1-u2);
  const double v_ns (v1-v2);
  const double w_ns (w1-w2);

  const bool csu ((abs(u1+u2-2.*mcp-_timesums[0].first) < _timesums[0].second));
  const bool csv ((abs(v1+v2-2.*mcp-_timesums[1].first) < _timesums[1].second));
  const bool csw ((abs(w1+w2-2.*mcp-_timesums[2].first) < _timesums[2].second));

  if (csu && csv && csw)
  {
    _scalefactor_calibrator->feed_calibration_data(u_ns,v_ns,w_ns,w_ns-_wLayerOffset);
  }
  _tsum_calibrator->fill_sum_histograms();

  if (_scalefactor_calibrator->map_is_full_enough() ||
      _scalefactor_calibrator->get_ratio_of_full_bins() > _ratio)
  {
    QSettings hexsettings(QString::fromStdString(_calibrationFilename),
                          QSettings::defaultFormat());
    /** @todo check whether one can set the goup this way */
    hexsettings.beginGroup(_groupname);
    AchimCalibrator::writeCalibData(hexsettings,_tsum_calibrator,_scalefactor_calibrator);
  }
  return hits;
}

void HexCalibrator::loadSettings(CASSSettings& s, DelaylineDetector &d)
{
  if(!d.isHex())
  {
    stringstream ss;
    ss << "HexCalibrator::loadSettings: Error The Hex-Sorter cannot work on '"<<d.name()
        << "' which is not a Hex Detector.";
    throw invalid_argument(ss.str());
  }
  s.beginGroup("HexSorting");
  _groupname = s.group();
  _timesums[0] = make_pair(s.value("TimeSumU",100).toDouble(),
                           s.value("TimeSumUWidth",0).toDouble());
  _timesums[1] = make_pair(s.value("TimeSumV",100).toDouble(),
                           s.value("TimeSumVWidth",0).toDouble());
  _timesums[2] = make_pair(s.value("TimeSumW",100).toDouble(),
                           s.value("TimeSumWWidth",0).toDouble());
  _calibrationFilename = s.value("SettingsFilename").toString().toStdString();
  QSettings hexsettings(QString::fromStdString(_calibrationFilename),
                        QSettings::defaultFormat());
  /** @todo make sure the below works */
  hexsettings.beginGroup(_groupname);
  _wLayerOffset = hexsettings.value("WLayerOffset",0).toDouble();
  s.endGroup();
}
