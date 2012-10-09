// Copyright (C) 2011,2012 Lutz Foucar

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
#include "log.h"

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
  Log::add(Log::INFO,"Postprocessor '" + _key +
           "' will display frame of detector '" + _detector +
           "'. It will use condition '" + _condition->key() +"'");
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
  copy(frame.begin(), frame.end(),
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
  Log::add(Log::INFO,"Postprocessor '" + _key +
           "' will histogram the frame z values of detector '" + _detector +
           "'. It will use condition '" + _condition->key() + "'");
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
  _mapLock = &CommonData::instance(_detector)->lock;
  _result = new Histogram2DFloat(CommonData::instance(_detector)->columns,
                                 CommonData::instance(_detector)->rows);
  /** @todo enable to resize the histogram, when the map does not have the default size */
  createHistList(2*cass::NbrOfWorkers,true);
  Log::add(Log::INFO,"Postprocessor '" + _key + "' will display the '"+ mapType +
           "' map of detector '" + _detector + "'. It will use condition '" +
           _condition->key() +"'");
}

void pp107::process(const cass::CASSEvent&/* evt*/)
{
  QReadLocker locker(_mapLock);
  _result->lock.lockForWrite();
  copy(_map->begin(), _map->end(),
       dynamic_cast<Histogram2DFloat*>(_result)->memory().begin());
  _result->nbrOfFills() = 1;
  _result->lock.unlock();
}




// *** sum up z values of all pixels ***

pp108::pp108(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  :PostprocessorBackend(pp, key)
{
  loadSettings(0);
}

void pp108::loadSettings(size_t)
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
  Log::add(Log::INFO,"Postprocessor '" + _key +
           "' will sum up the frame z values of detector '" + _detector +
           "'. It will use condition '" + _condition->key() + "'");
}

void pp108::process(const cass::CASSEvent& evt)
{
  DetectorHelper::AdvDet_sptr det
      (DetectorHelper::instance(_detector)->detector(evt));
  pixeldetector::frame_t::const_iterator pixel(det->frame().data.begin());
  _result->lock.lockForWrite();
  _result->clear();
  float sum(0);
  for (; pixel != det->frame().data.end(); ++pixel)
    sum += *pixel;
  *dynamic_cast<Histogram0DFloat*>(_result) = sum;
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
  Log::add(Log::INFO,"Postprocessor '" + _key +
           "' will add all hits of detector '" + _detector +
           "' to an image only when the spectral component is between '" +
           toString(_range.first) + "' and '" + toString(_range.second) +
           "', and the the split level is between '" + toString(_splitLevelRange.first) +
           "' and '" + toString(_splitLevelRange.second) +
           "'. It will fill the weight of the histograms with the " +
           "pixels z value" + toString(fillPixelvalueAsWeight) +
           "'. Condition is '" + _condition->key() + "'");
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
  Log::add(Log::INFO,"Postprocessor '" + _key +
           "' will retrieve the number of coalesced pixels (hits) of detector '"
           + _detector + "'. Condition is '" + _condition->key() + "'");
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
  Log::add(Log::INFO,"Postprocessor '" + _key +
           "' will retrieve the number of coalesced photonhits of detector '"
           + _detector + "'. Condition is '" + _condition->key() + "'");
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
  Log::add(Log::INFO,"Postprocessor '" + _key +
           "' will histogram the detected pixels of detector '" + _detector +
           "'. Condition is '" + _condition->key() + "'");
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
  Log::add(Log::INFO,"Postprocessor '" + _key +
           "' will add all hits of detector '" + _detector +
           "' to an image only when the spectral component is between '" +
           toString(_range.first) + "' and '" + toString(_range.second) +
           "'. It will fill the weight of the histograms with the " +
           "pixels z value" + toString(fillPixelvalueAsWeight) +
           "'. Condition is '" + _condition->key() + "'");
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
  Log::add(Log::INFO,"Postprocessor '" + _key +
           "' will retrieve the number of coalesced pixels (hits) of detector '" +
           _detector + "'. Condition is '" + _condition->key() + "'");
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






// *** will sum up the detected pixels ***

pp155::pp155(PostProcessors& pp, const PostProcessors::key_t &key)
    : PostprocessorBackend(pp, key)
{
  loadSettings(0);
}

void pp155::loadSettings(size_t)
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
  Log::add(Log::INFO,"Postprocessor '" + _key +
           "' will sum up the detected pixels of detector '" + _detector +
           "'. Condition is '" + _condition->key() + "'");
}

