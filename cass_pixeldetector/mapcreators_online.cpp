// Copyright (C) 2012 Lutz Foucar

/**
 * @file mapcreators_online.h contains correction map creators that work fast
 *                            easy for online purposes.
 *
 * @author Lutz Foucar
 */

#include <algorithm>
#include <functional>
#include <numeric>
#include <cmath>

#include <QtCore/QString>
#include <QtCore/QStringList>

#include "mapcreators_online.h"

#include "cass_settings.h"
#include "common_data.h"
#include "advanced_pixeldetector.h"

using namespace cass;
using namespace pixeldetector;
using namespace std;
using tr1::bind;
using tr1::placeholders::_1;



void OnlineFixedCreator::operator ()(const Frame &frame)
{
  _createMap(frame);
}

void OnlineFixedCreator::buildAndCalc(const Frame& frame)
{
  /** as long as there are not enough frames collected build up the specail storage */
  if (_specialstorage.size() < _nbrFrames)
  {
    _specialstorage.resize(frame.columns * frame.rows);
    specialstorage_t::iterator storagePixel(_specialstorage.begin());
    specialstorage_t::const_iterator lastStoragePixel(_specialstorage.end());
    frame_t::const_iterator pixel(frame.data.begin()) ;
    while(storagePixel != lastStoragePixel)
      (*storagePixel++).push_back(*pixel++);
  }
  else
  {
    specialstorage_t::iterator storagePixels(_specialstorage.begin());
    specialstorage_t::const_iterator lastStoragePixels(_specialstorage.end());
    frame_t::iterator offset(_commondata->offsetMap.begin());
    frame_t::iterator noise(_commondata->noiseMap.begin());
    for (;storagePixels != lastStoragePixels; ++offset, ++noise, ++storagePixels)
    {
      /** calc noise and offset from all pixels */
      specialstorage_t::value_type::iterator pixel(storagePixels->begin());
      specialstorage_t::value_type::const_iterator lastPixel(storagePixels->end());
      for(; pixel != lastPixel ; ++pixel)
      {
//        *offset = _calcOffset(pixels,_minDisregarded,_maxDisregarded);
//        *noise = calcNoise(pixels, *offset);
      }
      /** remove pixels that are potential photon hits */
      pixel = storagePixels->begin();
      const pixel_t maxNoise(*noise * _multiplier);
      while(pixel != storagePixels->end())
      {
        if ((maxNoise < *pixel - *offset))
          pixel = storagePixels->erase(pixel);
        else
          ++pixel;
      }
      /** calc noise and offset from remaining pixels */
      pixel = storagePixels->begin();
      lastPixel = storagePixels->end();
      for(; pixel != lastPixel ; ++pixel)
      {
//        *offset = _calcOffset(pixels,_minDisregarded,_maxDisregarded);
//        *noise = calcNoise(pixels, *offset);
      }
    }
    /** write the maps to file if requested and recreate the correction map.
     *  then reset everything.
     */
    if(_writeMaps)
      _commondata->saveMaps();
    _commondata->createCorMap();
    _createMap = bind(&OnlineFixedCreator::doNothing,this,_1);
    _specialstorage.clear();
  }
}

void OnlineFixedCreator::loadSettings(CASSSettings &s)
{
  string detectorname(s.group().split("/").at(s.group().split("/").length()-2).toStdString());
  s.beginGroup("FixedOnlineCreator");
  _commondata = CommonData::instance(detectorname);
  _nbrFrames = s.value("NbrFrames",200).toUInt();
//  _maxDisregarded = s.value("DisregardedHighValues",5).toUInt();
//  _minDisregarded = s.value("DisregardedLowValues",0).toUInt();
  _writeMaps = s.value("WriteMaps",true).toBool();
//  if(s.value("UseMedian",false).toBool())
//    _calcOffset = &calcMedian;
//  else
//    _calcOffset = &calcMean;
  if(s.value("StartInstantly",false).toBool())
    _createMap = bind(&OnlineFixedCreator::buildAndCalc,this,_1);
  else
    _createMap = bind(&OnlineFixedCreator::doNothing,this,_1);

  s.endGroup();
}
