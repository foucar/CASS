//Copyright (C) 2010 Lutz Foucar

/**
 * @file acqiris_detectors.cpp file contains definition of postprocessors that
 *                             extract information of acqiris detectors.
 *
 * @author Lutz Foucar
 */

#include <stdexcept>
#include <cmath>
#include <algorithm>

#include <QtCore/QString>

#include "acqiris_detectors.h"
#include "acqiris_detectors_helper.h"
#include "histogram.h"
#include "cass_event.h"
#include "acqiris_device.h"
#include "cass.h"
#include "convenience_functions.h"
#include "cass_settings.h"



//----------------Nbr of Peaks MCP---------------------------------------------
cass::pp150::pp150(PostProcessors &pp, const PostProcessors::key_t &key)
  :cass::PostprocessorBackend(pp,key)
{
  loadSettings(0);
}

void cass::pp150::loadSettings(size_t)
{
  using namespace cass::ACQIRIS;
  CASSSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  _detector = settings.value("Detector","blubb").toString().toStdString();
  setupGeneral();
  if (!setupCondition())
    return;
  _result = new Histogram0DFloat();
  createHistList(2*cass::NbrOfWorkers);
  HelperAcqirisDetectors::instance(_detector)->loadSettings();
  std::cout <<std::endl<< "PostProcessor "<<_key
      <<": retrieves the nbr of mcp signals"
      <<" of detector "<<_detector
      <<". Condition is"<<_condition->key()
      <<std::endl;
}

void cass::pp150::process(const cass::CASSEvent &evt)
{
  using namespace cass::ACQIRIS;
  TofDetector *det
      (dynamic_cast<TofDetector*>(HelperAcqirisDetectors::instance(_detector)->detector(evt)));
  _result->lock.lockForWrite();
  dynamic_cast<Histogram0DFloat*>(_result)->fill(det->mcp().output().size());
  _result->lock.unlock();
}










//----------------MCP Hits (Tof)-----------------------------------------------
cass::pp151::pp151(PostProcessors &pp, const PostProcessors::key_t &key)
  :cass::PostprocessorBackend(pp,key)
{
  loadSettings(0);
}

void cass::pp151::loadSettings(size_t)
{
  using namespace cass::ACQIRIS;
  CASSSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  _detector = settings.value("Detector","blubb").toString().toStdString();
  setupGeneral();
  if (!setupCondition())
    return;
  set1DHist(_result,_key);
  createHistList(2*cass::NbrOfWorkers);
  HelperAcqirisDetectors::instance(_detector)->loadSettings();
  std::cout <<std::endl<< "PostProcessor "<<_key
      <<": it histograms times of the found mcp signals"
      <<" of detector "<<_detector
      <<". Condition is"<<_condition->key()
      <<std::endl;
}

void cass::pp151::process(const cass::CASSEvent &evt)
{
  using namespace cass::ACQIRIS;
  using namespace std;
  TofDetector *det
      (dynamic_cast<TofDetector*>(HelperAcqirisDetectors::instance(_detector)->detector(evt)));
  SignalProducer::signals_t::const_iterator it (det->mcp().output().begin());
  _result->clear();
  _result->lock.lockForWrite();
  for (; it != det->mcp().output().end(); ++it)
    dynamic_cast<Histogram1DFloat*>(_result)->fill((*it)["time"]);
  _result->lock.unlock();
}










//----------------MCP Fwhm vs. height------------------------------------------
cass::pp152::pp152(PostProcessors &pp, const PostProcessors::key_t &key)
  :cass::PostprocessorBackend(pp,key)
{
  loadSettings(0);
}

void cass::pp152::loadSettings(size_t)
{
  using namespace cass::ACQIRIS;
  CASSSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  _detector = settings.value("Detector","blubb").toString().toStdString();
  setupGeneral();
  if (!setupCondition())
    return;
  set2DHist(_result,_key);
  createHistList(2*cass::NbrOfWorkers);
  HelperAcqirisDetectors::instance(_detector)->loadSettings();
  std::cout <<std::endl<< "PostProcessor "<<_key
      <<": histograms the FWHM vs the height of the found mcp signals"
      <<" of detector "<<_detector
      <<". Condition is"<<_condition->key()
      <<std::endl;
}

