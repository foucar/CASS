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

pp105::pp105(const name_t &name)
  : PostProcessor(name)
{
  loadSettings(0);
}

void pp105::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("PostProcessor");
  s.beginGroup(QString::fromStdString(name()));
  _detector = s.value("Detector","blubb").toString().toStdString();
  setupGeneral();
  if (!setupCondition())
    return;
  DetectorHelper::instance(_detector)->loadSettings();
  createHistList(
        tr1::shared_ptr<Histogram2DFloat>
        (new Histogram2DFloat
         (CommonData::instance(_detector)->columns,
          CommonData::instance(_detector)->rows)));
  Log::add(Log::INFO,"Postprocessor '" + name() +
           "' will display frame of detector '" + _detector +
           "'. It will use condition '" + _condition->name() +"'");
}

void pp105::process(const CASSEvent& evt, HistogramBackend &res)
{
  DetectorHelper::AdvDet_sptr det
      (DetectorHelper::instance(_detector)->detector(evt));
  const pixeldetector::frame_t& frame (det->frame().data);

  Histogram2DFloat &result(dynamic_cast<Histogram2DFloat&>(res));

  if (result.shape() != det->frame().shape())
  {
    throw invalid_argument("Postprocessor '" + name() +
                           "' incomming frame '" + toString(det->frame().columns) +
                           "x" + toString(det->frame().rows) + "'. Result '" +
                           toString(result.shape().first) + "x" +
                           toString(result.shape().second) + "'");
  }
  copy(frame.begin(), frame.end(),result.memory().begin());
  result.nbrOfFills() = 1;
}









// *** the maps of the pixeldetector ***

pp107::pp107(const name_t &name)
  : PostProcessor(name)
{
  loadSettings(0);
}

void pp107::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("PostProcessor");
  s.beginGroup(QString::fromStdString(name()));
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
    throw invalid_argument("p107::loadSettings(" +name() + "): MapType '"+ mapType +
                           "' does not exist");
  _mapLock = &CommonData::instance(_detector)->lock;
  createHistList(
        tr1::shared_ptr<Histogram2DFloat>
        (new Histogram2DFloat(CommonData::instance(_detector)->columns,
                              CommonData::instance(_detector)->rows)));
  Log::add(Log::INFO,"Postprocessor '" + name() + "' will display the '"+ mapType +
           "' map of detector '" + _detector + "'. It will use condition '" +
           _condition->name() +"'");
}

void pp107::process(const CASSEvent& /*evt*/, HistogramBackend &res)
{
  QReadLocker lock(_mapLock);
  Histogram2DFloat &result(dynamic_cast<Histogram2DFloat&>(res));

  if (result.shape().first != CommonData::instance(_detector)->columns ||
      result.shape().second != CommonData::instance(_detector)->rows)
  {
    throw invalid_argument("Postprocessor '" + name() +
                           "' The Map '" + toString(CommonData::instance(_detector)->columns) +
                           "x" + toString(CommonData::instance(_detector)->rows) + "'. Result '" +
                           toString(result.shape().first) + "x" +
                           toString(result.shape().second) + "'");
  }
  copy(_map->begin(), _map->end(),result.memory().begin());

  result.nbrOfFills() = 1;
}













// *** A Postprocessor that will display the coalesced photonhits of ccd detectors ***

pp144::pp144(const name_t &name)
  : PostProcessor(name)
{
  loadSettings(0);
}

void pp144::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("PostProcessor");
  s.beginGroup(QString::fromStdString(name()));
  _detector = s.value("Detector","blubb").toString().toStdString();
  setupGeneral();
  if (!setupCondition())
    return;
  createHistList(set2DHist(name()));
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
  Log::add(Log::INFO,"Postprocessor '" + name() +
           "' will add all hits of detector '" + _detector +
           "' to an image only when the spectral component is between '" +
           toString(_range.first) + "' and '" + toString(_range.second) +
           "', and the the split level is between '" + toString(_splitLevelRange.first) +
           "' and '" + toString(_splitLevelRange.second) +
           "'. It will fill the weight of the histograms with the " +
           "pixels z value" + toString(fillPixelvalueAsWeight) +
           "'. Condition is '" + _condition->name() + "'");
}

void pp144::process(const CASSEvent& evt, HistogramBackend &res)
{
  DetectorHelper::AdvDet_sptr det
      (DetectorHelper::instance(_detector)->detector(evt));
  AdvancedDetector::hits_t::const_iterator hit(det->hits().begin());

  Histogram2DFloat &result(dynamic_cast<Histogram2DFloat&>(res));

  result.clear();
  for (; hit != det->hits().end(); ++hit)
  {
    if (_splitLevelRange.first < hit->nbrPixels && hit->nbrPixels < _splitLevelRange.second)
      if (_range.first < hit->z && hit->z < _range.second)
        result.fill(hit->x,hit->y,_getZ(*hit));
  }
}










