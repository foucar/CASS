// Copyright (C) 2010 Stephan Kassemeyer

/** @file rankfilter.cpp contains definition of postprocessors that will
 *                     operate on histograms of other postprocessors,
 *                     calculating statistical rank filters like median filter.
 *
 * @author Stephan Kassemeyer
 */

#include <QtCore/QString>
#include <iterator>
#include <algorithm>

#include "rankfilter.h"
#include "histogram.h"
#include "cass_settings.h"
#include "log.h"

using namespace std;
using namespace std::tr1;
using namespace cass;


// ********** Postprocessor 301: median of last values ************

pp301::pp301(const name_t &name)
  : PostProcessor(name),
    _medianStorage(NULL)
{
  loadSettings(0);
}

void pp301::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("PostProcessor");
  s.beginGroup(QString::fromStdString(name()));

  // size of dataset for median
  _medianSize = s.value("MedianSize", 100).toInt();

  setupGeneral();

  // Get the input
  _one = setupDependency("HistName");
  bool ret (setupCondition());
  if (!(_one && ret))
    return;

  // Create the output
  createHistList(tr1::shared_ptr<Histogram0DFloat>(new Histogram0DFloat()));

  delete _medianStorage;
  _medianStorage = new deque<float>();

  Log::add(Log::INFO,"PostProcessor '" + name() +
           "' computes median with a size of '" + toString(_medianSize) +
           "' of pp '"+ _one->name() +
           "'. Condition is '" + _condition->name() + "'");
}

void pp301::process(const CASSEvent& evt, HistogramBackend &res)
{
  const HistogramFloatBase &one
      (dynamic_cast<const HistogramFloatBase&>(_one->result(evt.id())));
  Histogram0DFloat &result(dynamic_cast<Histogram0DFloat&>(res));

  QReadLocker lock(&one.lock);

  float value (accumulate(one.memory().begin(),
                          one.memory().end(),
                          0.f));
  one.lock.unlock();

  if (_medianStorage->empty())
    _medianStorage->resize(_medianSize,value);
  else
    _medianStorage->push_back(value);
  vector<float> lastData(_medianStorage->begin(),_medianStorage->end());
  nth_element(lastData.begin(), lastData.begin() + _medianSize/2,
              lastData.end());
  result = lastData[_medianSize/2];
  _medianStorage->pop_front();
}








// ****** Postprocessor 302: Binary data from file into 2DHistogram *******

pp302::pp302(const name_t &name)
  : PostProcessor(name)
{
  loadSettings(0);
}

void pp302::loadSettings(size_t)
{
  CASSSettings s;

  s.beginGroup("PostProcessor");
  s.beginGroup(QString::fromStdString(name()));

  string filename(s.value("BinaryFile", "").toString().toStdString());
  size_t sizeX(s.value("SizeX",0).toInt());
  size_t sizeY(s.value("SizeY",0).toInt());

  _result = shared_ptr<Histogram2DFloat>(new Histogram2DFloat( sizeX, 0, sizeX-1, sizeY, 0, sizeY-1 ));

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
      in.read(reinterpret_cast<char*>(&((_result)->memory().front())), size_raw );
    }
    else
      throw invalid_argument("pp302:loadSettings(): binary file Histogram2D: wrong size '"+
                             filename +"'");
  }
  else
    throw invalid_argument("pp302:loadSettings(): binary file Histogram2D: cannot open file '" +
                           filename +"'");

  Log::add(Log::INFO,"PostProcessor '" + name() +
           "' loads 2DHistogram from binary File '" + filename +
           "' which is is of size '" + toString(sizeX) +
           "x" + toString(sizeY));
}
