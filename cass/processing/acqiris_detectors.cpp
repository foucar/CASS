//Copyright (C) 2010 Lutz Foucar

/**
 * @file acqiris_detectors.cpp file contains definition of processors that
 *                             extract information of acqiris detectors.
 *
 * @author Lutz Foucar
 */

#include <stdexcept>
#include <cmath>
#include <algorithm>
#include <tr1/memory>

#include <QtCore/QString>

#include "acqiris_detectors.h"
#include "acqiris_detectors_helper.h"
#include "histogram.h"
#include "cass_event.h"
#include "acqiris_device.hpp"
#include "cass.h"
#include "convenience_functions.h"
#include "cass_settings.h"
#include "log.h"

using namespace cass;
using namespace ACQIRIS;
using namespace std;
using std::tr1::dynamic_pointer_cast;
using std::tr1::shared_ptr;

namespace cass
{
namespace ACQIRIS
{
/** load layer from file
 *
 * load the requested layer from .ini file and checks whether it is valid.
 * If it is not valid an invalid_argument exception is thrown
 *
 * @return key containing the layer name
 * @param s CASSSettings object to read the info from
 * @param detector the name of the detector that contains the layer
 * @param layerKey key how the layer value is called in the .ini file
 * @param ppNbr the processor number of the processor calling this function
 * @param key the key of the processor calling this function
 *
 * @author Lutz Foucar
 */
DelaylineDetector::anodelayers_t::key_type loadLayer(CASSSettings &s,
                                                     const HelperAcqirisDetectors::helperinstancesmap_t::key_type &detector,
                                                     const std::string &layerKey,
                                                     int ppNbr,
                                                     const string& key)
{
  HelperAcqirisDetectors::shared_pointer dethelp (HelperAcqirisDetectors::instance(detector));
  DelaylineDetector::anodelayers_t::key_type layer
      (s.value(layerKey.c_str(),"U").toString()[0].toLatin1());
  if (layer != 'U' && layer != 'V' && layer != 'W' &&
      layer != 'X' && layer != 'Y')
  {
    throw invalid_argument("pp" + toString(ppNbr) + "::loadSettings()'" + key +
                           "': The loaded value of '" + layerKey +"' '"+ layer +
                           "' does not exist. Can only be 'U', 'V', 'W', 'X' or 'Y'");
  }
  else if (dynamic_cast<const DelaylineDetector&>(dethelp->detector()).isHex())
  {
    if (layer == 'X' || layer == 'Y')
      throw invalid_argument("pp"+ toString(ppNbr) + "::loadSettings()'"+ key +
                             "': Detector '" + detector +
                             "' is Hex-detector and cannot have Layer '" +layer +"'");
  }
  else
  {
    if (layer == 'U' || layer == 'V' || layer == 'W')
      throw invalid_argument("pp" + toString(ppNbr) + "::loadSettings()'" + key +
                             "': Detector '" + detector +
                             "' is Quad-detector and cannot have Layer '" + layer + "'");
  }
  return layer;
}

/** load wireend from file
 *
 * load the requested wireend from .ini file. Check whether it is a valid
 * wireend otherwise throw invalid_argument exception.
 *
 * @return key containing the wireend name
 * @param s CASSSettings object to read the info from
 * @param wireendKey key how the wireend value is called in the .ini file
 * @param ppNbr the processor number of the processor calling this function
 * @param key the key of the processor calling this function
 *
 * @author Lutz Foucar
 */
AnodeLayer::wireends_t::key_type loadWireend(CASSSettings &s,
                                             const std::string & wireendKey,
                                             int ppNbr,
                                             const string& key)
{
  AnodeLayer::wireends_t::key_type wireend
      (s.value(wireendKey.c_str(),"1").toString()[0].toLatin1());
  if (wireend != '1' && wireend != '2')
    throw invalid_argument("pp" + toString(ppNbr) + "::loadSettings()'" + key +
                           "': The loaded value of '" + wireendKey +
                           "' '" + wireend + "' does not exist. Can only be '1' or '2'");
  return wireend;
}
}//end namespace acqiris
}//end namespace cass



