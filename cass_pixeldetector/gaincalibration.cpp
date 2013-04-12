// Copyright (C) 2011 Lutz Foucar

/**
 * @file mapcreators.h contains all correction map creators.
 *
 * @author Lutz Foucar
 */

#include <algorithm>
#include <functional>
#include <numeric>
#include <cmath>

#include <QtCore/QString>
#include <QtCore/QStringList>

#include "gaincalibration.h"

#include "cass_settings.h"
#include "common_data.h"
#include "advanced_pixeldetector.h"
#include "log.h"

using namespace cass;
using namespace pixeldetector;
using namespace std;
using tr1::bind;
using tr1::placeholders::_1;


namespace cass
{
namespace pixeldetector
{
}//end namespace pixeldetector
}//end namespace cass




void GainCalibration::generateCalibration(const Frame &frame)
{
//  if(!_createMaps)
//    return;
//  else
//  {
//    QWriteLocker (&_commondata->lock);
//    if (_storage.size() < _nbrFrames)
//      _storage.push_back(frame.data);
//    else
//    {
//      frame_t pixels;
//      frame_t::iterator offset(_commondata->offsetMap.begin());
//      frame_t::iterator offsetEnd(_commondata->offsetMap.end());
//      frame_t::iterator noise(_commondata->noiseMap.begin());
//      size_t idx(0);
//      for (;offset != offsetEnd; ++offset, ++noise, ++idx)
//      {
//        pixels.clear();
//        for (size_t i=0; i < 2; ++i)
//        {
//          createPixelList(*offset,*noise,_storage,idx,i,pixels);
//          if(!pixels.empty())
//          {
//            *offset = _calcOffset(pixels,_minDisregarded,_maxDisregarded);
//            *noise = calcNoise(pixels, *offset);
//          }
//        }
//      }
//      _createMaps = false;
//      _storage.clear();
//      _commondata->saveOffsetNoiseMaps();
//      _commondata->createCorMap();
//    }
//  }
}

void GainCalibration::loadSettings(CASSSettings &s)
{
//  string detectorname(s.group().split("/").at(s.group().split("/").length()-2).toStdString());
//  s.beginGroup("FixedCreator");
//  _commondata = CommonData::instance(detectorname);
//  _nbrFrames = s.value("NbrFrames",200).toUInt();
//  _maxDisregarded = s.value("DisregardedHighValues",5).toUInt();
//  _minDisregarded = s.value("DisregardedLowValues",0).toUInt();
//  _createMaps = s.value("StartInstantly",false).toBool();
//  _writeMaps = s.value("WriteMaps",true).toBool();
//  if(s.value("UseMedian",false).toBool())
//    _calcOffset = &calcMedian;
//  else
//    _calcOffset = &calcMean;
//  s.endGroup();
}


void GainCalibration::controlCalibration(const string &/*unused*/)
{
//  Log::add(Log::INFO,"MovingMaps::controlCalibration(): start training by collecting '" +
//           toString(_trainingsize) + "' Frames");
//  _framecounter = 0;
//  _createMap = bind(&MovingMaps::train,this,_1);
}

