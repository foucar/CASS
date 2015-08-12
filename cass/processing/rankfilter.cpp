// Copyright (C) 2010 Stephan Kassemeyer

/** @file rankfilter.cpp contains definition of processors that will
 *                     operate on histograms of other processors,
 *                     calculating statistical rank filters like median filter.
 *
 * @author Stephan Kassemeyer
 */

#include <iterator>
#include <algorithm>
#include <numeric>

#include <QtCore/QString>

#include "rankfilter.h"

#include "cass_settings.h"
#include "log.h"

using namespace std;
using namespace std::tr1;
using namespace cass;


// ********** processor 301: median of last values ************

pp301::pp301(const name_t &name)
  : AccumulatingProcessor(name)
{
  loadSettings(0);
}

void pp301::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("Processor");
  s.beginGroup(QString::fromStdString(name()));

  // size of dataset for median
  _medianSize = s.value("MedianSize", 100).toInt();

  setupGeneral();

  // Get the input
  _one = setupDependency("InputName");
  bool ret (setupCondition());
  if (!(_one && ret))
    return;

  createHistList(result_t::shared_pointer(new result_t()));

  Log::add(Log::INFO,"Processor '" + name() +
           "' computes median with a size of '" + toString(_medianSize) +
           "' of pp '"+ _one->name() +
           "'. Condition is '" + _condition->name() + "'");
}

void pp301::process(const CASSEvent& evt, result_t &result)
{
  const result_t &one(_one->result(evt.id()));
  QReadLocker lock(&one.lock);
  QMutexLocker mLock(&_mutex);

  const float value(accumulate(one.begin(), one.end(), 0.f));

  if (_medianStorage.empty())
    _medianStorage.resize(_medianSize,value);
  else
    _medianStorage.push_back(value);

  vector<float> lastData(_medianStorage.begin(),_medianStorage.end());
  nth_element(lastData.begin(), lastData.begin() + _medianSize/2,
              lastData.end());
  result.setValue(lastData[_medianSize/2]);
  _medianStorage.pop_front();
}








// ****** processor 302: Binary data from file into 2DHistogram *******

pp302::pp302(const name_t &name)
  : Processor(name)
{
  loadSettings(0);
}

void pp302::loadSettings(size_t)
{
  CASSSettings s;

  s.beginGroup("Processor");
  s.beginGroup(QString::fromStdString(name()));

  setupGeneral();

  string filename(s.value("BinaryFile", "").toString().toStdString());
  size_t sizeX(s.value("SizeX",0).toInt());
  size_t sizeY(s.value("SizeY",0).toInt());

  _result = result_t::shared_pointer(new result_t(sizeX,sizeY));

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
      in.read(reinterpret_cast<char*>(&(_result->front())), size_raw );
    }
    else
      throw invalid_argument("pp302:loadSettings(): binary file Histogram2D: wrong size '"+
                             filename +"'");
  }
  else
    throw invalid_argument("pp302:loadSettings(): binary file Histogram2D: cannot open file '" +
                           filename +"'");

  Log::add(Log::INFO,"Processor '" + name() +
           "' loads 2DHistogram from binary File '" + filename +
           "' which is is of size '" + toString(sizeX) +
           "x" + toString(sizeY));
}
