// Copyright (C) 2008-2011 Lutz Foucar

/**
 * @file achimcalibrator_hex.cpp file contains class that uses achims calibration
 *                               capabilities
 *
 * @author Lutz Foucar
 */

#include <cmath>
#include <cassert>

#include "achimcalibrator_hex.h"

#include "resorter/resort64c.h"
#include "cass_settings.h"
#include "histogram.h"
#include "convenience_functions.h"
#include "log.h"

using namespace cass;
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
        int direction(1);
        const double dpOSumu = direction*sums[HexCalibrator::u].first * 0.5;
        const double dpOSumv = direction*sums[HexCalibrator::v].first * 0.5;
        const double dpOSumw = direction*sums[HexCalibrator::w].first * 0.5;
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
       * @param layer The vector containing all the layers signals
       * @param center The center of the image in mm
       * @param scalefactors The scalefactors for the layers
       *
       * @author Achim Czasch
       * @author Lutz Foucar
       */
      void shift_pos(vector<double> &layer,
                     const pair<double,double> &center,
                     const vector<double> &scalefactors)
      {
        int direction(1);
        const double offs_u (0.50*center.first/scalefactors[HexCalibrator::u]);
        const double offs_v
            (0.25*(center.first - center.second *sqrt(3))/scalefactors[HexCalibrator::v]);
        const double offs_w
            (0.25*(center.first + center.second *sqrt(3))/scalefactors[HexCalibrator::w]);

        layer[HexCalibrator::u1] += direction*offs_u;
        layer[HexCalibrator::u1] -= direction*offs_u;
        layer[HexCalibrator::v2] += direction*offs_v;
        layer[HexCalibrator::v2] -= direction*offs_v;
        layer[HexCalibrator::w1] += direction*offs_w;
        layer[HexCalibrator::w2] -= direction*offs_w;
      }

      /** shift the w-layer
       *
       * use this function to align the w-layer to the u and v layer.
       *
       * @param w1 The first signal on the w-layer
       * @param w2 The second signal on the w-layer
       * @param w_offset The offset of the w-layer with respect to the u and v
       *                 layers
       *
       * @author Achim Czasch
       * @author Lutz Foucar
       */
      void shift_wLayer(double &w1, double &w2, const double w_offset)
      {
        int direction(1);
        const double w_offset_shift = direction * w_offset * 0.5;
        w1 += w_offset_shift;
        w2 -= w_offset_shift;
      }
    }
  }
}


HexCalibrator::HexCalibrator(PostProcessors &pp, const PostProcessors::key_t &key)
  :PostprocessorBackend(pp,key),
   _timesums(3,make_pair(0,0)),
   _scalefactors(2,1)
{
  loadSettings(0);
}

