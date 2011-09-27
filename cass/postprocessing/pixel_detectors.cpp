// Copyright (C) 2011 Lutz Foucar

/** @file pixel_detectors.cpp contains postprocessor dealing with more advanced
 *                          pixel detectors.
 *
 * @author Lutz Foucar
 */

#include <cassert>

#include "pixel_detectors.h"

#include "advanced_pixeldetector.h"
#include "cass_settings.h"
#include "convenience_functions.h"
#include "histogram.h"
#include "common_data.h"

using namespace cass;
using namespace std;
using namespace pixeldetector;

namespace cass
{
namespace pixeldetector
{
/** get a constant 1
 *
 * @tparam the type of container for which the constant should be returned
 * @return 1
 * @param container unused
 *
 * @author Lutz Foucar
 */
template <class containerType>
frame_t::value_type getConstant(const containerType& /*container*/)
{
  return 1.0f;
}

/** retrieve the z value of the container
 *
 * @tparam the type of container for which the z value should be returned
 * @return the z value of the container
 * @param container the container that contains the z value
 *
 * @author Lutz Foucar
 */
template <class containerType>
frame_t::value_type getZValue(const containerType& container)
{
  return container.z;
}
}//end namespace pixeldetector
}//end namespace cass





// *** frame of the pixeldetector ***

pp105::pp105(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  :PostprocessorBackend(pp, key)
{
  loadSettings(0);
}

void pp105::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("PostProcessor");
  s.beginGroup(QString::fromStdString(_key));
  _detector = s.value("Detector","blubb").toString().toStdString();
  setupGeneral();
  if (!setupCondition())
    return;
  _result = new Histogram2DFloat(1024,1024);
  createHistList(2*cass::NbrOfWorkers);
  DetectorHelper::instance(_detector)->loadSettings();
  cout<<endl<<"Postprocessor '"<<_key
      <<"' will display frame of detector '"<<_detector
      <<"'. It will use condition '"<<_condition->key()<<"'"
      <<endl;
}

void pp105::process(const cass::CASSEvent& evt)
{
  DetectorHelper::AdvDet_sptr det
      (DetectorHelper::instance(_detector)->detector(evt));
   const pixeldetector::frame_t& frame (det->frame().data);
  _result->lock.lockForWrite();
  if (_result->axis()[HistogramBackend::xAxis].nbrBins() != det->frame().columns ||
      _result->axis()[HistogramBackend::yAxis].nbrBins() != det->frame().rows)
  {
    histogramList_t::iterator hist(_histList.begin());
    for (; hist != _histList.end(); ++hist)
    {
      Histogram2DFloat *histo(dynamic_cast<Histogram2DFloat*>(hist->second));
      histo->resize(det->frame().columns,0,det->frame().columns-1,
                    det->frame().rows,   0,det->frame().rows   -1);
    }
    PostProcessors::keyList_t dependands (_pp.find_dependant(_key));
    PostProcessors::keyList_t::iterator dependand (dependands.begin());
    for (; dependand != dependands.end(); ++dependand)
      _pp.getPostProcessor(*dependand).histogramsChanged(_result);
  }
  copy(frame.begin(),
       frame.end(),
       dynamic_cast<Histogram2DFloat*>(_result)->memory().begin());
  _result->nbrOfFills() = 1;
  _result->lock.unlock();
}











// *** histogram of frame data of the pixeldetector ***

pp106::pp106(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  :PostprocessorBackend(pp, key)
{
  loadSettings(0);
}

void pp106::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("PostProcessor");
  s.beginGroup(QString::fromStdString(_key));
  _detector = s.value("Detector","blubb").toString().toStdString();
  setupGeneral();
  if (!setupCondition())
    return;
  set1DHist(_result,_key);
  createHistList(2*cass::NbrOfWorkers);
  DetectorHelper::instance(_detector)->loadSettings();
  cout<<endl<<"Postprocessor '"<<_key
      <<"' will histogram the frame z values of detector '"<<_detector
      <<"'. It will use condition '"<<_condition->key()<<"'"
      <<endl;
}

void pp106::process(const cass::CASSEvent& evt)
{
  DetectorHelper::AdvDet_sptr det
      (DetectorHelper::instance(_detector)->detector(evt));
  pixeldetector::frame_t::const_iterator pixel(det->frame().data.begin());
  _result->lock.lockForWrite();
  _result->clear();
  for (; pixel != det->frame().data.end(); ++pixel)
    dynamic_cast<Histogram1DFloat*>(_result)->fill(*pixel);
  _result->nbrOfFills() = 1;
  _result->lock.unlock();
}












