//Copyright (C) 2008-2011 Lutz Foucar

/**
 * @file achimsorter_hex.cpp file contains class that uses achims resort routine
 *
 * @author Lutz Foucar
 */

#include <stdexcept>
#include <sstream>
#include <algorithm>

#include "achimsorter_hex.h"

#include "resorter/resort64c.h"
#include "cass_settings.h"

using namespace cass::ACQIRIS;
using namespace std;

namespace cass
{
namespace ACQIRIS
{
namespace AchimHex
{
/** extract times from signal producer
 *
 * extract the time values from the signal producer and puts it into
 * the corrosponding tdc like array
 *
 * @param thePair container for tdc like array mapped to the corrosponding
 *                signalproducer.
 *
 * @author Lutz Foucar
 */
void extactTimes(pair<SignalProducer*,vector<double> > & thePair)
{
  vector<double> &tdcarray(thePair.second);
  tdcarray.clear();
  SignalProducer::signals_t &sigs(thePair.first->output());
  SignalProducer::signals_t::const_iterator sigsIt(sigs.begin());
  for (;sigsIt !=sigs.end(); ++sigsIt)
    tdcarray.push_back((*sigsIt)["time"]);
}
}//end namespace AchimHex
}//end namespace acqiris
}//end namespace cass




HexSorter::HexSorter()
  :DetectorAnalyzerBackend(),
   _sorter(new sort_class()),
   _count(7,0),
   _timesums(3,0)
{
  _sorter->Cmcp = 0;
  _sorter->Cu1  = 1;
  _sorter->Cu2  = 2;
  _sorter->Cv1  = 3;
  _sorter->Cv2  = 4;
  _sorter->Cw1  = 5;
  _sorter->Cw2  = 6;
  _sorter->count = &_count.front();
  _sorter->TDC_resolution_ns = 0.1;
  _sorter->tdc_array_row_length = 1000;
  _sorter->dont_overwrite_original_data = true;
  _sorter->use_pos_correction = false;
  _sorter->common_start_mode = true;
  _sorter->use_HEX = true;

  //this is needed to tell achims routine that we care for our own arrays//
  for (size_t i=0;i<7;++i)
    _sorter->tdc[i] = (double*)(0x1);
}

detectorHits_t& HexSorter::operator()(detectorHits_t &hits)
{
  for_each(_signals.begin(),_signals.end(),AchimHex::extactTimes);
  //assign the tdc arrays and copy the number of found signals
  for (size_t i(0); i<7;++i)
  {
    if (!_signals[i].second.empty())
      _sorter->tdc[i]  = &_signals[i].second.front();
    _count[i] = _signals[i].second.size();
  }
  // shift all time sums to zero
  _sorter->shift_sums(+1,_timesums[0],_timesums[1],_timesums[2]);
  // shift layer w so that all center lines of the layers meet in one point
  _sorter->shift_layer_w(+1,_wLayerOffset);
  // shift all layers so that the position picture is centered around X=zero,Y=zero
  _sorter->shift_position_origin(+1,_center.first,_center.second);
  int32_t nbrOfRecHits = _sorter->sort();
  //copy the reconstructed hits to our dethits//
  for (int i(0);i<nbrOfRecHits;++i)
  {
    detectorHit_t hit;
    hit["x"] = _sorter->output_hit_array[i]->x;
    hit["y"] = _sorter->output_hit_array[i]->y;
    hit["t"] = _sorter->output_hit_array[i]->time;
    hit["method"] = _sorter->output_hit_array[i]->method;
    hits.push_back(hit);
  }

  return hits;
}

