// Copyright (C) 2011 Lutz Foucar

/** @file pixel_detectors.cpp contains postprocessor dealing with more advanced
 *                          pixel detectors.
 *
 * @author Lutz Foucar
 */

#include <cassert>

#include "pixel_detectors.h"

#include "advanced_pixeldetector.h"

using namespace cass;
using namespace std;
using namespace pixeldetectors;




// *** A Postprocessor that will display the coalesced photonhits spectrum ***

cass::pp143::pp143(PostProcessors& pp, const cass::PostProcessors::key_t &key)
    : PostprocessorBackend(pp, key)
{
  loadSettings(0);
}

void cass::pp143::loadSettings(size_t)
{
  using namespace std;
  CASSSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(QString::fromStdString(_key));
  _detector = settings.value("Detector","blubb").toString().toStdString();
  setupGeneral();
  if (!setupCondition())
    return;
  _splitLevelRange = make_pair(settings.value("SplitLevelLowerLimit",0).toUInt(),
                               settings.value("SplitLevelUpperLimit",2).toUInt());
  set1DHist(_result,_key);
  createHistList(2*cass::NbrOfWorkers);
  HelperPixelDetectors::instance(_detector)->loadSettings();
  cout<<endl<<"Postprocessor '"<<_key
      <<"' will display the spectrum of detector '"<<_detector
      <<"' only when the the split level is between '"<<_splitLevelRange.first
      <<"' and '"<<_splitLevelRange.second
      <<"'. Condition is '"<<_condition->key()<<"'"
      <<endl;
}

void cass::pp143::process(const CASSEvent& evt)
{
  HelperPixelDetectors::PixDetContainer_sptr det
      (HelperPixelDetectors::instance(_detector)->detector(evt));
  PixelDetectorContainer::hitlist_t::const_iterator hit(det->hits().begin());
  _result->lock.lockForWrite();
  _result->clear();
  for (; hit != det->hits().end(); ++hit)
  {
    if (_splitLevelRange.first < hit->nbrPixels() && hit->nbrPixels() < _splitLevelRange.second)
      dynamic_cast<Histogram1DFloat*>(_result)->fill(hit->z());
  }
  _result->lock.unlock();
}











// *** A Postprocessor that will display the coalesced photonhits of ccd detectors ***

cass::pp144::pp144(PostProcessors& pp, const cass::PostProcessors::key_t &key)
    : PostprocessorBackend(pp, key)
{
  loadSettings(0);
}

void cass::pp144::loadSettings(size_t)
{
  using namespace std;
  CASSSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(QString::fromStdString(_key));
  _detector = settings.value("Detector","blubb").toString().toStdString();
  setupGeneral();
  if (!setupCondition())
    return;
  set2DHist(_result,_key);
  createHistList(2*cass::NbrOfWorkers);
  _range = make_pair(settings.value("SpectralLowerLimit",0.).toFloat(),
                     settings.value("SpectralUpperLimit",0.).toFloat());
  _splitLevelRange = make_pair(settings.value("SplitLevelLowerLimit",0).toUInt(),
                               settings.value("SplitLevelUpperLimit",2).toUInt());
  HelperPixelDetectors::instance(_detector)->loadSettings();
  cout<<endl<<"Postprocessor '"<<_key
      <<"' will display ccd image of detector '"<<_detector
      <<"' only when the spectral component is between '"<<_range.first
      <<"' and '"<<_range.second
      <<"', and the the split level is between '"<<_splitLevelRange.first
      <<"' and '"<<_splitLevelRange.second
      <<"'. Condition is '"<<_condition->key()<<"'"
      <<endl;
}

void cass::pp144::process(const CASSEvent& evt)
{
  HelperPixelDetectors::PixDetContainer_sptr det
      (HelperPixelDetectors::instance(_detector)->detector(evt));
  PixelDetectorContainer::hitlist_t::const_iterator hit(det->hits().begin());
  _result->lock.lockForWrite();
  _result->clear();
  for (; hit != det->hits().end(); ++hit)
  {
    if (_splitLevelRange.first < hit->nbrPixels() && hit->nbrPixels() < _splitLevelRange.second)
      if (_range.first < hit->z() && hit->z() < _range.second)
        dynamic_cast<Histogram2DFloat*>(_result)->fill(hit->x(),hit->y(),hit->z());
  }
  _result->lock.unlock();
}










// *** A Postprocessor that will retrieve the number of coalesced hits ***

cass::pp145::pp145(PostProcessors& pp, const cass::PostProcessors::key_t &key)
    : PostprocessorBackend(pp, key)
{
  loadSettings(0);
}

void cass::pp145::loadSettings(size_t)
{
  using namespace std;
  CASSSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(QString::fromStdString(_key));
  _detector = settings.value("Detector","blubb").toString().toStdString();
  setupGeneral();
  if (!setupCondition())
    return;
  _result = new Histogram0DFloat();
  createHistList(2*cass::NbrOfWorkers);
  HelperPixelDetectors::instance(_detector)->loadSettings();
  cout<<endl<<"Postprocessor '"<<_key
      <<"' will retrieve the number of coalesced photonhits of detector '"<<_detector
      <<"'. Condition is '"<<_condition->key()<<"'"
      <<endl;
}

void cass::pp145::process(const CASSEvent& evt)
{
  HelperPixelDetectors::PixDetContainer_sptr det
      (HelperPixelDetectors::instance(_detector)->detector(evt));
  const PixelDetectorContainer::hitlist_t& hits(det->hits());
  _result->lock.lockForWrite();
  dynamic_cast<Histogram0DFloat*>(_result)->fill(hits.size());
  _result->lock.unlock();
}








// *** A Postprocessor that will output the split level of the coalesced pixels ***

cass::pp146::pp146(PostProcessors& pp, const cass::PostProcessors::key_t &key)
    : PostprocessorBackend(pp, key)
{
  loadSettings(0);
}

void cass::pp146::loadSettings(size_t)
{
  using namespace std;
  CASSSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(QString::fromStdString(_key));
  _detector = settings.value("Detector","blubb").toString().toStdString();
  setupGeneral();
  if (!setupCondition())
    return;
  set1DHist(_result,_key);
  createHistList(2*cass::NbrOfWorkers);
  HelperPixelDetectors::instance(_detector)->loadSettings();
  cout<<endl<<"Postprocessor '"<<_key
      <<"' will retrieve the number of coalesced photonhits of detector '"<<_detector
      <<"'. Condition is '"<<_condition->key()<<"'"
      <<endl;
}

void cass::pp146::process(const CASSEvent& evt)
{
  HelperPixelDetectors::PixDetContainer_sptr det
      (HelperPixelDetectors::instance(_detector)->detector(evt));
  PixelDetectorContainer::hitlist_t::const_iterator hit(det->hits().begin());
  _result->lock.lockForWrite();
  _result->clear();
  for (; hit != det->hits().end(); ++hit)
  {
      dynamic_cast<Histogram1DFloat*>(_result)->fill(hit->nbrPixels());
  }
  _result->lock.unlock();
}