// *** the maps of the pixeldetector ***

pp107::pp107(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  :PostprocessorBackend(pp, key)
{
  loadSettings(0);
}

void pp107::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("PostProcessor");
  s.beginGroup(QString::fromStdString(_key));
  _detector = s.value("Detector","blubb").toString().toStdString();
  string mapType(s.value("MapType","offset").toString().toStdString());
  setupGeneral();
  if (!setupCondition())
    return;
  DetectorHelper::instance(_detector)->loadSettings();
  if (mapType == "offset")
    _map = &CommonData::instance(_detector)->offsetMap;
  else if (mapType == "noise")
    _map = &CommonData::instance(_detector)->noiseMap;
  else if (mapType == "gain_cte")
    _map = &CommonData::instance(_detector)->gain_cteMap;
  else if (mapType == "correction")
    _map = &CommonData::instance(_detector)->correctionMap;
  else
    throw invalid_argument("p107::loadSettings(" +_key + "): MapType '"+ mapType +
                           "' does not exist");
  _result = new Histogram2DFloat(CommonData::instance(_detector)->columns-1,
                                 CommonData::instance(_detector)->rows-1);
  createHistList(2*cass::NbrOfWorkers);
  cout<<endl<<"Postprocessor '"<<_key
      <<"' will display the '"<< mapType
      <<"' map of detector '"<<_detector
      <<"'. It will use condition '"<<_condition->key()<<"'"
      <<endl;
}

void pp107::process(const cass::CASSEvent&/* evt*/)
{
  QReadLocker lock(&CommonData::instance(_detector)->lock);
  _result->lock.lockForWrite();
  copy(_map->begin(),
       _map->end(),
       dynamic_cast<Histogram2DFloat*>(_result)->memory().begin());
  _result->nbrOfFills() = 1;
  _result->lock.unlock();
}









// *** will display the coalesced pixels (hits) spectrum ***

pp143::pp143(PostProcessors& pp, const PostProcessors::key_t &key)
    : PostprocessorBackend(pp, key)
{
  loadSettings(0);
}

void pp143::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("PostProcessor");
  s.beginGroup(QString::fromStdString(_key));
  _detector = s.value("Detector","blubb").toString().toStdString();
  setupGeneral();
  if (!setupCondition())
    return;
  _splitLevelRange = make_pair(s.value("SplitLevelLowerLimit",0).toUInt(),
                               s.value("SplitLevelUpperLimit",2).toUInt());
  set1DHist(_result,_key);
  createHistList(2*cass::NbrOfWorkers);
  DetectorHelper::instance(_detector)->loadSettings();
  cout<<endl<<"Postprocessor '"<<_key
      <<"' will display the spectrum of detector '"<<_detector
      <<"' only when the the split level is between '"<<_splitLevelRange.first
      <<"' and '"<<_splitLevelRange.second
      <<"'. Condition is '"<<_condition->key()<<"'"
      <<endl;
}

void pp143::process(const CASSEvent& evt)
{
  DetectorHelper::AdvDet_sptr det
      (DetectorHelper::instance(_detector)->detector(evt));
  AdvancedDetector::hits_t::const_iterator hit(det->hits().begin());
  _result->lock.lockForWrite();
  _result->clear();
  for (; hit != det->hits().end(); ++hit)
  {
    if (_splitLevelRange.first < hit->nbrPixels && hit->nbrPixels < _splitLevelRange.second)
      dynamic_cast<Histogram1DFloat*>(_result)->fill(hit->z);
  }
  _result->lock.unlock();
}











// *** A Postprocessor that will display the coalesced photonhits of ccd detectors ***

pp144::pp144(PostProcessors& pp, const cass::PostProcessors::key_t &key)
    : PostprocessorBackend(pp, key)
{
  loadSettings(0);
}

