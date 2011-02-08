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

      /** shift the values so that the timesum peaks around 0
       *
       * @param values The values that need to be shifted
       * @param sums The sums that they need to be shifted by
       *
       * @author Achim Czasch
       * @author Lutz Foucar
       */
      void shift_sum(vector<double> &values, const vector<pair<double,double> > &sums)
      {
        const double dpOSumu = sums[HexCalibrator::u].first * 0.5;
        const double dpOSumv = sums[HexCalibrator::v].first * 0.5;
        const double dpOSumw = sums[HexCalibrator::w].first * 0.5;
        values[HexCalibrator::u1] += dpOSumu;
        values[HexCalibrator::u2] += dpOSumu;
        values[HexCalibrator::v1] += dpOSumv;
        values[HexCalibrator::v2] += dpOSumv;
        values[HexCalibrator::w1] += dpOSumw;
        values[HexCalibrator::w2] += dpOSumw;
      }

      /** shift the position
       *
       * when the image is not centered around 0 one can use this function to
       * shift it to 0
       *
       * @param
       *
       * @author Achim Czasch
       * @author Lutz Foucar
       */
      void shift_pos(vector<double> &layer, const pair<double,double> &center)
      {
//        __int32 i;
//        double offs_u,offs_v,offs_w;
//
//        if (this->use_HEX) {
//            offs_u = 0.50*(dpCOx_pos)                  /this->fu;
//            offs_v = 0.25*(dpCOx_pos - dpCOy_pos*SQRT3)/this->fv;
//            offs_w = 0.25*(dpCOx_pos + dpCOy_pos*SQRT3)/this->fw;
//
//            for (i = 0;i < count[Cu1];++i) tdc[_Cu1][i] = tdc[_Cu1][i] + direction*offs_u;
//            for (i = 0;i < count[Cu2];++i) tdc[_Cu2][i] = tdc[_Cu2][i] - direction*offs_u;
//            for (i = 0;i < count[Cv1];++i) tdc[_Cv1][i] = tdc[_Cv1][i] + direction*offs_v;
//            for (i = 0;i < count[Cv2];++i) tdc[_Cv2][i] = tdc[_Cv2][i] - direction*offs_v;
//            for (i = 0;i < count[Cw1];++i) tdc[_Cw1][i] = tdc[_Cw1][i] + direction*offs_w;
//            for (i = 0;i < count[Cw2];++i) tdc[_Cw2][i] = tdc[_Cw2][i] - direction*offs_w;
//        } else {
//            offs_u = 0.5 * dpCOx_pos / this->fu;
//            offs_v = 0.5 * dpCOy_pos / this->fv;
//
//            for (i = 0;i < count[Cu1];++i) tdc[_Cu1][i] = tdc[_Cu1][i] + direction*offs_u;
//            for (i = 0;i < count[Cu2];++i) tdc[_Cu2][i] = tdc[_Cu2][i] - direction*offs_u;
//            for (i = 0;i < count[Cv1];++i) tdc[_Cv1][i] = tdc[_Cv1][i] + direction*offs_v;
//            for (i = 0;i < count[Cv2];++i) tdc[_Cv2][i] = tdc[_Cv2][i] - direction*offs_v;
//        }
      }

      /** shift the w-layer
       *
       * use this function to align the w-layer to the u and v layer.
       *
       * @param
       *
       * @author Achim Czasch
       * @author Lutz Foucar
       */
      void shift_wLayer(double &w1, double &w2, const double w_offset)
      {
//        __int32 i;
//         if (this->use_HEX) {
//             w_offset *= direction*0.5;
//             for (i = 0;i < count[Cw1];++i) tdc[_Cw1][i] = tdc[_Cw1][i] + w_offset;
//             for (i = 0;i < count[Cw2];++i) tdc[_Cw2][i] = tdc[_Cw2][i] - w_offset;
//         }
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
   /** @note can one put these parameters here and later set them with the right
    *        information?
    */
   _scalefactor_calibrator(new scalefactors_calibration_class(true,150,0,1,1,1)),
   _timesums(3,make_pair(0,0))
{}

detectorHits_t& HexCalibrator::operator()(detectorHits_t &hits)
{
  vector<double> values;
  vector<SignalProducer*>::iterator it(_sigprod.begin());
  for (; it != _sigprod.end(); ++it)
    values.push_back((*it)->firstGood());
  vector<double> layer(3);
  for (size_t i(0); i < 3 ; ++i)
    layer[i] = values[i*2+1] - values[i*2+2];
  vector<bool> layerChecksum(3);
  for (size_t i(0); i < 3 ; ++i)
    layerChecksum[i] =
        abs(values[i*2+1]+values[i*2+2]-2.*values[mcp]) < _timesums[i].second;

  AchimCalibrator::shift_sum(values,_timesums);
  AchimCalibrator::shift_pos(layer,_center);
  AchimCalibrator::shift_wLayer(values[w1],values[w2],_wLayerOffset);

  if (layerChecksum[u] && layerChecksum[u] && layerChecksum[u])
    _scalefactor_calibrator->feed_calibration_data(layer[u],
                                                   layer[v],
                                                   layer[w],
                                                   layer[w]-_wLayerOffset);
  _tsum_calibrator->fill_sum_histograms(values[u1],
                                        values[u2],
                                        values[v1],
                                        values[v2],
                                        values[w1],
                                        values[w2],
                                        values[mcp]);

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
  _sigprod.clear();
  _sigprod.push_back(&d.mcp());
  _sigprod.push_back(&d.layers()['U'].wireends()['1']);
  _sigprod.push_back(&d.layers()['U'].wireends()['2']);
  _sigprod.push_back(&d.layers()['V'].wireends()['1']);
  _sigprod.push_back(&d.layers()['V'].wireends()['2']);
  _sigprod.push_back(&d.layers()['W'].wireends()['1']);
  _sigprod.push_back(&d.layers()['W'].wireends()['2']);
  s.beginGroup("HexSorting");
  _groupname = s.group();
  _timesums[0] = make_pair(s.value("TimeSumU",100).toDouble(),
                           s.value("TimeSumUWidth",0).toDouble());
  _timesums[1] = make_pair(s.value("TimeSumV",100).toDouble(),
                           s.value("TimeSumVWidth",0).toDouble());
  _timesums[2] = make_pair(s.value("TimeSumW",100).toDouble(),
                           s.value("TimeSumWWidth",0).toDouble());
  _center = make_pair(s.value("CenterX",0).toDouble(),
                      s.value("CenterY",0).toDouble());
  _calibrationFilename = s.value("SettingsFilename").toString().toStdString();
  QSettings hexsettings(QString::fromStdString(_calibrationFilename),
                        QSettings::defaultFormat());
  /** @todo make sure the below works */
  hexsettings.beginGroup(_groupname);
  _wLayerOffset = hexsettings.value("WLayerOffset",0).toDouble();
  s.endGroup();
}
