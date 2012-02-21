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
#include <QtCore/QTime>
#include <QtCore/QtGlobal>
 
#include "mapcreators_online.h"

#include "cass_settings.h"
#include "common_data.h"
#include "advanced_pixeldetector.h"
#include "log.h"
#include "commonmode_calculator_base.h"

using namespace cass;
using namespace pixeldetector;
using namespace std;
using tr1::bind;
using tr1::placeholders::_1;


void OnlineFixedCreator::controlCalibration(const string &)
{
  Log::add(Log::INFO,"OnlineFixedCreator::controlCalibration(): Start collecting '" +
           toString(_nbrFrames) + "' frames for calibration");
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
    QTime t;
    t.start();
    Log::add(Log::INFO,"OnlineFixedCreator::buildAndCalc(): Collected '"
             + toString(_framecounter) +
             "' frames. Starting to generate the offset and noise map");
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
        tmp_noise  += ((*pixel - old_offset)*(*pixel - tmp_offset));
      }
      *offset = tmp_offset;
      *noise = sqrt(tmp_noise/(accumulatedValues-1));
      if(qFuzzyCompare(*noise,0.f))
      {
        cout << tmp_noise<< " "<< tmp_offset << " "<<accumulatedValues<<endl;
        Log::add(Log::DEBUG0,"OnlineFixedCreator::buildAndCalc(): the noise of pixel '" +
                 toString(distance(_specialstorage.begin(), storagePixels))
                 + "' is 0 after the first iteration.");
      }
      /** calc noise and offset from pixels that do not contain photon hits */
      pixel = storagePixels->begin();
      accumulatedValues = 0;
      tmp_offset = 0.;
      tmp_noise = 0.;
      const pixel_t maxNoise(*noise * _multiplier);
      for(; pixel != lastPixel ; ++pixel)
      {
        const pixel_t pixel_wo_offset(*pixel - *offset);
        if ((pixel_wo_offset < maxNoise))
        {
          ++accumulatedValues;
          const pixel_t old_offset(tmp_offset);
          tmp_offset += ((*pixel - tmp_offset) / accumulatedValues);
          tmp_noise += ((*pixel - old_offset)*(*pixel - tmp_offset));
        }
      }
      *offset = tmp_offset;
      *noise = sqrt(tmp_noise/(accumulatedValues-1));
      if(accumulatedValues == 0)
        Log::add(Log::WARNING,"OnlineFixedCreator::buildAndCalc(): for pixel '" +
                 toString(distance(_specialstorage.begin(), storagePixels))
                 + "' did not find any pixel below the maximum Noise of '" +
                 toString(maxNoise) +"'");
      if(qFuzzyCompare(*noise,0.f))
        Log::add(Log::WARNING,"OnlineFixedCreator::buildAndCalc(): the noise of pixel '" +
                 toString(distance(_specialstorage.begin(), storagePixels))
                 + "' is 0.");
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
    Log::add(Log::INFO,"OnlineFixedCreator::buildAndCalc(): Done creating maps: it took " +
             toString(t.elapsed()) + " ms.");
  }
}

void OnlineFixedCreator::loadSettings(CASSSettings &s)
{
  string detectorname(s.group().split("/").at(s.group().split("/").length()-2).toStdString());
  s.beginGroup("FixedOnlineCreator");
  _commondata = CommonData::instance(detectorname);
  _nbrFrames = s.value("NbrFrames",200).toUInt();
  _framecounter = 0;
  _writeMaps = s.value("WriteMaps",true).toBool();
  if(s.value("StartInstantly",false).toBool())
  {
    Log::add(Log::INFO,"OnlineFixedCreator::loadSettings(): Start collecting '" +
             toString(_nbrFrames) +"' frames for calibration");
    _createMap = bind(&OnlineFixedCreator::buildAndCalc,this,_1);
  }
  else
    _createMap = bind(&OnlineFixedCreator::doNothing,this,_1);
  _multiplier = s.value("Multiplier",4).toFloat();
  s.endGroup();
}



void OnlineFixedCreatorTest::controlCalibration(const string &)
{
  Log::add(Log::INFO,"OnlineFixedCreatorTest::controlCalibration(): Start collecting '" +
           toString(_nbrFrames) + "' frames for calibration");
  _createMap = bind(&OnlineFixedCreatorTest::buildAndCalc,this,_1);
}