void pp144::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("PostProcessor");
  s.beginGroup(QString::fromStdString(_key));
  _detector = s.value("Detector","blubb").toString().toStdString();
  setupGeneral();
  if (!setupCondition())
    return;
  set2DHist(_result,_key);
  createHistList(2*cass::NbrOfWorkers);
  _range = make_pair(s.value("SpectralLowerLimit",0.).toFloat(),
                     s.value("SpectralUpperLimit",0.).toFloat());
  _splitLevelRange = make_pair(s.value("SplitLevelLowerLimit",0).toUInt(),
                               s.value("SplitLevelUpperLimit",2).toUInt());
  bool fillPixelvalueAsWeight(s.value("PixelvalueAsWeight","true").toBool());
  if(fillPixelvalueAsWeight)
    _getZ = &getZValue<Hit>;
  else
    _getZ = &getConstant<Hit>;

  DetectorHelper::instance(_detector)->loadSettings();
  cout<<endl<<"Postprocessor '"<<_key
      <<"' will add all hits of detector '"<<_detector
      <<"' to an image only when the spectral component is between '"<<_range.first
      <<"' and '"<<_range.second
      <<"', and the the split level is between '"<<_splitLevelRange.first
      <<"' and '"<<_splitLevelRange.second
      <<boolalpha<<"'. It will fill the weight of the histograms with the "
      <<"pixels z value"<<fillPixelvalueAsWeight
      <<"'. Condition is '"<<_condition->key()<<"'"
      <<endl;
}

void cass::pp144::process(const CASSEvent& evt)
{
  DetectorHelper::AdvDet_sptr det
      (DetectorHelper::instance(_detector)->detector(evt));
  AdvancedDetector::hits_t::const_iterator hit(det->hits().begin());
  _result->lock.lockForWrite();
  _result->clear();
  for (; hit != det->hits().end(); ++hit)
  {
    if (_splitLevelRange.first < hit->nbrPixels && hit->nbrPixels < _splitLevelRange.second)
      if (_range.first < hit->z && hit->z < _range.second)
        dynamic_cast<Histogram2DFloat*>(_result)->fill(hit->x,hit->y,_getZ(*hit));
  }
  _result->lock.unlock();
}










// *** A Postprocessor that will retrieve the number of coalesced hits ***

pp145::pp145(PostProcessors& pp, const cass::PostProcessors::key_t &key)
    : PostprocessorBackend(pp, key)
{
  loadSettings(0);
}

void pp145::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("PostProcessor");
  s.beginGroup(QString::fromStdString(_key));
  _detector = s.value("Detector","blubb").toString().toStdString();
  setupGeneral();
  if (!setupCondition())
    return;
  _result = new Histogram0DFloat();
  createHistList(2*cass::NbrOfWorkers);
  DetectorHelper::instance(_detector)->loadSettings();
  cout<<endl<<"Postprocessor '"<<_key
      <<"' will retrieve the number of coalesced pixels (hits) of detector '"<<_detector
      <<"'. Condition is '"<<_condition->key()<<"'"
      <<endl;
}

void pp145::process(const CASSEvent& evt)
{
  DetectorHelper::AdvDet_sptr det
      (DetectorHelper::instance(_detector)->detector(evt));
  const AdvancedDetector::hits_t& hits(det->hits());
  _result->lock.lockForWrite();
  dynamic_cast<Histogram0DFloat*>(_result)->fill(hits.size());
  _result->lock.unlock();
}








// *** A Postprocessor that will output the split level of the coalesced pixels ***

pp146::pp146(PostProcessors& pp, const cass::PostProcessors::key_t &key)
    : PostprocessorBackend(pp, key)
{
  loadSettings(0);
}

void pp146::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("PostProcessor");
  s.beginGroup(QString::fromStdString(_key));
  _detector = s.value("Detector","blubb").toString().toStdString();
  setupGeneral();
  if (!setupCondition())
    return;
  set1DHist(_result,_key);
  createHistList(2*cass::NbrOfWorkers);
  DetectorHelper::instance(_detector)->loadSettings();
  cout<<endl<<"Postprocessor '"<<_key
      <<"' will retrieve the number of coalesced photonhits of detector '"<<_detector
      <<"'. Condition is '"<<_condition->key()<<"'"
      <<endl;
}

void pp146::process(const CASSEvent& evt)
{
  DetectorHelper::AdvDet_sptr det
      (DetectorHelper::instance(_detector)->detector(evt));
  AdvancedDetector::hits_t::const_iterator hit(det->hits().begin());
  _result->lock.lockForWrite();
  _result->clear();
  for (; hit != det->hits().end(); ++hit)
    dynamic_cast<Histogram1DFloat*>(_result)->fill(hit->nbrPixels);
  _result->lock.unlock();
}









// *** will display the detected pixels spectrum ***

pp147::pp147(PostProcessors& pp, const PostProcessors::key_t &key)
    : PostprocessorBackend(pp, key)
{
  loadSettings(0);
}