//----------------Nbr of Peaks MCP---------------------------------------------
pp150::pp150(const name_t &name)
  : Processor(name)
{
  loadSettings(0);
}

void pp150::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("Processor");
  s.beginGroup(QString::fromStdString(name()));
  setupGeneral();
  if (!setupCondition())
    return;
  _detector = s.value("Detector","blubb").toString().toStdString();
  HelperAcqirisDetectors::instance(_detector)->loadSettings();
  createHistList(tr1::shared_ptr<Histogram0DFloat>(new Histogram0DFloat()));
  Log::add(Log::INFO,"Processor '" + name() +
           "' retrieves the nbr of mcp signals of detector '" + _detector +
           "'. Condition is '" + _condition->name() + "'");
}

void pp150::process(const CASSEvent &evt, HistogramBackend &res)
{
  DetectorBackend &rawdet(
      HelperAcqirisDetectors::instance(_detector)->detector(evt));
  TofDetector &det(dynamic_cast<TofDetector&>(rawdet));

  Histogram0DFloat &result(dynamic_cast<Histogram0DFloat&>(res));

  result.fill(det.mcp().output().size());
}










//----------------MCP Hits (Tof)-----------------------------------------------
pp151::pp151(const name_t &name)
  : Processor(name)
{
  loadSettings(0);
}

void pp151::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("Processor");
  s.beginGroup(QString::fromStdString(name()));
  setupGeneral();
  if (!setupCondition())
    return;
  _detector = s.value("Detector","blubb").toString().toStdString();
  HelperAcqirisDetectors::instance(_detector)->loadSettings();
  createHistList(set1DHist(name()));
  Log::add(Log::INFO,"Processor '" + name() +
           "' histograms times of the found mcp signals of detector '" + _detector +
           "'. Condition is '"+ _condition->name() + "'");
}

void pp151::process(const CASSEvent &evt, HistogramBackend &res)
{
  DetectorBackend &rawdet(
      HelperAcqirisDetectors::instance(_detector)->detector(evt));
  TofDetector &det(dynamic_cast<TofDetector&>(rawdet));
  SignalProducer::signals_t::const_iterator it (det.mcp().output().begin());
  SignalProducer::signals_t::const_iterator end (det.mcp().output().end());

  Histogram1DFloat &result(dynamic_cast<Histogram1DFloat&>(res));

  result.clear();
  while ( it != end )
    result.fill((*it++)[ACQIRIS::time]);
}










//----------------MCP Fwhm vs. height------------------------------------------
pp152::pp152(const name_t &name)
  : Processor(name)
{
  loadSettings(0);
}

void pp152::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("Processor");
  s.beginGroup(QString::fromStdString(name()));
  setupGeneral();
  if (!setupCondition())
    return;
  _detector = s.value("Detector","blubb").toString().toStdString();
  HelperAcqirisDetectors::instance(_detector)->loadSettings();
  createHistList(set2DHist(name()));
  Log::add(Log::INFO,"Processor '" + name() +
           "' histograms the FWHM vs the height of the found mcp signals" +
           " of detector '" + _detector + "'. Condition is '" +
           _condition->name() + "'");
}

void pp152::process(const CASSEvent &evt, HistogramBackend &res)
{
  DetectorBackend &rawdet(
      HelperAcqirisDetectors::instance(_detector)->detector(evt));
  TofDetector &det(dynamic_cast<TofDetector&>(rawdet));
  SignalProducer::signals_t::const_iterator it(det.mcp().output().begin());
  SignalProducer::signals_t::const_iterator end(det.mcp().output().end());

  Histogram2DFloat &result(dynamic_cast<Histogram2DFloat&>(res));

  result.clear();
  for (; it != end; ++it)
    result.fill((*it)[fwhm],(*it)[height]);
}








//----------------Deadtime between consecutive MCP signals----------------------
pp153::pp153(const name_t &name)
  : Processor(name)
{
  loadSettings(0);
}

void pp153::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("Processor");
  s.beginGroup(QString::fromStdString(name()));
  setupGeneral();
  if (!setupCondition())
    return;
  _detector = s.value("Detector","blubb").toString().toStdString();
  HelperAcqirisDetectors::instance(_detector)->loadSettings();
  createHistList(set1DHist(name()));
  Log::add(Log::INFO,"Processor '" + name() +
           "' creates a histogram of the deatime between two consecutive " +
           "MCP Signals of detctor '" + _detector +
           "'. Condition is '" + _condition->name() + "'");
}