void cass::pp152::process(const cass::CASSEvent &evt)
{
  using namespace cass::ACQIRIS;
  using namespace std;
  TofDetector *det
      (dynamic_cast<TofDetector*>(HelperAcqirisDetectors::instance(_detector)->detector(evt)));
  SignalProducer::signals_t::const_iterator it (det->mcp().output().begin());
  _result->clear();
  _result->lock.lockForWrite();
  for (;it != det->mcp().output().end(); ++it)
    dynamic_cast<Histogram2DFloat*>(_result)->fill((*it)["fwhm"],(*it)["height"]);
  _result->lock.unlock();
}












//----------------Nbr of Peaks Anode-------------------------------------------
cass::pp160::pp160(PostProcessors &pp, const PostProcessors::key_t &key)
  :cass::PostprocessorBackend(pp,key)
{
  loadSettings(0);
}

void cass::pp160::loadSettings(size_t)
{
  using namespace cass::ACQIRIS;
  CASSSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  _detector = settings.value("Detector","blubb").toString().toStdString();
  _layer = settings.value("Layer","U").toString()[0].toAscii();
  _signal = settings.value("Wireend","1").toString()[0].toAscii();
  _result = new Histogram0DFloat();
  setupGeneral();
  if (!setupCondition())
    return;
  if (_signal != '1' && _signal != '2')
    throw std::runtime_error("pp160::loadSettings(): Wireend is not set up correctly");
  if (_layer != 'U' && _layer != 'V' && _layer != 'W' &&
      _layer != 'X' && _layer != 'Y')
    throw std::runtime_error("pp160::loadSettings(): Layer is not set up correctly");
  createHistList(2*cass::NbrOfWorkers);
  HelperAcqirisDetectors::instance(_detector)->loadSettings();
  std::cout <<std::endl<< "PostProcessor \""<<_key
      <<"\": histograms the nbr of signals in"
      <<" detector "<<_detector
      <<" layer "<<_layer
      <<" wireend "<<_signal
      <<". Condition is"<<_condition->key()
      <<std::endl;
}

void cass::pp160::process(const cass::CASSEvent &evt)
{
  using namespace cass::ACQIRIS;
  DelaylineDetector *det
      (dynamic_cast<DelaylineDetector*>(HelperAcqirisDetectors::instance(_detector)->detector(evt)));
  _result->lock.lockForWrite();
  dynamic_cast<Histogram0DFloat*>(_result)->fill(det->layers()[_layer].wireends()[_signal].output().size());
  _result->lock.unlock();
}











//----------------FWHM vs. Height of Wireend Signals---------------------------
cass::pp161::pp161(PostProcessors &pp, const PostProcessors::key_t &key)
  :cass::PostprocessorBackend(pp,key)
{
  loadSettings(0);
}

void cass::pp161::loadSettings(size_t)
{
  using namespace cass::ACQIRIS;
  CASSSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  _detector = settings.value("Detector","blubb").toString().toStdString();
  _layer = settings.value("Layer","U").toString()[0].toAscii();
  _signal = settings.value("Wireend","1").toString()[0].toAscii();
  setupGeneral();
  if (!setupCondition())
    return;
  if (_signal != '1' && _signal != '2')
    throw std::runtime_error("pp161::loadSettings(): Wireend is not set up correctly");
  if (_layer != 'U' && _layer != 'V' && _layer != 'W' &&
      _layer != 'X' && _layer != 'Y')
    throw std::runtime_error("pp161::loadSettings(): Layer is not set up correctly");
  set2DHist(_result,_key);
  createHistList(2*cass::NbrOfWorkers);
  HelperAcqirisDetectors::instance(_detector)->loadSettings();
  std::cout <<std::endl<< "PostProcessor "<<_key
      <<": histograms the FWHM vs the height of layer "<<_layer
      << " wireend "<<_signal
      <<" of detector "<<_detector
      <<". Condition is"<<_condition->key()
      <<std::endl;
}

void cass::pp161::process(const cass::CASSEvent &evt)
{
  using namespace cass::ACQIRIS;
  using namespace std;
  DelaylineDetector *det
      (dynamic_cast<DelaylineDetector*>(HelperAcqirisDetectors::instance(_detector)->detector(evt)));
  SignalProducer::signals_t::const_iterator it (det->layers()[_layer].wireends()[_signal].output().begin());
  _result->clear();
  _result->lock.lockForWrite();
  for (; it != det->layers()[_layer].wireends()[_signal].output().end(); ++it)
    dynamic_cast<Histogram2DFloat*>(_result)->fill((*it)["fwhm"],(*it)["height"]);
  _result->lock.unlock();
}










