// Copyright (C) 2011 Lutz Foucar

/**
 * @file hll_frame_processor.cpp contains hll correctionlike frame processor.
 *
 * @author Lutz Foucar
 */

#include <QtCore/QStringList>
#include <QtCore/QString>

#include "hll_frame_processor.h"

#include "cass_settings.h"
#include "advanced_pixeldetector.h"

using namespace cass;
using namespace pixeldetector;
using namespace commonmode;
using namespace std;

HLLProcessor::HLLProcessor()
{}

Frame& HLLProcessor::operator ()(Frame &frame)
{
  const CalculatorBase &commonModeCalculator(*_commonModeCalculator);
  QReadLocker lock(&_commondata->lock);
  frame_t::iterator pixel(frame.data.begin());
  frame_t::iterator pixelEnd(frame.data.end());
  frame_t::const_iterator offset(_commondata->offsetMap.begin());
  frame_t::const_iterator correction(_commondata->correctionMap.begin());
  size_t idx(0);
  float commonmodeLevel(0);
  const size_t width(commonModeCalculator.width());
  for (; pixel != pixelEnd; ++pixel, ++offset, ++correction, ++idx)
  {
    if(idx % width)
      commonmodeLevel = commonModeCalculator(pixel,idx);
    *pixel = (*pixel - *offset - commonmodeLevel) * *correction;
  }
  return frame;
}

void HLLProcessor::loadSettings(CASSSettings &s)
{
  string detectorname(s.group().split("/").back().toStdString());
  _commondata = CommonData::instance(detectorname);
  s.beginGroup("HLLProcessing");
  string commonmodetype (s.value("CommonModeCalculationType","none").toString().toStdString());
  _commonModeCalculator = CalculatorBase::instance(commonmodetype);
  _commonModeCalculator->loadSettings(s);
  s.endGroup();
}