void pp153::process(const CASSEvent& evt, HistogramBackend &res)
{
  DetectorBackend &rawdet(
      HelperAcqirisDetectors::instance(_detector)->detector(evt));
  TofDetector &det(dynamic_cast<TofDetector&>(rawdet));
  const SignalProducer::signals_t& mcp(det.mcp().output());

  Histogram1DFloat &result(dynamic_cast<Histogram1DFloat&>(res));

  result.clear();
  for (size_t i(1); i < mcp.size(); ++i)
  {
    const float diff(mcp[i-1][ACQIRIS::time] - mcp[i][ACQIRIS::time]);
    result.fill(diff);
  }
}












//----------------Nbr of Peaks Anode-------------------------------------------
pp160::pp160(const name_t &name)
  : Processor(name)
{
  loadSettings(0);
}

void pp160::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("Processor");
  s.beginGroup(QString::fromStdString(name()));
  setupGeneral();
  if (!setupCondition())
    return;
  _detector = loadDelayDet(s,160,name());
  _layer = loadLayer(s,_detector,"Layer",160,name());
  _signal = loadWireend(s,"Wireend",160,name());
  createHistList(tr1::shared_ptr<Histogram0DFloat>(new Histogram0DFloat()));
  Log::add(Log::INFO,"Processor '" + name() +
           "' outputs the nbr of signals of layer '" + _layer + "' wireend '" +
           _signal + "' of detector '" + _detector +"'. Condition is '" +
           _condition->name() + "'");
}

void pp160::process(const CASSEvent& evt, HistogramBackend &res)
{
  DetectorBackend &rawdet(
      HelperAcqirisDetectors::instance(_detector)->detector(evt));
  DelaylineDetector &det (dynamic_cast<DelaylineDetector&>(rawdet));

  Histogram0DFloat &result(dynamic_cast<Histogram0DFloat&>(res));

  result.fill(det.layers()[_layer].wireends()[_signal].output().size());
}











//----------------FWHM vs. Height of Wireend Signals---------------------------
pp161::pp161(const name_t &name)
  : Processor(name)
{
  loadSettings(0);
}

void pp161::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("Processor");
  s.beginGroup(QString::fromStdString(name()));
  setupGeneral();
  if (!setupCondition())
    return;
  _detector = loadDelayDet(s,161,name());
  _layer = loadLayer(s,_detector,"Layer",161,name());
  _signal = loadWireend(s,"Wireend",161,name());
  createHistList(set2DHist(name()));
  Log::add(Log::INFO,"Processor '" + name() +
           "' histograms the FWHM vs the height from the signals of layer '" +
           _layer + "' wireend '" + _signal + "' of detector '" + _detector +
           "'. Condition is '" + _condition->name() + "'");
}

void pp161::process(const CASSEvent& evt, HistogramBackend &res)
{
  DetectorBackend &rawdet(
      HelperAcqirisDetectors::instance(_detector)->detector(evt));
  DelaylineDetector &det (dynamic_cast<DelaylineDetector&>(rawdet));
  SignalProducer::signals_t::const_iterator it (det.layers()[_layer].wireends()[_signal].output().begin());
  SignalProducer::signals_t::const_iterator end (det.layers()[_layer].wireends()[_signal].output().end());

  Histogram2DFloat &result(dynamic_cast<Histogram2DFloat&>(res));

  result.clear();
  for (; it != end; ++it)
    result.fill((*it)[fwhm],(*it)[height]);
}










//----------------Timesum for the layers---------------------------------------
pp162::pp162(const name_t &name)
  : Processor(name)
{
  loadSettings(0);
}

