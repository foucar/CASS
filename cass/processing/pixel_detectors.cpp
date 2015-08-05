// Copyright (C) 2011,2012 Lutz Foucar

/** @file pixel_detectors.cpp contains processor dealing with more advanced
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
#include "cass_exceptions.hpp"

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
Detector::frame_t::value_type getConstant(const containerType& /*container*/)
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
Detector::frame_t::value_type getZValue(const containerType& container)
{
  return container.z;
}
}//end namespace pixeldetector
}//end namespace cass





// *** frame of the pixeldetector ***

pp105::pp105(const name_t &name)
  : Processor(name)
{
  loadSettings(0);
}

void pp105::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("Processor");
  s.beginGroup(QString::fromStdString(name()));
  _detector = s.value("Detector","UnamedPixeldetector").toString().toStdString();
  setupGeneral();
  if (!setupCondition())
    return;
  DetectorHelper::instance(_detector)->loadSettings();
  createHistList(
        tr1::shared_ptr<Histogram2DFloat>
        (new Histogram2DFloat
         (CommonData::instance(_detector)->columns,
          CommonData::instance(_detector)->rows)));
  Log::add(Log::INFO,"processor '" + name() +
           "' will display frame of detector '" + _detector +
           "'. It will use condition '" + _condition->name() +"'");
}

