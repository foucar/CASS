// Copyright (C) 2011 Lutz Foucar

/**
 * @file hll_frame_processor.cpp contains hll correctionlike frame processor.
 *
 * @author Lutz Foucar
 */

#include "hll_frame_processor.h"

#include "cass_settings.h"
#include "advanced_pixeldetector.h"

using namespace cass;
using namespace pixeldetector;
using namespace std;

HLLProcessor::HLLProcessor()
{}

Frame& HLLProcessor::operator ()(Frame &frame)
{
  const CommonModeCalulator &commonMode(*_commonModeCalc);
  QReadLocker lock(&_commondata->lock);
  frame_t::iterator pixel(frame.data.begin());
  frame_t::const_iterator offset(_commondata->offsetMap.begin());
  frame_t::const_iterator correction(_commondata->correctionMap.begin());
  size_t idx(0);
  float commonmodeLevel(0);
  for (; pixel != frame.data.end(); ++pixel, ++correction, ++idx)
  {
    if(idx % commonMode.width())
      commonmodeLevel = commonMode(pixel);
    *pixel = (*pixel - *offset - commonmodeLevel) * *correction;
  }
  return frame;
}

void HLLProcessor::loadSettings(CASSSettings &s)
{
  string detectorname(s.group().split("/").back().toStdString());
  _commondata = CommonData::instance(detectorname);
  s.beginGroup("HLLProcessing");
  string commonmodetype = s.value("CommonModeCalculationType","none").toString();
  _commonModeCalc = CommonModeCalulatorBase::instance(type);
  _commonModeCalc->loadSettings(s);
  s.endGroup();
}