void pp155::process(const CASSEvent& evt)
{
  DetectorHelper::AdvDet_sptr det
      (DetectorHelper::instance(_detector)->detector(evt));
  AdvancedDetector::pixels_t::const_iterator pixel(det->pixels().begin());
  _result->lock.lockForWrite();
  _result->clear();
  float sum(0);
  for (; pixel != det->pixels().end(); ++pixel)
    sum += pixel->z;
  *dynamic_cast<Histogram0DFloat*>(_result) = sum;
  _result->lock.unlock();
}










// *** will sum up the coalesced pixels (hits)  ***

pp156::pp156(PostProcessors& pp, const PostProcessors::key_t &key)
    : PostprocessorBackend(pp, key)
{
  loadSettings(0);
}

void pp156::loadSettings(size_t)
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
  _result = new Histogram0DFloat();
  createHistList(2*cass::NbrOfWorkers);
  DetectorHelper::instance(_detector)->loadSettings();
  cout<<endl<<"Postprocessor '"<<_key
      <<"' will sum up the coalesced pixels of detector '"<<_detector
      <<"' only when the the split level is between '"<<_splitLevelRange.first
      <<"' and '"<<_splitLevelRange.second
      <<"'. Condition is '"<<_condition->key()<<"'"
      <<endl;
}

void pp156::process(const CASSEvent& evt)
{
  DetectorHelper::AdvDet_sptr det
      (DetectorHelper::instance(_detector)->detector(evt));
  AdvancedDetector::hits_t::const_iterator hit(det->hits().begin());
  _result->lock.lockForWrite();
  _result->clear();
  float sum(0);
  for (; hit != det->hits().end(); ++hit)
  {
    if (_splitLevelRange.first < hit->nbrPixels && hit->nbrPixels < _splitLevelRange.second)
      sum += hit->z;
    *dynamic_cast<Histogram0DFloat*>(_result) = sum;
  }
  _result->lock.unlock();
}






// *** postprocessor to correct a distorted pnCCD image ***

pp241::pp241(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key)
{
  loadSettings(0);
}

void pp241::loadSettings(size_t)
{
  setupGeneral();
  _hist = setupDependency("HistName");
  bool ret (setupCondition());
  if ( !(_hist && ret) )
    return;
  if (_hist->getHist(0).dimension() != 2)
    throw std::runtime_error("PP type 241: Incomming is not a 2d histo");
  setup(dynamic_cast<const Histogram2DFloat&>(_hist->getHist(0)));

  CASSSettings s;
  s.beginGroup("PostProcessor");
  s.beginGroup(QString::fromStdString(_key));
  _thresholdA = s.value("ThresholdQuadrantA",0).toFloat();
  _thresholdB = s.value("ThresholdQuadrantA",0).toFloat();
  _thresholdC = s.value("ThresholdQuadrantA",0).toFloat();
  _thresholdD = s.value("ThresholdQuadrantA",0).toFloat();

  _weightAdjectentRow = s.value("WeightAdjecentRow",0.75).toFloat();
  _weightSecondRow = s.value("WeightSecondNextRow",0.5).toFloat();
  _weightSum = 5 + 2*_weightAdjectentRow*5 + 2*_weightSecondRow*5;

  _minRow = s.value("MinimumRow",0).toUInt();
  _maxRow = s.value("MaximumRow",1024).toUInt();

  Log::add(Log::INFO,"Postprocessor '" + _key +
           "' corrects the distorted offset of image in '" + _hist->key() +
           ". Condition on PostProcessor '" + _condition->key() + "'");
}

void pp241::setup(const Histogram2DFloat &one)
{
  CASSSettings s;
  s.beginGroup("PostProcessor");
  s.beginGroup(QString::fromStdString(_key));
  const AxisProperty &xaxis(one.axis()[HistogramBackend::xAxis]);
  const AxisProperty &yaxis(one.axis()[HistogramBackend::xAxis]);
  _result = new Histogram2DFloat(xaxis.nbrBins(),
                                 xaxis.lowerLimit(),
                                 xaxis.upperLimit(),
                                 yaxis.nbrBins(),
                                 yaxis.lowerLimit(),
                                 yaxis.upperLimit(),
                                 xaxis.title(),
                                 yaxis.title());
  createHistList(2*cass::NbrOfWorkers);
}