void pp147::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("PostProcessor");
  s.beginGroup(QString::fromStdString(_key));
  _detector = s.value("Detector","blubb").toString().toStdString();
  setupGeneral();
  if (!setupCondition())
    return;
  set1DHist(_result,_key);
  createHistList(2*cass::NbrOfWorkers);
  DetectorHelper::instance(_detector)->loadSettings();
  cout<<endl<<"Postprocessor '"<<_key
      <<"' will histogram the detected pixels of detector '"<<_detector
      <<"'. Condition is '"<<_condition->key()<<"'"
      <<endl;
}

void pp147::process(const CASSEvent& evt)
{
  DetectorHelper::AdvDet_sptr det
      (DetectorHelper::instance(_detector)->detector(evt));
  AdvancedDetector::pixels_t::const_iterator pixel(det->pixels().begin());
  _result->lock.lockForWrite();
  _result->clear();
  for (; pixel != det->pixels().end(); ++pixel)
    dynamic_cast<Histogram1DFloat*>(_result)->fill(pixel->z);
  _result->lock.unlock();
}











// *** will display the detected pixel of pixeldetectors as 2d image ***

pp148::pp148(PostProcessors& pp, const cass::PostProcessors::key_t &key)
    : PostprocessorBackend(pp, key)
{
  loadSettings(0);
}

void pp148::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("PostProcessor");
  s.beginGroup(QString::fromStdString(_key));
  _detector = s.value("Detector","blubb").toString().toStdString();
  setupGeneral();
  if (!setupCondition())
    return;
  set2DHist(_result,_key);
  createHistList(2*cass::NbrOfWorkers);
  _range = make_pair(s.value("SpectralLowerLimit",0.).toFloat(),
                     s.value("SpectralUpperLimit",0.).toFloat());
  bool fillPixelvalueAsWeight(s.value("PixelvalueAsWeight","true").toBool());
  if(fillPixelvalueAsWeight)
    _getZ = &getZValue<Pixel>;
  else
    _getZ = &getConstant<Pixel>;

  DetectorHelper::instance(_detector)->loadSettings();
  cout<<endl<<"Postprocessor '"<<_key
      <<"' will add all hits of detector '"<<_detector
      <<"' to an image only when the spectral component is between '"<<_range.first
      <<"' and '"<<_range.second
      <<boolalpha<<"'. It will fill the weight of the histograms with the "
      <<"pixels z value"<<fillPixelvalueAsWeight
      <<"'. Condition is '"<<_condition->key()<<"'"
      <<endl;
}

void cass::pp148::process(const CASSEvent& evt)
{
  DetectorHelper::AdvDet_sptr det
      (DetectorHelper::instance(_detector)->detector(evt));
  AdvancedDetector::pixels_t::const_iterator pixel(det->pixels().begin());
  _result->lock.lockForWrite();
  _result->clear();
  for (; pixel != det->pixels().end(); ++pixel)
  {
    if (_range.first < pixel->z && pixel->z < _range.second)
      dynamic_cast<Histogram2DFloat*>(_result)->fill(pixel->x,pixel->y,_getZ(*pixel));
  }
  _result->lock.unlock();
}










// *** A Postprocessor that will retrieve the number of detected pixels ***

pp149::pp149(PostProcessors& pp, const cass::PostProcessors::key_t &key)
    : PostprocessorBackend(pp, key)
{
  loadSettings(0);
}

void pp149::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("PostProcessor");
  s.beginGroup(QString::fromStdString(_key));
  _detector = s.value("Detector","blubb").toString().toStdString();
  setupGeneral();
  if (!setupCondition())
    return;
  _result = new Histogram0DFloat();
  createHistList(2*cass::NbrOfWorkers);
  DetectorHelper::instance(_detector)->loadSettings();
  cout<<endl<<"Postprocessor '"<<_key
      <<"' will retrieve the number of coalesced pixels (hits) of detector '"<<_detector
      <<"'. Condition is '"<<_condition->key()<<"'"
      <<endl;
}

void pp149::process(const CASSEvent& evt)
{
  DetectorHelper::AdvDet_sptr det
      (DetectorHelper::instance(_detector)->detector(evt));
  const AdvancedDetector::pixels_t& pixels(det->pixels());
  _result->lock.lockForWrite();
  dynamic_cast<Histogram0DFloat*>(_result)->fill(pixels.size());
  _result->lock.unlock();
}