//----------------Timesum for the layers---------------------------------------
cass::pp162::pp162(PostProcessors &pp, const PostProcessors::key_t &key)
  :cass::PostprocessorBackend(pp,key)
{
  loadSettings(0);
}

void cass::pp162::loadSettings(size_t)
{
  using namespace cass::ACQIRIS;
  using namespace std;
  CASSSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  _detector = settings.value("Detector","blubb").toString().toStdString();
  _layer = settings.value("Layer","U").toString()[0].toAscii();
  _range = make_pair(settings.value("TimeRangeLow",0).toDouble(),
                     settings.value("TimeRangeHigh",20000).toDouble());
  _result = new Histogram0DFloat();
  setupGeneral();
  if (!setupCondition())
    return;
  if (_layer != 'U' && _layer != 'V' && _layer != 'W' &&
      _layer != 'X' && _layer != 'Y')
    throw std::runtime_error("pp162::loadSettings(): Layer is not set up correctly");
  createHistList(2*cass::NbrOfWorkers);
  HelperAcqirisDetectors::instance(_detector)->loadSettings();
  cout <<endl<< "PostProcessor "<<_key
      <<" it histograms the timesum of layer "<<_layer
      <<" of detector "<<_detector
      <<". Condition is"<<_condition->key()
      <<endl;
}

void cass::pp162::process(const cass::CASSEvent &evt)
{
  using namespace cass::ACQIRIS;
  DelaylineDetector *det
      (dynamic_cast<DelaylineDetector*>(HelperAcqirisDetectors::instance(_detector)->detector(evt)));
  const double one (det->layers()[_layer].wireends()['1'].firstGood(_range));
  const double two (det->layers()[_layer].wireends()['2'].firstGood(_range));
  const double mcp (det->mcp().firstGood(_range));
  _result->lock.lockForWrite();
  dynamic_cast<Histogram0DFloat*>(_result)->fill( one + two - 2.*mcp);
  _result->lock.unlock();
}










//----------------Timesum vs Postition for the layers--------------------------
cass::pp163::pp163(PostProcessors &pp, const PostProcessors::key_t &key)
  :cass::PostprocessorBackend(pp,key)
{
  loadSettings(0);
}

void cass::pp163::loadSettings(size_t)
{
  using namespace cass::ACQIRIS;
  using namespace std;
  CASSSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  _detector = settings.value("Detector","blubb").toString().toStdString();
  _layer = settings.value("Layer","U").toString()[0].toAscii();
  _range = make_pair(settings.value("TimeRangeLow",0).toDouble(),
                     settings.value("TimeRangeHigh",20000).toDouble());
  setupGeneral();
  if (!setupCondition())
    return;
  if (_layer != 'U' && _layer != 'V' && _layer != 'W' &&
      _layer != 'X' && _layer != 'Y')
    throw std::runtime_error("pp163::loadSettings(): Layer is not set up correctly");
  set2DHist(_result,_key);
  createHistList(2*cass::NbrOfWorkers);
  HelperAcqirisDetectors::instance(_detector)->loadSettings();
  std::cout <<std::endl<< "PostProcessor "<<_key
      <<": histograms the timesum vs Postion on layer "<<_layer
      <<" of detector "<<_detector
      <<". Condition is"<<_condition->key()
      <<std::endl;
}

void cass::pp163::process(const cass::CASSEvent &evt)
{
  using namespace cass::ACQIRIS;
  using namespace std;
  DelaylineDetector *det
      (dynamic_cast<DelaylineDetector*>(HelperAcqirisDetectors::instance(_detector)->detector(evt)));
  const double one (det->layers()[_layer].wireends()['1'].firstGood(_range));
  const double two (det->layers()[_layer].wireends()['2'].firstGood(_range));
  const double mcp (det->mcp().firstGood(_range));
  const double timesum (one + two - 2.*mcp);
  const double position (one - two);
  _result->clear();
  _result->lock.lockForWrite();
  dynamic_cast<Histogram2DFloat*>(_result)->fill(position,timesum);
  _result->lock.unlock();
}











//----------------Detector First Hit-------------------------------------------
cass::pp164::pp164(PostProcessors &pp, const PostProcessors::key_t &key)
  :cass::PostprocessorBackend(pp,key)
{
  loadSettings(0);
}

