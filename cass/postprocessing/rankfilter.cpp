// Copyright (C) 2010 Stephan Kassemeyer

/** @file rankfilter.h file contains definition of postprocessors that will
 *                     operate on histograms of other postprocessors,
 *                     calculating statistical rank filters like median filter.
 * @author Stephan Kassemeyer
 */

#include <QtCore/QString>
#include <iterator>
#include <algorithm>

#include "cass.h"
#include "rankfilter.h"
#include "postprocessor.h"
#include "histogram.h"
#include "convenience_functions.h"
#include "cass_settings.h"
#include "log.h"

using namespace std;
using namespace cass;


// ********** Postprocessor 301: median of last values ************

cass::pp301::pp301(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key), _medianStorage(NULL)
{
  loadSettings(0);
}

void cass::pp301::loadSettings(size_t)
{
  CASSSettings s;

  s.beginGroup("PostProcessor");
  s.beginGroup(QString::fromStdString(_key));

  // size of dataset for median
  _medianSize = s.value("MedianSize", 100).toInt();

  setupGeneral();

  // Get the input
  _one = setupDependency("HistName");
  bool ret (setupCondition());
  if (!(_one && ret)) return;

  // Create the output
  _result = new Histogram0DFloat();
  createHistList(2*cass::NbrOfWorkers);

  delete _medianStorage;
  _medianStorage = new deque<float>();

  Log::add(Log::INFO,"PostProcessor '" + _key +
           "' computes median with a size of '" + toString(_medianSize) +
           "' of pp '"+ _one->key() +
           "'. Condition is '" + _condition->key() + "'");
}

void cass::pp301::process(const CASSEvent &evt)
{
  // Get the input
  const HistogramFloatBase &one
      (dynamic_cast<const HistogramFloatBase&>((*_one)(evt)));

  one.lock.lockForRead();
  float value (accumulate(one.memory().begin(),
                          one.memory().end(),
                          0.f));
  one.lock.unlock();

  float result(0);
  if (_medianStorage->empty())
    _medianStorage->resize(_medianSize,value);
  else
    _medianStorage->push_back(value);
  vector<float> lastData(_medianStorage->begin(),_medianStorage->end());
  nth_element(lastData.begin(), lastData.begin() + _medianSize/2,
              lastData.end());
  result = lastData[_medianSize/2];
  _medianStorage->pop_front();

  _result->lock.lockForWrite();
  *dynamic_cast<Histogram0DFloat*>(_result) = result;
  _result->lock.unlock();
}


// ****************** Postprocessor 302: Binary data from file into 2DHistogram ********************

cass::pp302::pp302(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key)
{
  loadSettings(0);
}

void cass::pp302::loadSettings(size_t)
{
  using namespace std;
  setupGeneral();
  CASSSettings s;

  s.beginGroup("PostProcessor");
  s.beginGroup(QString::fromStdString(_key));

  _write = false;

  string filename(s.value("binaryFile", "").toString().toStdString());
  size_t sizeX(s.value("sizeX",0).toInt());
  size_t sizeY(s.value("sizeX",0).toInt());

  _result = new Histogram2DFloat( sizeX, 0, sizeX-1, sizeY, 0, sizeY-1 );

  // load binary file into _result:
  std::ifstream in(filename.c_str(), std::ios::binary|std::ios::ate);
  if (in.is_open())
  {
    const size_t size_raw = in.tellg();
    const size_t size = size_raw / sizeof(float);
    Log::add(Log::DEBUG4,"pp302:loadSettings(): binary size: " + toString(size)+
             " " + toString(sizeX*sizeY) );
    if (size == sizeX*sizeY)
    {
      in.seekg(0,std::ios::beg); // go to beginning
      in.read( reinterpret_cast<char*>(&(static_cast<Histogram2DFloat*>(_result)->memory()[0])), size_raw );
    }
    else
      Log::add(Log::ERROR,"pp302:loadSettings(): binary file Histogram2D: wrong size '"+ filename +"'");
  }
  else
    Log::add(Log::ERROR,"pp302:loadSettings(): binary file Histogram2D: cannot open file '" + filename +"'");

  createHistList(2*cass::NbrOfWorkers,true);

  Log::add(Log::INFO,"PostProcessor '" + _key +
           "' loads 2DHistogram from binary File '" + filename +
           "' which is is of size '" + toString(sizeX) +
           "x" + toString(sizeY) +
           "'. Condition is '" + _condition->key() + "'");
}


void cass::pp302::process(const CASSEvent &/*evt*/)
{
  // don't do anything. Values are fetched on loadSettings.
}




