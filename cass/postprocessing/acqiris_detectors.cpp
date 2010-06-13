//Copyright (C) 2010 Lutz Foucar

#include <stdexcept>
#include <cmath>
#include <algorithm>

#include <QtCore/QSettings>
#include <QtCore/QString>

#include "acqiris_detectors.h"
#include "acqiris_detectors_helper.h"
#include "histogram.h"
#include "cass_event.h"
#include "acqiris_device.h"
#include "com.h"
#include "cfd.h"
#include "delayline_detector_analyzer_simple.h"
#include "delayline_detector.h"
#include "tof_analyzer_simple.h"
#include "tof_detector.h"
#include "cass.h"


void cass::set1DHist(cass::HistogramBackend*& hist, PostProcessors::key_t key)
{
  //open the settings//
  QSettings param;
  param.beginGroup("PostProcessor");
  param.beginGroup(key.c_str());
  //create new histogram using the parameters//
  VERBOSEOUT(std::cerr << "Creating 1D histogram with"
             <<" XNbrBins:"<<param.value("XNbrBins",1).toUInt()
             <<" XLow:"<<param.value("XLow",0).toFloat()
             <<" XUp:"<<param.value("XUp",0).toFloat()
             <<std::endl);
  hist = new cass::Histogram1DFloat(param.value("XNbrBins",1).toUInt(),
                                    param.value("XLow",0).toFloat(),
                                    param.value("XUp",0).toFloat());
}

void cass::set2DHist(cass::HistogramBackend*& hist, PostProcessors::key_t key)
{
  //open the settings//
  QSettings param;
  param.beginGroup("PostProcessor");
  param.beginGroup(key.c_str());
  //create new histogram using the parameters//
  VERBOSEOUT(std::cerr << "Creating 2D histogram with"
             <<" XNbrBins:"<<param.value("XNbrBins",1).toUInt()
             <<" XLow:"<<param.value("XLow",0).toFloat()
             <<" XUp:"<<param.value("XUp",0).toFloat()
             <<" YNbrBins:"<<param.value("YNbrBins",1).toUInt()
             <<" YLow:"<<param.value("YLow",0).toFloat()
             <<" YUp:"<<param.value("YUp",0).toFloat()
             <<std::endl);
  hist = new cass::Histogram2DFloat(param.value("XNbrBins",1).toUInt(),
                                    param.value("XLow",0).toFloat(),
                                    param.value("XUp",0).toFloat(),
                                    param.value("YNbrBins",1).toUInt(),
                                    param.value("YLow",0).toFloat(),
                                    param.value("YUp",0).toFloat());
}










//----------------Nbr of Peaks MCP---------------------------------------------
cass::pp150::pp150(PostProcessors &pp, const PostProcessors::key_t &key)
  :cass::PostprocessorBackend(pp,key)
{
//  loadSettings(0);
}

void cass::pp150::loadSettings(size_t)
{
  using namespace cass::ACQIRIS;
  QSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  _detector = static_cast<Detectors>(settings.value("Detector",1).toUInt());
  _result = new Histogram0DFloat();
  createHistList(2*cass::NbrOfWorkers);
  HelperAcqirisDetectors::instance(_detector)->loadSettings();
  std::cout <<std::endl<< "PostProcessor "<<_key
      <<": retrieves the nbr of mcp signals"
      <<" of detector "<<_detector
      <<std::endl;
}

void cass::pp150::process(const cass::CASSEvent &evt)
{
  using namespace cass::ACQIRIS;
  TofDetector *det
      (dynamic_cast<TofDetector*>(HelperAcqirisDetectors::instance(_detector)->detector(evt)));
  _result->lock.lockForWrite();
  dynamic_cast<Histogram0DFloat*>(_result)->fill(det->mcp().peaks().size());
  _result->lock.unlock();
}










//----------------MCP Hits (Tof)-----------------------------------------------
cass::pp151::pp151(PostProcessors &pp, const PostProcessors::key_t &key)
  :cass::PostprocessorBackend(pp,key)
{
//  loadSettings(0);
}