void pp162::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("Processor");
  s.beginGroup(QString::fromStdString(name()));
  setupGeneral();
  if (!setupCondition())
    return;
  _detector = loadDelayDet(s,162,name());
  _layer = loadLayer(s,_detector,"Layer",162,name());
  _range = make_pair(s.value("TimeRangeLow",0).toDouble(),
                     s.value("TimeRangeHigh",20000).toDouble());
  createHistList(tr1::shared_ptr<Histogram0DFloat>(new Histogram0DFloat()));
  Log::add(Log::INFO,"Processor '" + name() +
           "' calculates the timesum of layer '" + _layer + "' of detector '" +
           _detector + "'. It will use the first signals that appeared in the" +
           "ToF range from '" + toString(_range.first) + "' ns to '" +
           toString(_range.second) + "' ns. Condition is '" + _condition->name() + "'");
}

void pp162::process(const CASSEvent& evt, HistogramBackend &res)
{
  DetectorBackend &rawdet(
        HelperAcqirisDetectors::instance(_detector)->detector(evt));
  DelaylineDetector &det (dynamic_cast<DelaylineDetector&>(rawdet));
  const double one (det.layers()[_layer].wireends()['1'].firstGood(_range));
  const double two (det.layers()[_layer].wireends()['2'].firstGood(_range));
  const double mcp (det.mcp().firstGood(_range));

  Histogram0DFloat &result(dynamic_cast<Histogram0DFloat&>(res));

  result.fill( one + two - 2.*mcp);
}










//----------------Timesum vs Position for the layers--------------------------
pp163::pp163(const name_t &name)
  : Processor(name)
{
  loadSettings(0);
}

void pp163::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("ostProcessor");
  s.beginGroup(QString::fromStdString(name()));
  setupGeneral();
  if (!setupCondition())
    return;
  _detector = loadDelayDet(s,163,name());
  _layer = loadLayer(s,_detector,"Layer",163,name());
  _range = make_pair(s.value("TimeRangeLow",0).toDouble(),
                     s.value("TimeRangeHigh",20000).toDouble());
  createHistList(set2DHist(name()));
  Log::add(Log::INFO,"Processor '" + name() +
           "' histograms the timesum vs Positon on layer '" + _layer + "' of detector '" +
           _detector + "'. Condition is '" + _condition->name() + "'");
}

void pp163::process(const CASSEvent& evt, HistogramBackend &res)
{
  DetectorBackend &rawdet(
        HelperAcqirisDetectors::instance(_detector)->detector(evt));
  DelaylineDetector &det (dynamic_cast<DelaylineDetector&>(rawdet));
  const double one (det.layers()[_layer].wireends()['1'].firstGood(_range));
  const double two (det.layers()[_layer].wireends()['2'].firstGood(_range));
  const double mcp (det.mcp().firstGood(_range));
  const double timesum (one + two - 2.*mcp);
  const double position (one - two);

  Histogram2DFloat &result(dynamic_cast<Histogram2DFloat&>(res));

  result.clear();
  result.fill(position,timesum);
}











//----------------Detector First Hit-------------------------------------------
pp164::pp164(const name_t &name)
  : Processor(name)
{
  loadSettings(0);
}

void pp164::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("Processor");
  s.beginGroup(QString::fromStdString(name()));
  setupGeneral();
  if (!setupCondition())
    return;
  _detector = loadDelayDet(s,164,name());
  _first = loadLayer(s,_detector,"FirstLayer",164,name());
  _second = loadLayer(s,_detector,"SecondLayer",164,name());
  _range = make_pair(s.value("TimeRangeLow",0).toDouble(),
                     s.value("TimeRangeHigh",20000).toDouble());
  _tsrange = make_pair(make_pair(s.value("TimesumFirstLayerLow",20).toDouble(),
                                 s.value("TimesumFirstLayerHigh",200).toDouble()),
                       make_pair(s.value("TimesumSecondLayerLow",20).toDouble(),
                                 s.value("TimesumSecondLayerHigh",200).toDouble()));
  createHistList(set2DHist(name()));
  Log::add(Log::INFO,"Processor '" + name() +
           "' creates a detector picture of the first Hit on the detector created" +
           " from  Layers '" + _first + "' and '" + _second + "' of detector '" +
           _detector + "'. The signals from wich the frist hit is calculated have to be in the" +
           " range from '" + toString(_range.first) + "' ns to '" + toString(_range.second) +
           "' ns. The Timesum range of the first layer goes from '"+ toString(_tsrange.first.first) +
           "' to '" + toString(_tsrange.first.second) + "'. The Timesum range of the second layer goes from '" +
           toString(_tsrange.second.first) + "' to '" + toString(_tsrange.second.second) +
           "'. Condition is '" + _condition->name() + "'");
}