void pp241::histogramsChanged(const HistogramBackend* in)
{
  QWriteLocker lock(&_histLock);
  //return when there is no incomming histogram
  if(!in)
    return;
  //return when the incomming histogram is not a direct dependant
  if (find(_dependencies.begin(),_dependencies.end(),in->key()) == _dependencies.end())
    return;
  //return when it is the wrong dimension
  if (in->dimension() != 2)
    return;
  setup(*dynamic_cast<const Histogram2DFloat*>(in));
  //notify all pp that depend on us that our histograms have changed
  PostProcessors::keyList_t dependands (_pp.find_dependant(_key));
  PostProcessors::keyList_t::iterator it (dependands.begin());
  for (; it != dependands.end(); ++it)
    _pp.getPostProcessor(*it).histogramsChanged(_result);
}

void pp241::process(const CASSEvent& evt)
{
  using namespace std;
  const Histogram2DFloat &imagehist
      (dynamic_cast<const Histogram2DFloat&>((*_hist)(evt)));
  imagehist.lock.lockForRead();
  _result->lock.lockForWrite();
  _result->clear();

  const HistogramFloatBase::storage_t& image(imagehist.memory());
  HistogramFloatBase::storage_t& corimage(dynamic_cast<HistogramFloatBase*>(_result)->memory());

  for(size_t row=0; row < 512; ++row)
  {
    //1st quadrant in cass (1st in hll)
    //determine the average offset at the left side pixels
    float averageOffsetA(0);
    for (size_t col=2; col<=6; ++col)
    {
      averageOffsetA += image[row*1024 + col];

      if (row >= 2 && row < 510)
      {
        averageOffsetA += image[(row-1)*1024 + col] * _weightAdjectentRow;
        averageOffsetA += image[(row+1)*1024 + col] * _weightAdjectentRow;
        averageOffsetA += image[(row-2)*1024 + col] * _weightSecondRow;
        averageOffsetA += image[(row+2)*1024 + col] * _weightSecondRow;
      }
    }
    averageOffsetA = (row >= 2 && row < 510) ? averageOffsetA / _weightSum : averageOffsetA / 5.f;
    //calc the slope
    const float slopeA = ((_minRow <= row) && (row <= _maxRow) && (averageOffsetA < _thresholdA)) ? (0.0055 * averageOffsetA - 0.0047) : 0.f;
    averageOffsetA = ((_minRow <= row) && (row <= _maxRow) && (averageOffsetA < _thresholdA)) ? averageOffsetA : 0.f;
    for(size_t col=0; col < 512; ++col)
    {
      corimage[row*1024 + col] = image[row*1024 + col] - (slopeA * col) - averageOffsetA;
    }

    //2nd quadrant in cass (4th in hll)
    //determine the average offset at the right side pixels
    float averageOffsetB(0);
    for (size_t col=1017; col<=1021; ++col)
    {
      averageOffsetB += image[row*1024 + col];

      if (row >= 2 && row < 510)
      {
        averageOffsetB += image[(row-1)*1024 + col] * _weightAdjectentRow;
        averageOffsetB += image[(row+1)*1024 + col] * _weightAdjectentRow;
        averageOffsetB += image[(row-2)*1024 + col] * _weightSecondRow;
        averageOffsetB += image[(row+2)*1024 + col] * _weightSecondRow;
      }
    }
    averageOffsetB = (row >= 2 && row < 510) ? averageOffsetB / _weightSum : averageOffsetB / 5.f;
    //calc the slope
    const float slopeB = ((_minRow <= row) && (row <= _maxRow) && (averageOffsetB < _thresholdB))? (0.0056 * averageOffsetB + 0.0007) : 0.f;
    averageOffsetB = ((_minRow <= row) && (row <= _maxRow) && (averageOffsetB < _thresholdB)) ? averageOffsetB : 0.f;
    for(size_t col=512; col < 1024; ++col)
    {
      corimage[row*1024 + col] = image[row*1024 + col] - (slopeB * (1023-col)) - averageOffsetB;
    }
  }
  for(size_t row = 512; row < 1024; ++row)
  {
    //3rd quadrant in cass (2nd in hll)
    //determine the average offset at the left side pixels
    float averageOffsetC(0);
    for (size_t col=2; col<=6; ++col)
    {
      averageOffsetC += image[row*1024 + col];

      if (row >= 514 && row < 1022)
      {
        averageOffsetC += image[(row-1)*1024 + col] * _weightAdjectentRow;
        averageOffsetC += image[(row+1)*1024 + col] * _weightAdjectentRow;
        averageOffsetC += image[(row-2)*1024 + col] * _weightSecondRow;
        averageOffsetC += image[(row+2)*1024 + col] * _weightSecondRow;
      }
    }
    averageOffsetC = (row >= 514 && row < 1022) ? averageOffsetC / _weightSum : averageOffsetC / 5.f;
    //calc the slope
    const float slopeC = ((_minRow <= row) && (row <= _maxRow) && (averageOffsetC < _thresholdC)) ? (0.0050 * averageOffsetC + 0.0078) : 0.f;
    averageOffsetC = ((_minRow <= row) && (row <= _maxRow) && (averageOffsetC < _thresholdC)) ? averageOffsetC : 0.f;
    for(size_t col=0; col < 512; ++col)
    {
      corimage[row*1024 + col] = image[row*1024 + col] - (slopeC * col) - averageOffsetC;
    }

    //4th quadrant in cass (3rd in hll)
    //determine the average offset at the right side pixels
    float averageOffsetD(0);
    for (size_t col=1017; col<=1021; ++col)
    {
      averageOffsetD += image[row*1024 + col];

      if (row >= 514 && row < 1022)
      {
        averageOffsetD += image[(row-1)*1024 + col] * _weightAdjectentRow;
        averageOffsetD += image[(row+1)*1024 + col] * _weightAdjectentRow;
        averageOffsetD += image[(row-2)*1024 + col] * _weightSecondRow;
        averageOffsetD += image[(row+2)*1024 + col] * _weightSecondRow;
      }
    }
    averageOffsetD = (row >= 514 && row < 1022) ? averageOffsetD / _weightSum : averageOffsetD / 5.f;
    //calc the slope
    const float slopeD = ((_minRow <= row) && (row <= _maxRow) && (averageOffsetD < _thresholdD)) ? (0.0049 * averageOffsetD + 0.0043) : 0.f;
    averageOffsetD = ((_minRow <= row) && (row <= _maxRow) && (averageOffsetD < _thresholdD)) ? averageOffsetD : 0.f;
    for(size_t col=512; col < 1024; ++col)
    {
      corimage[row*1024 + col] = image[row*1024 + col] - (slopeD * (1023-col)) - averageOffsetD;
    }
  }
  _result->lock.unlock();
  imagehist.lock.unlock();
}