void cass::pp151::loadSettings(size_t)
{
  using namespace cass::ACQIRIS;
  QSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  _detector = static_cast<Detectors>(settings.value("Detector",1).toUInt());
  set1DHist(_result,_key);
  createHistList(2*cass::NbrOfWorkers);
  HelperAcqirisDetectors::instance(_detector)->loadSettings();
  std::cout <<std::endl<< "PostProcessor "<<_key
      <<": it histograms times of the found mcp signals"
      <<" of detector "<<_detector
      <<std::endl;
}

void cass::pp151::process(const cass::CASSEvent &evt)
{
  using namespace cass::ACQIRIS;
  using namespace std;
  TofDetector *det
      (dynamic_cast<TofDetector*>(HelperAcqirisDetectors::instance(_detector)->detector(evt)));
  Signal::peaks_t::const_iterator it (det->mcp().peaks().begin());
  _result->clear();
  _result->lock.lockForWrite();
  for (; it != det->mcp().peaks().end(); ++it)
    dynamic_cast<Histogram1DFloat*>(_result)->fill(it->time());
  _result->lock.unlock();
}










//----------------MCP Fwhm vs. height------------------------------------------
cass::pp152::pp152(PostProcessors &pp, const PostProcessors::key_t &key)
  :cass::PostprocessorBackend(pp,key)
{
//  loadSettings(0);
}

void cass::pp152::loadSettings(size_t)
{
  using namespace cass::ACQIRIS;
  QSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  _detector = static_cast<Detectors>(settings.value("Detector",1).toUInt());
  set2DHist(_result,_key);
  createHistList(2*cass::NbrOfWorkers);
  HelperAcqirisDetectors::instance(_detector)->loadSettings();
  std::cout <<std::endl<< "PostProcessor "<<_key
      <<": histograms the FWHM vs the height of the found mcp signals"
      <<" of detector "<<_detector
      <<std::endl;
}

void cass::pp152::process(const cass::CASSEvent &evt)
{
  using namespace cass::ACQIRIS;
  using namespace std;
  TofDetector *det
      (dynamic_cast<TofDetector*>(HelperAcqirisDetectors::instance(_detector)->detector(evt)));
  Signal::peaks_t::const_iterator it (det->mcp().peaks().begin());
  _result->clear();
  _result->lock.lockForWrite();
  for (;it != det->mcp().peaks().end(); ++it)
    dynamic_cast<Histogram1DFloat*>(_result)->fill(it->fwhm(),it->height());
  _result->lock.unlock();
}












//----------------Nbr of Peaks Anode-------------------------------------------
cass::pp160::pp160(PostProcessors &pp, const PostProcessors::key_t &key)
  :cass::PostprocessorBackend(pp,key)
{
//  loadSettings(0);
}

void cass::pp160::loadSettings(size_t)
{
  using namespace cass::ACQIRIS;
  QSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  _detector = static_cast<Detectors>(settings.value("Detector",1).toUInt());
  _layer = settings.value("Layer",'U').toChar().toAscii();
  _signal = settings.value("Wireend",'1').toChar().toAscii();
  _result = new Histogram0DFloat();
  createHistList(2*cass::NbrOfWorkers);
  HelperAcqirisDetectors::instance(_detector)->loadSettings();
  std::cout <<std::endl<< "PostProcessor "<<_key
      <<": histograms the nbr of signals in"
      <<" detector "<<_detector
      <<" layer "<<_layer
      <<" wireend "<<_signal
      <<std::endl;
}

void cass::pp160::process(const cass::CASSEvent &evt)
{
  using namespace cass::ACQIRIS;
  DelaylineDetector *det
      (dynamic_cast<DelaylineDetector*>(HelperAcqirisDetectors::instance(_detector)->detector(evt)));
  _result->lock.lockForWrite();
  dynamic_cast<Histogram1DFloat*>(_result)->fill(det->layers()[_layer].wireend()[_signal].peaks().size());
  _result->lock.unlock();
}











//----------------FWHM vs. Height of Wireend Signals---------------------------
cass::pp161::pp161(PostProcessors &pp, const PostProcessors::key_t &key)
  :cass::PostprocessorBackend(pp,key)
{
//  loadSettings(0);
}