void pp164::process(const CASSEvent& evt, HistogramBackend &res)
{
  DetectorBackend &rawdet(
        HelperAcqirisDetectors::instance(_detector)->detector(evt));
  DelaylineDetector &det (dynamic_cast<DelaylineDetector&>(rawdet));
  const double f1 (det.layers()[_first].wireends()['1'].firstGood(_range));
  const double f2 (det.layers()[_first].wireends()['2'].firstGood(_range));
  const double s1 (det.layers()[_second].wireends()['1'].firstGood(_range));
  const double s2 (det.layers()[_second].wireends()['2'].firstGood(_range));
  const double mcp (det.mcp().firstGood(_range));
  const double tsf (f1 + f2 - 2.*mcp);
  const double tss (s1 + s2 - 2.*mcp);
  const double f (f1-f2);
  const double s (s1-s2);
  const bool csf = (_tsrange.first.first < tsf && tsf < _tsrange.first.second);
  const bool css = (_tsrange.second.first < tss && tss < _tsrange.second.second);

  Histogram2DFloat &result(dynamic_cast<Histogram2DFloat&>(res));

  result.clear();
  if (csf && css)
    result.fill(f,s);
}


















//----------------Nbr of rec. Hits --------------------------------------------
pp165::pp165(const name_t &name)
  : Processor(name)
{
  loadSettings(0);
}

void pp165::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("Processor");
  s.beginGroup(QString::fromStdString(name()));
  setupGeneral();
  if (!setupCondition())
    return;
  _detector = loadDelayDet(s,165,name());
  createHistList(tr1::shared_ptr<Histogram0DFloat>(new Histogram0DFloat()));
  Log::add(Log::INFO,"Processor '" + name() +
           "' outputs the number of reconstructed hits of detector '" + _detector +
           "'. Condition is '" + _condition->name() + "'");
}

void pp165::process(const CASSEvent& evt, HistogramBackend &res)
{
  DetectorBackend &rawdet(
        HelperAcqirisDetectors::instance(_detector)->detector(evt));
  DelaylineDetector &det (dynamic_cast<DelaylineDetector&>(rawdet));

  Histogram0DFloat &result(dynamic_cast<Histogram0DFloat&>(res));
  result.fill(det.hits().size());
}
















//----------------Detector Values----------------------------------------------
pp166::pp166(const name_t &name)
  : Processor(name)
{
  loadSettings(0);
}

void pp166::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("Processor");
  s.beginGroup(QString::fromStdString(name()));
  setupGeneral();
  if (!setupCondition())
    return;
  _detector = loadDelayDet(s,166,name());
  _first = static_cast<ACQIRIS::detectorHits>(s.value("XInput",0).toInt());
  _second = static_cast<ACQIRIS::detectorHits>(s.value("YInput",1).toInt());
  _third =  static_cast<ACQIRIS::detectorHits>(s.value("ConditionInput",2).toInt());
  _cond = make_pair(min(s.value("ConditionLow",-50000.).toFloat(),
                        s.value("ConditionHigh",50000.).toFloat()),
                    max(s.value("ConditionLow",-50000.).toFloat(),
                        s.value("ConditionHigh",50000.).toFloat()));
  createHistList(set2DHist(name()));
  Log::add(Log::INFO,"Processor '" + name() + "' histograms the Property '" +
           toString(_second) + "' vs. '" + toString(_first) +
           "' of the reconstructed detectorhits of detector '" + _detector +
           "'. It puts a condition from '" + toString(_cond.first) +
           "' to '" + toString(_cond.second) +  "' on Property '" +  toString(_third) +
           "'. Condition is '" + _condition->name() + "'");
}