void HexSorter::loadSettings(CASSSettings& s, DelaylineDetector &d)
{
  if(!d.isHex())
    throw invalid_argument("HexSorter::loadSettings: Error The Hex-Sorter cannot work on '" +
                           d.name() + "' which is not a Hex Detector.");
  _signals.clear();
  _signals.push_back(make_pair(&d.mcp(),vector<double>()));
  _signals.push_back(make_pair(&d.layers()['U'].wireends()['1'],vector<double>()));
  _signals.push_back(make_pair(&d.layers()['U'].wireends()['2'],vector<double>()));
  _signals.push_back(make_pair(&d.layers()['V'].wireends()['1'],vector<double>()));
  _signals.push_back(make_pair(&d.layers()['V'].wireends()['2'],vector<double>()));
  _signals.push_back(make_pair(&d.layers()['W'].wireends()['1'],vector<double>()));
  _signals.push_back(make_pair(&d.layers()['W'].wireends()['2'],vector<double>()));

  /** @todo add the loading of the settings, make sure they are documented. */

  s.beginGroup("HexSorting");
  _sorter->uncorrected_time_sum_half_width_u = s.value("TimeSumUWidth",0).toDouble();
  _sorter->uncorrected_time_sum_half_width_v = s.value("TimeSumVWidth",0).toDouble();
  _sorter->uncorrected_time_sum_half_width_w = s.value("TimeSumWWidth",0).toDouble();
  _timesums[0] = s.value("TimeSumU",100).toDouble();
  _timesums[1] = s.value("TimeSumV",100).toDouble();
  _timesums[2] = s.value("TimeSumW",100).toDouble();
  _sorter->max_runtime = s.value("MaxRuntime",130).toDouble();
  _sorter->dead_time_anode = s.value("DeadTimeAnode",20).toDouble();
  _sorter->dead_time_mcp = s.value("DeadTimeMCP",20).toDouble();
  _sorter->MCP_radius = s.value("MCPRadius",88).toDouble();
  _sorter->use_MCP = s.value("UseMCP",true).toBool();
  _sorter->fu = s.value("ScalefactorU",1).toDouble();
  _center = make_pair(s.value("CenterX",0).toDouble(),
                      s.value("CenterY",0).toDouble());


  //the following settings can be retrieved from the calibration file
  string settingsfilename (s.value("SettingsFilename").toString().toStdString());
  QSettings hexsettings(QString::fromStdString(settingsfilename),
                        QSettings::defaultFormat());
  /** @todo check whether one can set the goup this way */
  hexsettings.beginGroup(s.group());
  _sorter->fv = hexsettings.value("ScalefactorV",1).toDouble();
  _sorter->fw = hexsettings.value("ScalefactorW",1).toDouble();
  _wLayerOffset = hexsettings.value("WLayerOffset",0).toDouble();
  int size = hexsettings.beginReadArray("SumUCorrectionPoints");
  _sorter->use_sum_correction = static_cast<bool>(size);
  for (int i = 0; i< size; ++i)
  {
    hexsettings.setArrayIndex(i);
    _sorter->
        signal_corrector->
        sum_corrector_U->
        set_point(hexsettings.value("Position").toDouble(),
                  hexsettings.value("Correction").toDouble());
  }
  hexsettings.endArray();
  size = hexsettings.beginReadArray("SumVCorrectionPoints");
  for (int i = 0; i < size; ++i)
  {
    hexsettings.setArrayIndex(i);
    _sorter->
        signal_corrector->
        sum_corrector_V->
        set_point(hexsettings.value("Position").toDouble(),
                  hexsettings.value("Correction").toDouble());
  }
  hexsettings.endArray();
  size = hexsettings.beginReadArray("SumWCorrectionPoints");
  for (int i = 0; i < size; ++i)
  {
    hexsettings.setArrayIndex(i);
    _sorter->
        signal_corrector->
        sum_corrector_W->
        set_point(hexsettings.value("Position").toDouble(),
                  hexsettings.value("Correction").toDouble());
  }
  hexsettings.endArray();

  int error_code = _sorter->init_after_setting_parameters();
  if (error_code != 0)
  {
    char error_text[500];
    _sorter->get_error_text(error_code,500,error_text);
    throw invalid_argument("HexSorter::loadSettings: Error '" + toString(error_code) +
                           "' while trying to initialize the sorter: '" + error_text + "'");
  }
}