void OnlineFixedCreatorTest::buildAndCalc(const Frame& frame)
{
  QWriteLocker (&_commondata->lock);
  /** as long as there are not enough frames collected build up the specail storage */
  if (_framecounter < _nbrFrames)
  {
    ++_framecounter;
    _storage.push_back(frame.data);
  }
  else
  {
    QTime t;
    t.start();
    Log::add(Log::INFO,"OnlineFixedCreatorTest::buildAndCalc(): Collected '"
             + toString(_framecounter) +
             "' frames. Starting to generate the offset and noise map");
    frame_t::iterator offset(_commondata->offsetMap.begin());
    frame_t::const_iterator offsetEnd(_commondata->offsetMap.end());
    frame_t::iterator noise(_commondata->noiseMap.begin());
    size_t idx(0);
    for (;offset != offsetEnd; ++offset, ++noise, ++idx)
    {
      storage_t::iterator storagePixels(_storage.begin());
      storage_t::const_iterator lastStoragePixels(_storage.end());
      size_t accumulatedValues(0);
      pixel_t tmp_offset(0.);
      pixel_t tmp_noise(0.);
      for (;storagePixels != lastStoragePixels; ++storagePixels)
      {
        const pixel_t pixel((*storagePixels)[idx]);
        ++accumulatedValues;
        const pixel_t old_offset(tmp_offset);
        tmp_offset += ((pixel - tmp_offset) / accumulatedValues);
        tmp_noise  += ((pixel - old_offset)*(pixel - tmp_offset));
      }
      *offset = tmp_offset;
      *noise = sqrt(tmp_noise/(accumulatedValues-1));
      if(qFuzzyCompare(*noise,0.f))
        Log::add(Log::WARNING,"OnlineFixedCreatorTest::buildAndCalc(): the noise of pixel '" +
                 toString(idx) + "' is 0 after the first iteration.");
      /** calc noise and offset from pixels that do not contain photon hits */
      storagePixels = _storage.begin();
      accumulatedValues = 0;
      tmp_offset = 0.;
      tmp_noise = 0.;
      const pixel_t maxNoise(*noise * _multiplier);
      for (;storagePixels != lastStoragePixels; ++storagePixels)
      {
        const pixel_t pixel((*storagePixels)[idx]);
        const pixel_t pixel_wo_offset(pixel - *offset);
        if ((pixel_wo_offset < maxNoise))
        {
          ++accumulatedValues;
          const pixel_t old_offset(tmp_offset);
          tmp_offset += ((pixel - tmp_offset) / accumulatedValues);
          tmp_noise += ((pixel - old_offset)*(pixel - tmp_offset));
        }
      }
      *offset = tmp_offset;
      *noise = sqrt(tmp_noise/(accumulatedValues-1));
      if(accumulatedValues == 0)
        Log::add(Log::WARNING,"OnlineFixedCreator::buildAndCalc(): for pixel '" +
                 toString(idx)
                 + "' did not find any pixel below the maximum Noise of '" +
                 toString(maxNoise) +"'");
      if(qFuzzyCompare(*noise,0.f))
        Log::add(Log::WARNING,"OnlineFixedCreator::buildAndCalc(): the noise of pixel '" +
                 toString(idx) + "' is 0.");
    }
    /** save the values to the map */
    _commondata->createCorMap();
    /** now do it again, but this time correct for the common mode level */
    const commonmode::CalculatorBase &calcCommonMode(*_commonModeCalculator);
    offset = _commondata->offsetMap.begin();
    noise = _commondata->noiseMap.begin();
    idx = 0;
    pixel_t commonmodeLevel(0.);
    const size_t width(calcCommonMode.width());
    for (;offset != offsetEnd; ++offset, ++noise, ++idx)
    {
      storage_t::iterator storagePixels(_storage.begin());
      storage_t::const_iterator lastStoragePixels(_storage.end());
      size_t accumulatedValues(0);
      pixel_t tmp_offset(0.);
      pixel_t tmp_noise(0.);
      const pixel_t maxNoise(*noise * _multiplier);
      for (;storagePixels != lastStoragePixels; ++storagePixels)
      {
        if ((idx % width) == 0)
          commonmodeLevel = calcCommonMode(storagePixels->begin()+idx,idx);
        const pixel_t pixel((*storagePixels)[idx]);
        const pixel_t pixel_wo_commonmode(pixel - commonmodeLevel);
        const pixel_t corectedpixel(pixel_wo_commonmode - *offset);
        if ((corectedpixel < maxNoise))
        {
          ++accumulatedValues;
          const pixel_t old_offset(tmp_offset);
          tmp_offset += ((pixel_wo_commonmode - tmp_offset) / accumulatedValues);
          tmp_noise += ((pixel_wo_commonmode - old_offset)*(pixel_wo_commonmode - tmp_offset));
        }
      }
    }

    /** write the maps to file if requested and recreate the correction map.
     *  then reset everything.
     */
    if(_writeMaps)
      _commondata->saveMaps();
    _commondata->createCorMap();
    _createMap = bind(&OnlineFixedCreatorTest::doNothing,this,_1);
    _storage.clear();
    _framecounter = 0;
    Log::add(Log::INFO,"OnlineFixedCreatorTest::buildAndCalc(): Done creating maps: it took " +
             toString(t.elapsed()) + " ms.");
  }
}

void OnlineFixedCreatorTest::loadSettings(CASSSettings &s)
{
  string detectorname(s.group().split("/").at(s.group().split("/").length()-2).toStdString());
  s.beginGroup("FixedOnlineCreator");
  _commondata = CommonData::instance(detectorname);
  _nbrFrames = s.value("NbrFrames",200).toUInt();
  _framecounter = 0;
  _writeMaps = s.value("WriteMaps",true).toBool();
  if(s.value("StartInstantly",false).toBool())
  {
    Log::add(Log::INFO,"OnlineFixedCreatorTest::loadSettings(): Start collecting '" +
             toString(_nbrFrames) +"' frames for calibration");
    _createMap = bind(&OnlineFixedCreatorTest::buildAndCalc,this,_1);
  }
  else
    _createMap = bind(&OnlineFixedCreatorTest::doNothing,this,_1);
  _multiplier = s.value("Multiplier",4).toFloat();
  string commonmodetype (s.value("CommonModeCalculationType","simpleMean").toString().toStdString());
  s.endGroup();
  _commonModeCalculator = commonmode::CalculatorBase::instance(commonmodetype);
  _commonModeCalculator->loadSettings(s);
}