void pp166::process(const CASSEvent& evt, HistogramBackend &res)
{
  DetectorBackend &rawdet(
        HelperAcqirisDetectors::instance(_detector)->detector(evt));
  DelaylineDetector &det (dynamic_cast<DelaylineDetector&>(rawdet));
  detectorHits_t::iterator it (det.hits().begin());
  detectorHits_t::iterator end (det.hits().end());
  Histogram2DFloat &result(dynamic_cast<Histogram2DFloat&>(res));
  result.clear();
  for (; it != end; ++it)
  {
    if (_cond.first < (*it)[_third] && (*it)[_third] < _cond.second)
      result.fill((*it)[_first],(*it)[_second]);
  }
}







//----------------Deadtime between consecutive Anode signals----------------------
pp167::pp167(const name_t &name)
  : Processor(name)
{
  loadSettings(0);
}

void pp167::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("Processor");
  s.beginGroup(QString::fromStdString(name()));
  setupGeneral();
  if (!setupCondition())
    return;
  _detector = loadDelayDet(s,167,name());
  _layer = loadLayer(s,_detector,"Layer",167,name());
  _signal = loadWireend(s,"Wireend",167,name());
  createHistList(set1DHist(name()));
  HelperAcqirisDetectors::instance(_detector)->loadSettings();
  Log::add(Log::INFO,"Processor '" + name() +
           "' creates a histogram of the deatime between two consecutive " +
           "Anode Signals of detctor '" + _detector +
           "'. Condition is '" + _condition->name() + "'");
}

void pp167::process(const CASSEvent& evt, HistogramBackend &res)
{
  DetectorBackend &rawdet(
      HelperAcqirisDetectors::instance(_detector)->detector(evt));
  DelaylineDetector &det(dynamic_cast<DelaylineDetector&>(rawdet));
  const SignalProducer::signals_t& anode(det.layers()[_layer].wireends()[_signal].output());

  Histogram1DFloat &result(dynamic_cast<Histogram1DFloat&>(res));

  result.clear();
  for (size_t i(1); i < anode.size(); ++i)
    result.fill(anode[i-1][ACQIRIS::time] - anode[i][ACQIRIS::time]);
}






//----------------PIPICO-------------------------------------------------------
pp220::pp220(const name_t &name)
  : Processor(name)
{
  loadSettings(0);
}

void pp220::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("Processor");
  s.beginGroup(QString::fromStdString(name()));
  _detector01 = s.value("FirstDetector","blubb").toString().toStdString();
  _detector02 = s.value("SecondDetector","blubb").toString().toStdString();
  setupGeneral();
  if (!setupCondition())
    return;
  createHistList(set2DHist(name()));
  HelperAcqirisDetectors::instance(_detector01)->loadSettings();
  HelperAcqirisDetectors::instance(_detector02)->loadSettings();
  Log::add(Log::INFO,"Processor '"+ name() +
      "' create a PIPICO Histogram of detectors '" + _detector01 +
      "' and '" + _detector02 + "'. Condition is '"+ _condition->name() + "'");
}

void pp220::process(const CASSEvent& evt, HistogramBackend &res)
{
  DetectorBackend &rawdet01(
      HelperAcqirisDetectors::instance(_detector01)->detector(evt));
  TofDetector &det01(dynamic_cast<TofDetector&>(rawdet01));
  DetectorBackend &rawdet02(
      HelperAcqirisDetectors::instance(_detector02)->detector(evt));
  TofDetector &det02(dynamic_cast<TofDetector&>(rawdet02));
  SignalProducer::signals_t::const_iterator it01(det01.mcp().output().begin());
  SignalProducer::signals_t::const_iterator end01(det01.mcp().output().end());
  SignalProducer::signals_t::const_iterator end02(det02.mcp().output().end());

  Histogram2DFloat &result(dynamic_cast<Histogram2DFloat&>(res));

  result.clear();
  for (; it01 != end01;++it01)
  {
    //if both detectors are the same, then the second iterator should start
    //i+1, otherwise we will just draw all hits vs. all hits
    SignalProducer::signals_t::const_iterator it02((_detector01==_detector02) ?
                                                     it01+1 :
                                                     det02.mcp().output().begin());
    for (; it02 != end02; ++it02)
      result.fill((*it01)[ACQIRIS::time],(*it02)[ACQIRIS::time]);
  }
}