void cass::pp164::loadSettings(size_t)
{
  using namespace cass::ACQIRIS;
  using namespace std;
  CASSSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  _detector = settings.value("Detector","blubb").toString().toStdString();
  _first = settings.value("FirstLayer","U").toString()[0].toAscii();
  _second = settings.value("SecondLayer","V").toString()[0].toAscii();
  _tsrange = make_pair(make_pair(settings.value("TimesumFirstLayerLow",20).toDouble(),
                                 settings.value("TimesumFirstLayerHigh",200).toDouble()),
                       make_pair(settings.value("TimesumSecondLayerLow",20).toDouble(),
                                 settings.value("TimesumSecondLayerHigh",200).toDouble()));
  setupGeneral();
  if (!setupCondition())
    return;
  if (_first != 'U' && _first != 'V' && _first != 'W' &&
      _first != 'X' && _first != 'Y')
    throw std::runtime_error("pp164::loadSettings(): First Layer is not set up correctly");
  if (_second != 'U' && _second != 'V' && _second != 'W' &&
      _second != 'X' && _second != 'Y')
    throw std::runtime_error("pp164::loadSettings(): Second Layer is not set up correctly");
  set2DHist(_result,_key);
  createHistList(2*cass::NbrOfWorkers);
  HelperAcqirisDetectors::instance(_detector)->loadSettings();
  std::cout <<std::endl<< "PostProcessor "<<_key
      <<": histograms a detector picture of the first Hit on the detector created"
      <<" from  Layers "<<_first
      <<" and "<<_second
      <<" of detector "<<_detector
      <<". Condition is "<<_condition->key()
      <<std::endl;
}

void cass::pp164::process(const cass::CASSEvent &evt)
{
  using namespace cass::ACQIRIS;
  using namespace std;
  DelaylineDetector *det
      (dynamic_cast<DelaylineDetector*>(HelperAcqirisDetectors::instance(_detector)->detector(evt)));
  const double f1 (det->layers()[_first].wireends()['1'].firstGood(_range));
  const double f2 (det->layers()[_first].wireends()['2'].firstGood(_range));
  const double s1 (det->layers()[_second].wireends()['1'].firstGood(_range));
  const double s2 (det->layers()[_second].wireends()['2'].firstGood(_range));
  const double mcp (det->mcp().firstGood(_range));
  const double tsf (f1 + f2 - 2.*mcp);
  const double tss (s1 + s2 - 2.*mcp);
  const double f (f1-f2);
  const double s (s1-s2);
  const bool csf = (_tsrange.first.first < tsf && tsf < _tsrange.first.second);
  const bool css = (_tsrange.second.first < tss && tss < _tsrange.second.second);
  _result->clear();
  _result->lock.lockForWrite();
  if (csf && css)
    dynamic_cast<Histogram2DFloat*>(_result)->fill(f,s);
  _result->lock.unlock();
}


















//----------------Nbr of rec. Hits --------------------------------------------
cass::pp165::pp165(PostProcessors &pp, const PostProcessors::key_t &key)
  :cass::PostprocessorBackend(pp,key)
{
  loadSettings(0);
}

void cass::pp165::loadSettings(size_t)
{
  using namespace cass::ACQIRIS;
  CASSSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  _detector = settings.value("Detector","blubb").toString().toStdString();
  _result = new Histogram0DFloat();
  setupGeneral();
  if (!setupCondition())
    return;
  createHistList(2*cass::NbrOfWorkers);
  HelperAcqirisDetectors::instance(_detector)->loadSettings();
  std::cout <<std::endl<< "PostProcessor "<<_key
      <<": outputs the number of reconstructed hits"
      <<" of detector "<<_detector
      <<". Condition is"<<_condition->key()
      <<std::endl;
}

void cass::pp165::process(const cass::CASSEvent &evt)
{
  using namespace cass::ACQIRIS;
  DelaylineDetector *det
      (dynamic_cast<DelaylineDetector*>(HelperAcqirisDetectors::instance(_detector)->detector(evt)));
  _result->lock.lockForWrite();
  dynamic_cast<Histogram0DFloat*>(_result)->fill(det->hits().size());
  _result->lock.unlock();
}
















//----------------Detector Values----------------------------------------------
cass::pp166::pp166(PostProcessors &pp, const PostProcessors::key_t &key)
  :cass::PostprocessorBackend(pp,key)