// *** A Postprocessor that will retrieve the number of coalesced hits ***

pp145::pp145(const name_t &name)
  : PostProcessor(name)
{
  loadSettings(0);
}

void pp145::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("PostProcessor");
  s.beginGroup(QString::fromStdString(name()));
  _detector = s.value("Detector","blubb").toString().toStdString();
  setupGeneral();
  if (!setupCondition())
    return;
  createHistList(tr1::shared_ptr<Histogram0DFloat>(new Histogram0DFloat()));
  DetectorHelper::instance(_detector)->loadSettings();
  Log::add(Log::INFO,"Postprocessor '" + name() +
           "' will retrieve the number of coalesced pixels (hits) of detector '"
           + _detector + "'. Condition is '" + _condition->name() + "'");
}

void pp145::process(const CASSEvent& evt, HistogramBackend &res)
{
  DetectorHelper::AdvDet_sptr det
      (DetectorHelper::instance(_detector)->detector(evt));
  const AdvancedDetector::hits_t& hits(det->hits());

  Histogram0DFloat &result(dynamic_cast<Histogram0DFloat&>(res));

  result.fill(hits.size());
}








// *** A Postprocessor that will output the split level of the coalesced pixels ***

pp146::pp146(const name_t &name)
  : PostProcessor(name)
{
  loadSettings(0);
}

void pp146::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("PostProcessor");
  s.beginGroup(QString::fromStdString(name()));
  _detector = s.value("Detector","blubb").toString().toStdString();
  setupGeneral();
  if (!setupCondition())
    return;
  createHistList(set1DHist(name()));
  DetectorHelper::instance(_detector)->loadSettings();
  Log::add(Log::INFO,"Postprocessor '" + name() +
           "' will retrieve the number of coalesced photonhits of detector '"
           + _detector + "'. Condition is '" + _condition->name() + "'");
}

void pp146::process(const CASSEvent& evt, HistogramBackend &res)
{
  DetectorHelper::AdvDet_sptr det
      (DetectorHelper::instance(_detector)->detector(evt));
  AdvancedDetector::hits_t::const_iterator hit(det->hits().begin());

  Histogram1DFloat &result(dynamic_cast<Histogram1DFloat&>(res));

  result.clear();
  for (; hit != det->hits().end(); ++hit)
    result.fill(hit->nbrPixels);
}








// *** will display the detected pixel of pixeldetectors as 2d image ***

pp148::pp148(const name_t &name)
  : PostProcessor(name)
{
  loadSettings(0);
}

void pp148::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("PostProcessor");
  s.beginGroup(QString::fromStdString(name()));
  _detector = s.value("Detector","blubb").toString().toStdString();
  setupGeneral();
  if (!setupCondition())
    return;
  createHistList(set2DHist(name()));
  _range = make_pair(s.value("SpectralLowerLimit",0.).toFloat(),
                     s.value("SpectralUpperLimit",0.).toFloat());
  bool fillPixelvalueAsWeight(s.value("PixelvalueAsWeight","true").toBool());
  if(fillPixelvalueAsWeight)
    _getZ = &getZValue<Pixel>;
  else
    _getZ = &getConstant<Pixel>;

  DetectorHelper::instance(_detector)->loadSettings();
  Log::add(Log::INFO,"Postprocessor '" + name() +
           "' will add all hits of detector '" + _detector +
           "' to an image only when the spectral component is between '" +
           toString(_range.first) + "' and '" + toString(_range.second) +
           "'. It will fill the weight of the histograms with the " +
           "pixels z value" + toString(fillPixelvalueAsWeight) +
           "'. Condition is '" + _condition->name() + "'");
}

void pp148::process(const CASSEvent& evt, HistogramBackend &res)
{
  DetectorHelper::AdvDet_sptr det
      (DetectorHelper::instance(_detector)->detector(evt));
  AdvancedDetector::pixels_t::const_iterator pixel(det->pixels().begin());

  Histogram2DFloat &result(dynamic_cast<Histogram2DFloat&>(res));

  result.clear();
  for (; pixel != det->pixels().end(); ++pixel)
    if (_range.first < pixel->z && pixel->z < _range.second)
      result.fill(pixel->x,pixel->y,_getZ(*pixel));
}










// *** A Postprocessor that will retrieve the number of detected pixels ***

pp149::pp149(const name_t &name)
  : PostProcessor(name)
{
  loadSettings(0);
}