void HexCalibrator::loadSettings(size_t)
{
  _calibwritten = false;
  setupGeneral();
  if (!setupCondition())
    return;
  CASSSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(QString::fromStdString(_key));
  _detector = loadDelayDet(settings,161,_key);
  const DetectorBackend &rawdet(
        HelperAcqirisDetectors::instance(_detector)->detector());
  const DelaylineDetector &d (dynamic_cast<const DelaylineDetector&>(rawdet));
  if(!d.isHex())
    throw invalid_argument("HexCalibrator::loadSettings: Error The Hex-Sorter cannot work on '" +
                           d.name() + "' which is not a Hex Detector.");
  CASSSettings s;
  s.beginGroup("AcqirisDetectors");
  s.beginGroup(QString::fromStdString(_detector));
  s.beginGroup("HexSorting");
  _groupname = s.group();
  assert(_timesums.size() == 3);
  _timesums[u] = make_pair(s.value("TimeSumU",100).toDouble(),
                           s.value("TimeSumUWidth",0).toDouble());
  _timesums[v] = make_pair(s.value("TimeSumV",100).toDouble(),
                           s.value("TimeSumVWidth",0).toDouble());
  _timesums[w] = make_pair(s.value("TimeSumW",100).toDouble(),
                           s.value("TimeSumWWidth",0).toDouble());
  _center = make_pair(s.value("CenterX",0).toDouble(),
                      s.value("CenterY",0).toDouble());
  assert(_scalefactors.size() == 3);
  _scalefactors[u] = s.value("ScalefactorU",1).toDouble();
  _maxRuntime = s.value("MaxRuntime",130).toDouble();
  _calibrationFilename = s.value("SettingsFilename").toString().toStdString();
  QSettings hexsettings(QString::fromStdString(_calibrationFilename),
                        QSettings::defaultFormat());
  /** @todo make sure the below works */
  hexsettings.beginGroup(_groupname);
  _wLayerOffset = hexsettings.value("WLayerOffset",0).toDouble();
  _scalefactors[v] = hexsettings.value("ScalefactorV",1).toDouble();
  _scalefactors[w] = hexsettings.value("ScalefactorW",1).toDouble();
  s.endGroup();
  _tsum_calibrator =
      shared_ptr<sum_walk_calibration_class>(new sum_walk_calibration_class(49,true,_maxRuntime,0.1));
  _scalefactor_calibrator =
      shared_ptr<scalefactors_calibration_class>(new scalefactors_calibration_class(true,
                                                                                    _maxRuntime,
                                                                                    _maxRuntime*0.78,
                                                                                    _scalefactors[u],
                                                                                    _scalefactors[v],
                                                                                    _scalefactors[w]));
  _result = new Histogram0DFloat();
  createHistList(2*cass::NbrOfWorkers);
  Log::add(Log::INFO,"PostProcessor '" + _key + "' calibrates the hex detector '" +
           _detector + "'. Condition is '" + _condition->key() + "'");
}

void HexCalibrator::process(const CASSEvent &evt)
{
  DetectorBackend &rawdet(
        HelperAcqirisDetectors::instance(_detector)->detector(evt));
  DelaylineDetector &d (dynamic_cast<DelaylineDetector&>(rawdet));
  _result->lock.lockForWrite();
  vector<double> values(7);
  values[mcp] = d.mcp().firstGood();
  values[u1] = d.layers()['U'].wireends()['1'].firstGood();
  values[u2] = d.layers()['U'].wireends()['2'].firstGood();
  values[v1] = d.layers()['V'].wireends()['1'].firstGood();
  values[v2] = d.layers()['V'].wireends()['2'].firstGood();
  values[w1] = d.layers()['W'].wireends()['1'].firstGood();
  values[w2] = d.layers()['W'].wireends()['2'].firstGood();

  vector<double> layer(3);
  layer[u] = values[u1] - values[u2];
  layer[v] = values[v1] - values[v2];
  layer[w] = values[w1] - values[w2];

  vector<bool> layerChecksum(3);
  layerChecksum[u] = abs(values[u1]+values[u2]-2.*values[mcp]) < _timesums[u].second;
  layerChecksum[v] = abs(values[v1]+values[v2]-2.*values[mcp]) < _timesums[v].second;
  layerChecksum[w] = abs(values[w1]+values[w2]-2.*values[mcp]) < _timesums[w].second;

  AchimCalibrator::shift_sum(values,_timesums);
  AchimCalibrator::shift_pos(layer,_center,_scalefactors);
  AchimCalibrator::shift_wLayer(values[w1],values[w2],_wLayerOffset);

  if (layerChecksum[u] && layerChecksum[v] && layerChecksum[w])
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
  /** write the parameters when we are told that we can or our set threshold is
   *  reached.but only if it has not been written yet
   */
  if (!_calibwritten)
  {
    if (_scalefactor_calibrator->map_is_full_enough() ||
        _scalefactor_calibrator->get_ratio_of_full_bins() > _ratio)
    {
      _calibwritten = true;
      QSettings hexsettings(QString::fromStdString(_calibrationFilename),
                            QSettings::defaultFormat());
      /** @todo check whether one can set the goup this way */
      hexsettings.beginGroup(_groupname);
      AchimCalibrator::writeCalibData(hexsettings,_tsum_calibrator,_scalefactor_calibrator);
      hexsettings.endGroup();
    }
  }
  _result->lock.unlock();
}