//----------------Particle Value----------------------------------------------
pp250::pp250(const name_t &name)
  : Processor(name)
{
  loadSettings(0);
}

void pp250::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("Processor");
  s.beginGroup(QString::fromStdString(name()));
  setupGeneral();
  if (!setupCondition())
    return;
  _detector = loadDelayDet(s,250,name());
  _particle = loadParticle(s,_detector,250,name());
  _property = static_cast<ACQIRIS::particleHits>(s.value("Property",0).toInt());
  createHistList(set1DHist(name()));
  Log::add(Log::INFO,"Processor '" + name() + "' histograms the Property '" +
           toString(_property) + "' of the particle '" + _particle + "' of detector '" +
           _detector + "'. Condition is '" + _condition->name() + "'");
}

void pp250::process(const CASSEvent& evt, HistogramBackend &res)
{
  DetectorBackend &rawdet(
        HelperAcqirisDetectors::instance(_detector)->detector(evt));
  DelaylineDetector &det (dynamic_cast<DelaylineDetector&>(rawdet));
  Particle &particle(det.particles()[_particle]);
  particleHits_t::iterator it (particle.hits().begin());
  particleHits_t::iterator end (particle.hits().end());

  Histogram1DFloat &result(dynamic_cast<Histogram1DFloat&>(res));

  result.clear();
  while( it != end )
    result.fill((*it++)[_property]);
}






//----------------Particle Values----------------------------------------------
pp251::pp251(const name_t &name)
  : Processor(name)
{
  loadSettings(0);
}

void pp251::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("Processor");
  s.beginGroup(QString::fromStdString(name()));
  setupGeneral();
  if (!setupCondition())
    return;
  _detector = loadDelayDet(s,251,name());
  _particle = loadParticle(s,_detector,251,name());
  _property01 = static_cast<ACQIRIS::particleHits>(s.value("Property",0).toInt());
  _property02 = static_cast<ACQIRIS::particleHits>(s.value("Property",1).toInt());
  createHistList(set2DHist(name()));
  Log::add(Log::INFO,"Processor '" + name() + "' histograms the Property '" +
           toString(_property02) + "' vs. '" + toString(_property01) + "' of the particle '" +
           _particle + "' of detector '" + _detector + "'. Condition is '"+
           _condition->name() + "'");
}

void pp251::process(const CASSEvent& evt, HistogramBackend &res)
{
  DetectorBackend &rawdet(
        HelperAcqirisDetectors::instance(_detector)->detector(evt));
  DelaylineDetector &det(dynamic_cast<DelaylineDetector&>(rawdet));
  Particle &particle(det.particles()[_particle]);
  particleHits_t::iterator it(particle.hits().begin());
  particleHits_t::iterator end(particle.hits().end());

  Histogram2DFloat &result(dynamic_cast<Histogram2DFloat&>(res));

  result.clear();
  for (; it != end; ++it)
    result.fill((*it)[_property01],(*it)[_property02]);
}




//----------------Number of Particles---------------------------------------------
pp252::pp252(const name_t &name)
  : Processor(name)
{
  loadSettings(0);
}

void pp252::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("Processor");
  s.beginGroup(QString::fromStdString(name()));
  setupGeneral();
  if (!setupCondition())
    return;
  _detector = loadDelayDet(s,252,name());
  _particle = loadParticle(s,_detector,252,name());
  createHistList(tr1::shared_ptr<Histogram0DFloat>(new Histogram0DFloat()));
  Log::add(Log::INFO,"Processor '" + name() +
           + "' outputs how many particles were found for '" + _particle +
           + "' of detector '" + _detector + "'. Condition is '" + _condition->name() + "'");
}

void pp252::process(const CASSEvent& evt, HistogramBackend &res)
{
  DetectorBackend &rawdet(
        HelperAcqirisDetectors::instance(_detector)->detector(evt));
  DelaylineDetector &det (dynamic_cast<DelaylineDetector&>(rawdet));
  Particle& particle (det.particles()[_particle]);

  Histogram0DFloat &result(dynamic_cast<Histogram0DFloat&>(res));

  result.fill(particle.hits().size());
}