void pp149::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("PostProcessor");
  s.beginGroup(QString::fromStdString(name()));
  _detector = s.value("Detector","blubb").toString().toStdString();
  setupGeneral();
  if (!setupCondition())
    return;
  createHistList(tr1::shared_ptr<Histogram0DFloat>(new Histogram0DFloat()));
  DetectorHelper::instance(_detector)->loadSettings();
  Log::add(Log::INFO,"Postprocessor '" + name() +
           "' will retrieve the number of coalesced pixels (hits) of detector '" +
           _detector + "'. Condition is '" + _condition->name() + "'");
}

void pp149::process(const CASSEvent& evt, HistogramBackend &res)
{
  DetectorHelper::AdvDet_sptr det
      (DetectorHelper::instance(_detector)->detector(evt));
  const AdvancedDetector::pixels_t& pixels(det->pixels());

  Histogram0DFloat &result(dynamic_cast<Histogram0DFloat&>(res));
  result.fill(pixels.size());
}










// *** postprocessor to correct a distorted pnCCD image ***

pp241::pp241(const name_t &name)
  : PostProcessor(name)
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
  if (_hist->result().dimension() != 2)
    throw std::runtime_error("PP type 241: Incomming is not a 2d histo");

  createHistList(_hist->result().copy_sptr());

  CASSSettings s;
  s.beginGroup("PostProcessor");
  s.beginGroup(QString::fromStdString(name()));
  _thresholdA = s.value("ThresholdQuadrantA",0).toFloat();
  _thresholdB = s.value("ThresholdQuadrantA",0).toFloat();
  _thresholdC = s.value("ThresholdQuadrantA",0).toFloat();
  _thresholdD = s.value("ThresholdQuadrantA",0).toFloat();

  _weightAdjectentRow = s.value("WeightAdjecentRow",0.75).toFloat();
  _weightSecondRow = s.value("WeightSecondNextRow",0.5).toFloat();
  _weightSum = 5 + 2*_weightAdjectentRow*5 + 2*_weightSecondRow*5;

  _minRow = s.value("MinimumRow",0).toUInt();
  _maxRow = s.value("MaximumRow",1024).toUInt();

  Log::add(Log::INFO,"Postprocessor '" + name() +
           "' corrects the distorted offset of image in '" + _hist->name() +
           ". Condition on PostProcessor '" + _condition->name() + "'");
}

void pp241::process(const CASSEvent& evt, HistogramBackend &res)
{
  const Histogram2DFloat &imagehist
      (dynamic_cast<const Histogram2DFloat&>(_hist->result(evt.id())));

  HistogramFloatBase &result(dynamic_cast<HistogramFloatBase&>(res));

  QReadLocker lock(&imagehist.lock);

  result.clear();

  const HistogramFloatBase::storage_t& image(imagehist.memory());
  HistogramFloatBase::storage_t& corimage(result.memory());

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
}








// *** set bad pixel of detector to a user selected value ***

pp242::pp242(const name_t &name)
  : PostProcessor(name)
{
  loadSettings(0);
}

void pp242::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("PostProcessor");
  s.beginGroup(QString::fromStdString(name()));
  _detector = s.value("Detector","blubb").toString().toStdString();
  _value = s.value("Value",0.f).toFloat();
  setupGeneral();
  if (!setupCondition())
    return;
  DetectorHelper::instance(_detector)->loadSettings();
  _mask = &CommonData::instance(_detector)->correctionMap;
  _maskLock = &CommonData::instance(_detector)->lock;
  createHistList(
        tr1::shared_ptr<Histogram2DFloat>
        (new Histogram2DFloat(CommonData::instance(_detector)->columns,
                              CommonData::instance(_detector)->rows)));
  Log::add(Log::INFO,"Postprocessor '" + name() + "' sets the masked pixels of detector '" +
           _detector + "' to '" +toString(_value) + "' It will use condition '"
           + _condition->name() +"'");
}

void pp242::process(const CASSEvent& evt, HistogramBackend &res)
{
  DetectorHelper::AdvDet_sptr det
      (DetectorHelper::instance(_detector)->detector(evt));
  const pixeldetector::frame_t& frame (det->frame().data);

  Histogram2DFloat &result(dynamic_cast<Histogram2DFloat&>(res));

  if (result.shape() != det->frame().shape())
  {
    throw invalid_argument("Postprocessor '" + name() +
                           "' incomming frame '" + toString(det->frame().columns) +
                           "x" + toString(det->frame().rows) + "'. Result '" +
                           toString(result.shape().first) + "x" +
                           toString(result.shape().second) + "'");
  }
  copy(frame.begin(), frame.end(), result.memory().begin());
  QReadLocker locker(_maskLock);
  HistogramFloatBase::storage_t::iterator pixel(result.memory().begin());
  pixeldetector::frame_t::const_iterator mask(_mask->begin());
  for (; pixel != result.memory().end(); ++pixel, ++mask)
  {
    if (qFuzzyCompare(*mask,0))
      *pixel = _value;
  }
  result.nbrOfFills() = 1;
}






