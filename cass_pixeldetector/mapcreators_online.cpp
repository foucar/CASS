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


void OnlineFixedCreator::controlCalibration(const string &)
{
  cout <<endl<< "OnlineFixedCreator::controlCalibration(): Start collecting "<<_nbrFrames
       <<" frames for calibration"<<endl;
  _createMap = bind(&OnlineFixedCreator::buildAndCalc,this,_1);
}

void OnlineFixedCreator::buildAndCalc(const Frame& frame)
{
  QWriteLocker (&_commondata->lock);
  /** as long as there are not enough frames collected build up the specail storage */
  if (_framecounter < _nbrFrames)
  {
    ++_framecounter;
    _specialstorage.resize(frame.columns * frame.rows);
    specialstorage_t::iterator storagePixel(_specialstorage.begin());
    specialstorage_t::const_iterator lastStoragePixel(_specialstorage.end());
    frame_t::const_iterator pixel(frame.data.begin()) ;
    while(storagePixel != lastStoragePixel)
      (*storagePixel++).push_back(*pixel++);
  }
  else
  {
    cout <<endl<<"OnlineFixedCreator::buildAndCalc(): collected "<<_framecounter
         << " frames. Starting to generate the offset and noise map"<<endl;
    specialstorage_t::iterator storagePixels(_specialstorage.begin());
    specialstorage_t::const_iterator lastStoragePixels(_specialstorage.end());
    frame_t::iterator offset(_commondata->offsetMap.begin());
    frame_t::iterator noise(_commondata->noiseMap.begin());
    for (;storagePixels != lastStoragePixels; ++offset, ++noise, ++storagePixels)
    {
      /** calc noise and offset from all pixels */
      specialstorage_t::value_type::iterator pixel(storagePixels->begin());
      specialstorage_t::value_type::const_iterator lastPixel(storagePixels->end());
      size_t accumulatedValues(0);
      pixel_t tmp_offset(0.);
      pixel_t tmp_noise(0.);
      for(; pixel != lastPixel ; ++pixel)
      {
        ++accumulatedValues;
        const pixel_t old_offset(tmp_offset);
        tmp_offset += ((*pixel - tmp_offset) / accumulatedValues);
        tmp_noise += ((*pixel - old_offset)*(*pixel - tmp_offset));
      }
      *offset = tmp_offset;
      *noise = sqrt(tmp_noise/(accumulatedValues-1));
      /** calc noise and offset from pixels that do not contain photon hits */
      pixel = storagePixels->begin();
      accumulatedValues = 0;
      tmp_offset = 0.;
      tmp_noise = 0.;
      const pixel_t maxNoise(*noise * _multiplier);
      for(; pixel != lastPixel ; ++pixel)
      {
        const pixel_t pixel_wo_offset(*pixel - *offset);
        if ((maxNoise < pixel_wo_offset))
        {
          ++accumulatedValues;
          const pixel_t old_offset(tmp_offset);
          tmp_offset += ((*pixel - tmp_offset) / accumulatedValues);
          tmp_noise += ((*pixel - old_offset)*(*pixel - tmp_offset));
        }
      }
      *offset = tmp_offset;
      *noise = sqrt(tmp_noise/(accumulatedValues-1));
    }
    /** write the maps to file if requested and recreate the correction map.
     *  then reset everything.
     */
    if(_writeMaps)
      _commondata->saveMaps();
    _commondata->createCorMap();
    _createMap = bind(&OnlineFixedCreator::doNothing,this,_1);
    _specialstorage.clear();
    _framecounter = 0;
  }
}

void OnlineFixedCreator::loadSettings(CASSSettings &s)
{
  string detectorname(s.group().split("/").at(s.group().split("/").length()-2).toStdString());
  s.beginGroup("FixedOnlineCreator");
  _commondata = CommonData::instance(detectorname);
  _nbrFrames = s.value("NbrFrames",200).toUInt();
  _framecounter = 0;
//  _maxDisregarded = s.value("DisregardedHighValues",5).toUInt();
//  _minDisregarded = s.value("DisregardedLowValues",0).toUInt();
  _writeMaps = s.value("WriteMaps",true).toBool();
//  if(s.value("UseMedian",false).toBool())
//    _calcOffset = &calcMedian;
//  else
//    _calcOffset = &calcMean;
  if(s.value("StartInstantly",false).toBool())
  {
    cout <<endl<< "OnlineFixedCreator::loadSettings(): Start collecting "<<_nbrFrames
         <<" frames for calibration"<<endl;
    _createMap = bind(&OnlineFixedCreator::buildAndCalc,this,_1);
  }
  else
    _createMap = bind(&OnlineFixedCreator::doNothing,this,_1);
  _multiplier = s.value("Multiplier",4).toFloat();
  s.endGroup();
}