void cass::pp161::loadSettings(size_t)
{
  using namespace cass::ACQIRIS;
  QSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  _detector = static_cast<Detectors>(settings.value("Detector",1).toUInt());
  _layer = settings.value("Layer",'U').toChar().toAscii();
  _signal = settings.value("Wireend",'1').toChar().toAscii();
  set2DHist(_result,_key);
  createHistList(2*cass::NbrOfWorkers);
  HelperAcqirisDetectors::instance(_detector)->loadSettings();
  std::cout <<std::endl<< "PostProcessor "<<_key
      <<": histograms the FWHM vs the height of layer "<<_layer
      << " wireend "<<_signal
      <<" of detector "<<_detector
      <<std::endl;
}

void cass::pp161::process(const cass::CASSEvent &evt)
{
  using namespace cass::ACQIRIS;
  using namespace std;
  DelaylineDetector *det
      (dynamic_cast<DelaylineDetector*>(HelperAcqirisDetectors::instance(_detector)->detector(evt)));
  Signal::peaks_t::const_iterator it (det->layers()[_layer].wireend()[_signal].peaks().begin());
  _result->clear();
  _result->lock.lockForWrite();
  for (; it != det->layers()[_layer].wireend()[_signal].peaks().end(); ++it)
    dynamic_cast<Histogram1DFloat*>(_result)->fill(it->fwhm(),it->height());
  _result->lock.unlock();
}










//----------------Timesum for the layers---------------------------------------
cass::pp162::pp162(PostProcessors &pp, const PostProcessors::key_t &key)
  :cass::PostprocessorBackend(pp,key)
{
//  loadSettings(0);
}

void cass::pp162::loadSettings(size_t)
{
  using namespace cass::ACQIRIS;
  QSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  _detector = static_cast<Detectors>(settings.value("Detector",1).toUInt());
  _layer = settings.value("Layer",'U').toChar().toAscii();
  _result = new Histogram0DFloat();
  createHistList(2*cass::NbrOfWorkers);
  std::cout <<std::endl<< "PostProcessor "<<_key
      <<" it histograms the timesum of layer "<<_layer
      <<" of detector "<<_detector
      <<std::endl;
}

void cass::pp162::process(const cass::CASSEvent &evt)
{
  using namespace cass::ACQIRIS;
  //get right filled detector from the helper
  DelaylineDetector *det
      (dynamic_cast<DelaylineDetector*>(HelperAcqirisDetectors::instance(_detector)->detector(evt)));
  _result->lock.lockForWrite();
  dynamic_cast<Histogram1DFloat*>(_result)->fill(det->timesum(_layer));
  _result->lock.unlock();
}










//----------------Timesum vs Postition for the layers--------------------------
cass::pp163::pp163(PostProcessors &pp, const PostProcessors::key_t &key)
  :cass::PostprocessorBackend(pp,key)
{
//  loadSettings(0);
}

void cass::pp163::loadSettings(size_t)
{
  using namespace cass::ACQIRIS;
  QSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  _detector = static_cast<Detectors>(settings.value("Detector",1).toUInt());
  _layer = settings.value("Layer",'U').toChar().toAscii();
  set2DHist(_result,_key);
  createHistList(2*cass::NbrOfWorkers);
  HelperAcqirisDetectors::instance(_detector)->loadSettings();
  std::cout <<std::endl<< "PostProcessor "<<_key
      <<": histograms the timesum vs Postion on layer "<<_layer
      <<" of detector "<<_detector
      <<std::endl;
}

void cass::pp163::process(const cass::CASSEvent &evt)
{
  using namespace cass::ACQIRIS;
  using namespace std;
  //get right filled detector from the helper
  DelaylineDetector *det
      (dynamic_cast<DelaylineDetector*>(HelperAcqirisDetectors::instance(_detector)->detector(evt)));
  _result->clear();
  _result->lock.lockForWrite();
  dynamic_cast<Histogram1DFloat*>(_result)->fill(det->position(_layer),det->timesum(_layer));
  _result->lock.unlock();
}











