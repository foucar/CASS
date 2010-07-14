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








// ********** Postprocessor 66: median of last values ************

cass::pp301::pp301(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key), _medianStorage(NULL)
{
  loadSettings(0);
}

void cass::pp301::loadSettings(size_t)
{
  CASSSettings settings;

  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());

  // size of dataset for median
  _medianSize = settings.value("medianSize", 100).toInt();

  setupGeneral();

  // Get the input
  _one = setupDependency("HistName");
  bool ret (setupCondition());
  if (!(_one && ret)) return;

  // Create the output
  _result = new Histogram0DFloat();
  createHistList(2*cass::NbrOfWorkers);

  delete _medianStorage;
  _medianStorage = new std::deque<float>();

  std::cout << "PostProcessor " << _key
      << ": computes median of " << _medianSize
      << " of pp " << _one->key()
      <<"Condition is "<<_condition->key()
      << std::endl;
}

void cass::pp301::process(const CASSEvent &evt)
{
  // Get the input
  using namespace std;
  const HistogramFloatBase &one
      (dynamic_cast<const HistogramFloatBase&>((*_one)(evt)));

  // Sum up the histogram under lock
  one.lock.lockForRead();
  float value (accumulate(one.memory().begin(),
                          one.memory().end(),
                          0.f));
  one.lock.unlock();

  float result(0);
  _medianStorage->push_back(value);
  if (_medianStorage->size() >= _medianSize)
  {
    std::vector<float> lastData(_medianStorage->size());
    std::copy( _medianStorage->begin(), _medianStorage->end(), lastData.begin() );
    std::nth_element(lastData.begin(), lastData.begin() + _medianSize/2,
        lastData.end());
    result = lastData[_medianSize/2];
    _medianStorage->pop_front();
  }

  _result->lock.lockForWrite();
  *dynamic_cast<Histogram0DFloat*>(_result) = result;
  _result->lock.unlock();
}