void pp105::process(const CASSEvent& evt, HistogramBackend &res)
{
  DetectorHelper::AdvDet_sptr det
      (DetectorHelper::instance(_detector)->detector(evt));
  const pixeldetector::Detector::frame_t& frame (det->frame().data);

  Histogram2DFloat &result(dynamic_cast<Histogram2DFloat&>(res));

  if (result.shape() != det->frame().shape())
  {
    throw invalid_argument("processor '" + name() +
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
  : Processor(name)
{
  loadSettings(0);
}

void pp107::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("Processor");
  s.beginGroup(QString::fromStdString(name()));
  _detector = s.value("Detector","UnnamedPixeldetector").toString().toStdString();
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
  Log::add(Log::INFO,"processor '" + name() + "' will display the '"+ mapType +
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
    throw invalid_argument("processor '" + name() +
                           "' The Map '" + toString(CommonData::instance(_detector)->columns) +
                           "x" + toString(CommonData::instance(_detector)->rows) + "'. Result '" +
                           toString(result.shape().first) + "x" +
                           toString(result.shape().second) + "'");
  }
  copy(_map->begin(), _map->end(),result.memory().begin());

  result.nbrOfFills() = 1;
}









// *** raw frame of the pixeldetector ***

pp109::pp109(const name_t &name)
  : Processor(name)
{
  loadSettings(0);
}

void pp109::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("Processor");
  s.beginGroup(QString::fromStdString(name()));
  setupGeneral();
  if (!setupCondition())
    return;
  _detector = s.value("CASSID",-1).toUInt();
  const Detector::shape_t shape(Frame::shapeFromName(name()));
  const int cols(s.value("nCols",static_cast<int>(shape.first)).toUInt());
  const int rows(s.value("nRows",static_cast<int>(shape.second)).toUInt());
  createHistList(
        tr1::shared_ptr<Histogram2DFloat>
        (new Histogram2DFloat(cols,rows)));
  Log::add(Log::INFO,"processor '" + name() +
           "' will display the raw frame of detector with CASSID '" +
           toString(_detector) + "' which has shape '" + toString(cols) + "'x'" +
           toString(rows) + "'. It will use condition '" + _condition->name() +"'");
}

void pp109::process(const CASSEvent& evt, HistogramBackend &res)
{
  CASSEvent::devices_t::const_iterator devIt(
        evt.devices().find(CASSEvent::PixelDetectors));
  if (devIt == evt.devices().end())
    throw logic_error("pp109::process(): Device " +
                      string("'PixelDetectors' does not exist in CASSEvent"));
  const Device &dev (dynamic_cast<const Device&>(*(devIt->second)));
  Device::detectors_t::const_iterator detIt(dev.dets().find(_detector));
  if (detIt == dev.dets().end())
    throw InvalidData("pp109::process(): Detector '" +
                           toString(_detector) + "' does not exist in Device " +
                           "'PixelDetectors' within the CASSEvent");
  const Detector &det(detIt->second);
  if (det.id() != evt.id())
    throw InvalidData("pp109::process(): The dataId '" +
                      toString(det.id()) + "' of detector '" + toString(_detector)+
                      "' is inconsistent with the eventId '" + toString(evt.id()) + "'");

  Histogram2DFloat &result(dynamic_cast<Histogram2DFloat&>(res));

  if (result.shape() != det.shape())
  {
    throw invalid_argument("processor '" + name() +
                           "' incomming frame '" + toString(det.shape().first) +
                           "x" + toString(det.shape().second) + "'. Result '" +
                           toString(result.shape().first) + "x" +
                           toString(result.shape().second) + "'");
  }
  copy(det.frame().begin(), det.frame().end(),result.memory().begin());
  result.nbrOfFills() = 1;
}










// *** A processor that will display the coalesced photonhits of ccd detectors ***

pp144::pp144(const name_t &name)
  : Processor(name)
{
  loadSettings(0);
}

void pp144::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("Processor");
  s.beginGroup(QString::fromStdString(name()));
  _detector = s.value("Detector","UnnamedPixeldetector").toString().toStdString();
  setupGeneral();
  if (!setupCondition())
    return;
  createHistList(set2DHist(name()));
  _range = make_pair(s.value("SpectralLowerLimit",0.).toFloat(),
                     s.value("SpectralUpperLimit",0.).toFloat());
  _splitLevelRange = make_pair(s.value("SplitLevelLowerLimit",0).toUInt(),
                               s.value("SplitLevelUpperLimit",2).toUInt());
  _baseValue = s.value("BaseValue",0).toFloat();
  bool fillPixelvalueAsWeight(s.value("PixelvalueAsWeight","true").toBool());
  if(fillPixelvalueAsWeight)
    _getZ = &getZValue<Hit>;
  else
    _getZ = &getConstant<Hit>;

  DetectorHelper::instance(_detector)->loadSettings();
  Log::add(Log::INFO,"processor '" + name() +
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
  fill(result.memory().begin(),result.memory().end(),_baseValue);
  for (; hit != det->hits().end(); ++hit)
  {
    if (_splitLevelRange.first < hit->nbrPixels && hit->nbrPixels < _splitLevelRange.second)
      if (_range.first < hit->z && hit->z < _range.second)
        result.fill(hit->x,hit->y,_getZ(*hit));
  }
}










// *** A processor that will retrieve the number of coalesced hits ***

pp145::pp145(const name_t &name)
  : Processor(name)
{
  loadSettings(0);
}

void pp145::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("Processor");
  s.beginGroup(QString::fromStdString(name()));
  _detector = s.value("Detector","UnnamedPixeldetector").toString().toStdString();
  setupGeneral();
  if (!setupCondition())
    return;
  createHistList(tr1::shared_ptr<Histogram0DFloat>(new Histogram0DFloat()));
  DetectorHelper::instance(_detector)->loadSettings();
  Log::add(Log::INFO,"processor '" + name() +
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








// *** A processor that will output the split level of the coalesced pixels ***

pp146::pp146(const name_t &name)
  : Processor(name)
{
  loadSettings(0);
}

void pp146::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("Processor");
  s.beginGroup(QString::fromStdString(name()));
  _detector = s.value("Detector","UnnamedPixeldetector").toString().toStdString();
  setupGeneral();
  if (!setupCondition())
    return;
  createHistList(set1DHist(name()));
  DetectorHelper::instance(_detector)->loadSettings();
  Log::add(Log::INFO,"processor '" + name() +
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
  : Processor(name)
{
  loadSettings(0);
}

void pp148::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("Processor");
  s.beginGroup(QString::fromStdString(name()));
  _detector = s.value("Detector","UnnamedPixeldetector").toString().toStdString();
  setupGeneral();
  if (!setupCondition())
    return;
  createHistList(set2DHist(name()));
  _range = make_pair(s.value("SpectralLowerLimit",0.).toFloat(),
                     s.value("SpectralUpperLimit",0.).toFloat());
  _baseValue = s.value("BaseValue",0).toFloat();
  bool fillPixelvalueAsWeight(s.value("PixelvalueAsWeight","true").toBool());
  if(fillPixelvalueAsWeight)
    _getZ = &getZValue<Pixel>;
  else
    _getZ = &getConstant<Pixel>;

  DetectorHelper::instance(_detector)->loadSettings();
  Log::add(Log::INFO,"processor '" + name() +
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
  fill(result.memory().begin(),result.memory().end(),_baseValue);
  for (; pixel != det->pixels().end(); ++pixel)
    if (_range.first < pixel->z && pixel->z < _range.second)
      result.fill(pixel->x,pixel->y,_getZ(*pixel));
}










// *** A processor that will retrieve the number of detected pixels ***

pp149::pp149(const name_t &name)
  : Processor(name)
{
  loadSettings(0);
}

void pp149::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("Processor");
  s.beginGroup(QString::fromStdString(name()));
  _detector = s.value("Detector","UnnamedPixeldetector").toString().toStdString();
  setupGeneral();
  if (!setupCondition())
    return;
  createHistList(tr1::shared_ptr<Histogram0DFloat>(new Histogram0DFloat()));
  DetectorHelper::instance(_detector)->loadSettings();
  Log::add(Log::INFO,"processor '" + name() +
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










// *** processor to correct a distorted pnCCD image ***

pp241::pp241(const name_t &name)
  : Processor(name)
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
  s.beginGroup("Processor");
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

  Log::add(Log::INFO,"processor '" + name() +
           "' corrects the distorted offset of image in '" + _hist->name() +
           ". Condition on Processor '" + _condition->name() + "'");
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
  : Processor(name)
{
  loadSettings(0);
}

void pp242::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("Processor");
  s.beginGroup(QString::fromStdString(name()));
  _detector = s.value("Detector","UnnamedPixeldetector").toString().toStdString();
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
  Log::add(Log::INFO,"processor '" + name() + "' sets the masked pixels of detector '" +
           _detector + "' to '" +toString(_value) + "' It will use condition '"
           + _condition->name() +"'");
}

void pp242::process(const CASSEvent& evt, HistogramBackend &res)
{
  DetectorHelper::AdvDet_sptr det
      (DetectorHelper::instance(_detector)->detector(evt));
  const pixeldetector::Detector::frame_t& frame (det->frame().data);

  Histogram2DFloat &result(dynamic_cast<Histogram2DFloat&>(res));

  if (result.shape() != det->frame().shape())
  {
    throw invalid_argument("processor '" + name() +
                           "' incomming frame '" + toString(det->frame().columns) +
                           "x" + toString(det->frame().rows) + "'. Result '" +
                           toString(result.shape().first) + "x" +
                           toString(result.shape().second) + "'");
  }
  copy(frame.begin(), frame.end(), result.memory().begin());
  QReadLocker locker(_maskLock);
  HistogramFloatBase::storage_t::iterator pixel(result.memory().begin());
  pixeldetector::Detector::frame_t::const_iterator mask(_mask->begin());
  for (; pixel != result.memory().end(); ++pixel, ++mask)
  {
    if (qFuzzyCompare(*mask,0))
      *pixel = _value;
  }
  result.nbrOfFills() = 1;
}








// *** apply mask to image ***

pp243::pp243(const name_t &name)
  : Processor(name)
{
  loadSettings(0);
}

void pp243::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("Processor");
  s.beginGroup(QString::fromStdString(name()));
  _image = setupDependency("HistName");
  _mask = setupDependency("MaskName");
  _value = s.value("Value",0.f).toFloat();
  setupGeneral();
  if (!setupCondition())
    return;
  createHistList(_image->result().copy_sptr());
  Log::add(Log::INFO,"processor '" + name() + "' sets pixels of '" +
           _image->name() + "' that are masked in '" + _mask->name() + "' to '"
           + toString(_value) + "' It will use condition '"
           + _condition->name() +"'");
}

void pp243::process(const CASSEvent& evt, HistogramBackend &res)
{
  const Histogram2DFloat &image
      (dynamic_cast<const Histogram2DFloat&>(_image->result(evt.id())));
  const Histogram2DFloat &mask
      (dynamic_cast<const Histogram2DFloat&>(_mask->result(evt.id())));

  Histogram2DFloat &result(dynamic_cast<Histogram2DFloat&>(res));
  QReadLocker imagelock(&image.lock);
  QReadLocker masklock(&mask.lock);

  Histogram2DFloat::storage_t::iterator dest(result.memory().begin());
  Histogram2DFloat::storage_t::const_iterator src(image.memory().begin());
  Histogram2DFloat::storage_t::const_iterator maskIt(mask.memory().begin());
  for (; dest != result.memory().end(); ++dest, ++src, ++maskIt)
    *dest = qFuzzyIsNull(*maskIt) ? _value : *src;
  result.nbrOfFills() = 1;
}






// *** generate pixel histograms ***

pp244::pp244(const name_t &name)
  : Processor(name)
{
  loadSettings(0);
}

void pp244::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("Processor");
  s.beginGroup(QString::fromStdString(name()));
  _image = setupDependency("ImageName");
  setupGeneral();
  if (!setupCondition())
    return;

  _isPnCCD = s.value("IsPnCCD",false).toBool();

  if (_image->result().dimension() != 2)
    throw invalid_argument("pp244::loadSettings: '" + name() + "' input '" +
                           _image->name() + "' is not a 2d histogram");

  const Histogram2DFloat &image(dynamic_cast<const Histogram2DFloat&>(_image->result()));
  if (_isPnCCD && (image.shape().first != 1024 || image.shape().second != 1024))
    throw invalid_argument("pp244::loadSettings(): '" + name() +
                           "' should be a pnCCD, but cols '" +
                           toString(image.shape().first) + "' and rows '"
                           + toString(image.shape().second) +
                           "' don't indicate a pnCCD");
  const size_t nPixels =
      _isPnCCD ? 512 + 2048 : image.shape().first * image.shape().second;

  const size_t nbins(s.value("XNbrBins",1).toUInt());
  const float low(s.value("XLow",0).toFloat());
  const float up(s.value("XUp",0).toFloat());
  const string title(s.value("XTitle","x-axis").toString().toStdString());
  _weight = s.value("Weight",1).toFloat();
  _maskval = s.value("MaskValue",0).toFloat();

  createHistList(
        tr1::shared_ptr<Histogram2DFloat>
        (new Histogram2DFloat(nbins,low,up,nPixels,0,nPixels-1,title,"Pixel")));

  Log::add(Log::INFO,"processor '" + name() +
           "' generates histogram nbr Bins '" + toString(nbins) + "', low '" +
           toString(low) + "', up '" + toString(up) + "', title '" + title +
           "', for all pixels of '" + _image->name() + "'. Condition '" +
           _condition->name() +"'");
}

void pp244::process(const CASSEvent& evt, HistogramBackend &res)
{
  const Histogram2DFloat &image
      (dynamic_cast<const Histogram2DFloat&>(_image->result(evt.id())));

  Histogram2DFloat &result(dynamic_cast<Histogram2DFloat&>(res));
  result.clear();

  QReadLocker imagelock(&image.lock);


  const size_t cols(image.shape().first);
  const size_t rows(image.shape().second);
  const size_t nPixels(cols*rows);
  const AxisProperty &prop(result.axis()[Histogram2DFloat::xAxis]);
  const float up(prop.upperLimit());
  const float low(prop.lowerLimit());
  const size_t nBins(prop.nbrBins());
  for (size_t i=0; i<nPixels; ++i)
  {
     /** find where in the histogram the pixel value would be put
      *  only if its within the range of the values
      */
    const float pixval(image.memory()[i]);
    if (!qFuzzyCompare(pixval,_maskval) && low <= pixval && pixval < up)
    {
      const size_t col(i % cols);
      const size_t row(i / cols);

      const size_t bin(
            static_cast<size_t>(nBins * ((pixval - low) / (up - low))));

      /** get the right row for the pixel from the result and add 1 at the bin
       *  of the pixel value
       */
      size_t rowidx(i);
      if (_isPnCCD)
        rowidx = (row < 512) ? col : col + cols;
      Histogram2DFloat::storage_t::iterator rrow(result.memory().begin() + rowidx*nBins);
      rrow[bin] += _weight;

      /** if it is a pnCCD now add the row to see the cte */
      if (_isPnCCD)
      {
        size_t cterow = (row < 512) ? row : 1023 - row;
        Histogram2DFloat::storage_t::iterator cterowidx
            (result.memory().begin() + (cterow + 2048)*nBins);
        cterowidx[bin] += _weight;
      }
    }
  }
  result.nbrOfFills() = 1;
}