//----------------Detector First Hit-------------------------------------------
cass::pp164::pp164(PostProcessors &pp, const PostProcessors::key_t &key)
  :cass::PostprocessorBackend(pp,key)
{
//  loadSettings(0);
}

void cass::pp164::loadSettings(size_t)
{
  using namespace cass::ACQIRIS;
  QSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  _detector = static_cast<Detectors>(settings.value("Detector",1).toUInt());
  _first = settings.value("Layer",'U').toChar().toAscii();
  _second = settings.value("Layer",'V').toChar().toAscii();
  set2DHist(_result,_key);
  createHistList(2*cass::NbrOfWorkers);
  HelperAcqirisDetectors::instance(_detector)->loadSettings();
  std::cout <<std::endl<< "PostProcessor "<<_key
      <<": histograms a detector picture of the first Hit on the detector created"
      <<" from  Layers "<<_first
      <<" and "<<_second
      <<" of detector "<<_detector
      <<std::endl;
}

void cass::pp164::process(const cass::CASSEvent &evt)
{
  using namespace cass::ACQIRIS;
  using namespace std;
  //get right filled detector from the helper
  DelaylineDetector *det =
      dynamic_cast<DelaylineDetector*>(HelperAcqirisDetectors::instance(_detector)->detector(evt));
  //get the requested layers//
  AnodeLayer &f = det->layers()[_first];
  AnodeLayer &s = det->layers()[_second];
  //get the timesums for the layers//
  const double tsf = det->timesum(_first);
  const double tss = det->timesum(_second);
  //check timesum//
  const bool csf = (f.tsLow() < tsf && tsf < f.tsHigh());
  const bool css = (s.tsLow() < tss && tss < s.tsHigh());
  //only fill when timesum is fullfilled
  _result->clear();
  _result->lock.lockForWrite();
  if (csf && css)
    dynamic_cast<Histogram1DFloat*>(_result)->fill(f.position(),s.position());
  _result->lock.unlock();
}


















//----------------Nbr of rec. Hits --------------------------------------------
cass::pp165::pp165(PostProcessors &pp, const PostProcessors::key_t &key)
  :cass::PostprocessorBackend(pp,key),
  _nbrHits(0)
{
  loadSettings(0);
}

cass::pp165::~pp165()
{
  _pp.histograms_delete(_key);
  _nbrHits=0;
}

void cass::pp165::loadSettings(size_t)
{
  using namespace cass::ACQIRIS;
  QSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  _detector = static_cast<Detectors>(settings.value("Detector",1).toUInt());
  _pp.histograms_delete(_key);
  _nbrHits = new Histogram0DFloat();
  _pp.histograms_replace(_key,_nbrHits);
  HelperAcqirisDetectors::instance(_detector)->loadSettings();
  std::cout <<std::endl<< "PostProcessor "<<_key
      <<": outputs the number of reconstructed hits"
      <<" of detector "<<_detector
      <<std::endl;
}

void cass::pp165::operator()(const cass::CASSEvent &evt)
{
  using namespace cass::ACQIRIS;
  //get right filled detector from the helper
  DelaylineDetector *det
      (dynamic_cast<DelaylineDetector*>(HelperAcqirisDetectors::instance(_detector)->detector(evt)));
  _nbrHits->lock.lockForWrite();
  _nbrHits->fill(det->hits().size());
  _nbrHits->lock.unlock();
}
















//----------------Detector Values----------------------------------------------
cass::pp166::pp166(PostProcessors &pp, const PostProcessors::key_t &key)
  :cass::PostprocessorBackend(pp,key),
  _hist(0)

{
  loadSettings(0);
}

cass::pp166::~pp166()
{
  _pp.histograms_delete(_key);
  _hist=0;
}