{
  loadSettings(0);
}

void cass::pp166::loadSettings(size_t)
{
  using namespace cass::ACQIRIS;
  using namespace std;
  CASSSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  _first = settings.value("XInput",'x').toString()[0].toAscii();
  _second = settings.value("YInput",'y').toString()[0].toAscii();
  _third =  settings.value("ConditionInput",'t').toString()[0].toAscii();
  _detector = settings.value("Detector","blubb").toString().toStdString();
  _condition =
      make_pair(min(settings.value("ConditionLow",-50000.).toFloat(),
                    settings.value("ConditionHigh",50000.).toFloat()),
                max(settings.value("ConditionLow",-50000.).toFloat(),
                    settings.value("ConditionHigh",50000.).toFloat()));
  setupGeneral();
  if (!setupCondition())
    return;
  set2DHist(_result,_key);
  createHistList(2*cass::NbrOfWorkers);
  HelperAcqirisDetectors::instance(_detector)->loadSettings();
  std::cout <<std::endl<< "PostProcessor "<<_key
      <<": histograms the Property "<<_second
      <<" vs. "<<_first
      <<" of the reconstructed detectorhits of detector "<<_detector
      <<" condition low "<<_condition.first
      <<" high "<<_condition.second
      <<" on Property "<< _third
      <<". Condition is"<<PostprocessorBackend::_condition->key()
      <<std::endl;
}

void cass::pp166::process(const cass::CASSEvent &evt)
{
  using namespace cass::ACQIRIS;
  using namespace std;
  DelaylineDetector *det
      (dynamic_cast<DelaylineDetector*>(HelperAcqirisDetectors::instance(_detector)->detector(evt)));
  detectorHits_t::iterator it (det->hits().begin());
  _result->clear();
  _result->lock.lockForWrite();
  for (; it != det->hits().end(); ++it)
  {
    if (_condition.first < (*it)[_third] && (*it)[_third] < _condition.second)
      dynamic_cast<Histogram2DFloat*>(_result)->fill((*it)[_first],(*it)[_second]);
  }
  _result->lock.unlock();
}









//----------------PIPICO-------------------------------------------------------
cass::pp220::pp220(PostProcessors &pp, const PostProcessors::key_t &key)
  :cass::PostprocessorBackend(pp,key)
{
  loadSettings(0);
}

void cass::pp220::loadSettings(size_t)
{
  using namespace cass::ACQIRIS;
  CASSSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  _detector01 = settings.value("FirstDetector","blubb").toString().toStdString();
  _detector02 = settings.value("SecondDetector","blubb").toString().toStdString();
  setupGeneral();
  if (!setupCondition())
    return;
  set2DHist(_result,_key);
  createHistList(2*cass::NbrOfWorkers);
  HelperAcqirisDetectors::instance(_detector01)->loadSettings();
  HelperAcqirisDetectors::instance(_detector02)->loadSettings();
  std::cout <<std::endl<< "PostProcessor "<<_key
      <<": create a PIPICO Histogram"
      <<" of detectors "<<_detector01
      <<" and "<<_detector02
      <<". Condition is"<<_condition->key()
      <<std::endl;
}

void cass::pp220::process(const cass::CASSEvent &evt)
{
  using namespace cass::ACQIRIS;
  using namespace std;
  TofDetector *det01
      (dynamic_cast<TofDetector*>(HelperAcqirisDetectors::instance(_detector01)->detector(evt)));
  TofDetector *det02
      (dynamic_cast<TofDetector*>(HelperAcqirisDetectors::instance(_detector02)->detector(evt)));
  SignalProducer::signals_t::const_iterator it01(det01->mcp().output().begin());
  _result->clear();
  _result->lock.lockForWrite();
  for (; it01 != det01->mcp().output().end();++it01)
  {
    //if both detectors are the same, then the second iterator should start
    //i+1, otherwise we will just draw all hits vs. all hits
    SignalProducer::signals_t::const_iterator it02((_detector01==_detector02) ?
                                                   it01+1 :
                                                   det02->mcp().output().begin());
    for (; it02 != det02->mcp().output().end(); ++it02)
      dynamic_cast<Histogram2DFloat*>(_result)->fill((*it01)["time"],(*it02)["time"]);
  }
  _result->lock.unlock();
}