// *** the maps of the pixeldetector ***

pp242::pp242(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  :PostprocessorBackend(pp, key)
{
  loadSettings(0);
}

void pp242::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("PostProcessor");
  s.beginGroup(QString::fromStdString(_key));
  _detector = s.value("Detector","blubb").toString().toStdString();
  _value = s.value("Value",0.f).toFloat();
  setupGeneral();
  if (!setupCondition())
    return;
  DetectorHelper::instance(_detector)->loadSettings();
  _mask = &CommonData::instance(_detector)->correctionMap;
  _maskLock = &CommonData::instance(_detector)->lock;
  _result = new Histogram2DFloat(CommonData::instance(_detector)->columns,
                                 CommonData::instance(_detector)->rows);
  createHistList(2*cass::NbrOfWorkers,true);
  Log::add(Log::INFO,"Postprocessor '" + _key + "' sets the masked pixels of detector '" +
           _detector + "' to '" +toString(_value) + "' It will use condition '"
           + _condition->key() +"'");
}

void pp242::process(const cass::CASSEvent& evt)
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
  HistogramFloatBase::storage_t &rframe(dynamic_cast<Histogram2DFloat*>(_result)->memory());
  copy(frame.begin(), frame.end(), rframe.begin());
  QReadLocker locker(_maskLock);
  HistogramFloatBase::storage_t::iterator pixel(rframe.begin());
  pixeldetector::frame_t::const_iterator mask(_mask->begin());
  for (; pixel != rframe.end(); ++pixel, ++mask)
  {
    if (qFuzzyCompare(*mask,0))
      *pixel = _value;
  }
  _result->nbrOfFills() = 1;
  _result->lock.unlock();
}