void cass::pp166::loadSettings(size_t)
{
  using namespace cass::ACQIRIS;
  using namespace std;
  QSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  _first = settings.value("XInput",'x').toChar().toAscii();
  _second = settings.value("YInput",'y').toChar().toAscii();
  _condition =
      make_pair(min(settings.value("ConditionLow",-50000.).toFloat(),
                    settings.value("ConditionHigh",50000.).toFloat()),
                max(settings.value("ConditionLow",-50000.).toFloat(),
                    settings.value("ConditionHigh",50000.).toFloat()));
  set2DHist(_hist,_key);
  _pp.histograms_replace(_key,_hist);
  HelperAcqirisDetectors::instance(_detector)->loadSettings();
  if (_first == 'x' && _second == 'y')
    _third = 't';
  else if (_first == 'y' && _second == 'x')
    _third = 't';
  else if (_first == 'x' && _second == 't')
    _third = 'y';
  else if (_first == 't' && _second == 'x')
    _third = 'y';
  else if (_first == 'y' && _second == 't')
    _third = 'x';
  else if (_first == 't' && _second == 'y')
    _third = 'x';
  std::cout <<std::endl<< "PostProcessor "<<_key
      <<": histograms the Property "<<_second
      <<" vs. "<<_first
      <<" of the reconstructed detectorhits of detector "<<_detector
      <<" condition low "<<_condition.first
      <<" high "<<_condition.second
      <<" on Property "<< _third
      <<std::endl;
}

void cass::pp166::operator()(const cass::CASSEvent &evt)
{
  using namespace cass::ACQIRIS;
  using namespace std;
  DelaylineDetector *det
      (dynamic_cast<DelaylineDetector*>(HelperAcqirisDetectors::instance(_detector)->detector(evt)));
  DelaylineDetector::dethits_t::iterator it (det->hits().begin());
  _hist->lock.lockForWrite();
  fill(_hist->memory().begin(),_hist->memory().end(),0.f);
  for (; it != det->hits().end(); ++it)
  {
    if (_condition.first < it->values()[_third] && it->values()[_third] < _condition.second)
      _hist->fill(it->values()[_first],it->values()[_second]);
  }
  _hist->lock.unlock();
}









//----------------PIPICO-------------------------------------------------------
cass::pp220::pp220(PostProcessors &pp, const PostProcessors::key_t &key)
  :cass::PostprocessorBackend(pp,key),
  _pipico(0)
{
  loadSettings(0);
}

cass::pp220::~pp220()
{
  _pp.histograms_delete(_key);
  _pipico=0;
}

void cass::pp220::loadSettings(size_t)
{
  using namespace cass::ACQIRIS;
  QSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  _detector01 = static_cast<Detectors>(settings.value("FirstDetector",1).toUInt());
  _detector02 = static_cast<Detectors>(settings.value("SecondDetector",1).toUInt());
  set2DHist(_pipico,_key);
  _pp.histograms_replace(_key,_pipico);
  HelperAcqirisDetectors::instance(_detector01)->loadSettings();
  HelperAcqirisDetectors::instance(_detector02)->loadSettings();
  std::cout <<std::endl<< "PostProcessor "<<_key
      <<": create a PIPICO Histogram"
      <<" of detectors "<<_detector01
      <<" and "<<_detector02
      <<std::endl;
}

void cass::pp220::operator()(const cass::CASSEvent &evt)
{
  using namespace cass::ACQIRIS;
  using namespace std;
  //get first detector from the helper
  TofDetector *det01 =
      dynamic_cast<TofDetector*>(HelperAcqirisDetectors::instance(_detector01)->detector(evt));
  //get second detector from the helper
  TofDetector *det02 =
      dynamic_cast<TofDetector*>(HelperAcqirisDetectors::instance(_detector02)->detector(evt));
  //get iterator of the peaks in the first detector//
  Signal::peaks_t::const_iterator it01(det01->mcp().peaks().begin());
  //draw all found hits vs another//
  _pipico->lock.lockForWrite();
  fill(_pipico->memory().begin(),_pipico->memory().end(),0.f);
  for (; it01 != det01->mcp().peaks().end();++it01)
  {
    //if both detectors are the same, then the second iterator should start
    //i+1, otherwise we will just draw all hits vs. all hits
    Signal::peaks_t::const_iterator it02((_detector01==_detector02) ?
                                         it01+1 :
                                         det02->mcp().peaks().begin());
    for (; it02 != det02->mcp().peaks().end(); ++it02)
    {
      _pipico->fill(it01->time(),it02->time());
    }
  }
  _pipico->lock.unlock();
}
