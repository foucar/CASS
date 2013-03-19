// Copyright (C) 2010-2011 Lutz Foucar
// (C) 2010 Thomas White - Updated to new PP framework

/** @file operations.cpp file contains definition of postprocessors that will
 *                       operate on histograms of other postprocessors
 * @author Lutz Foucar
 */

#include <QtCore/QString>
#include <iterator>
#include <algorithm>
#include <numeric>

#include "cass.h"
#include "operations.h"
#include "postprocessor.h"
#include "histogram.h"
#include "convenience_functions.h"
#include "cass_settings.h"
#include "log.h"
#include "input_base.h"


using namespace cass;
using namespace std;

// ************ Postprocessor 4: Apply boolean NOT to 0D histogram *************

pp4::pp4(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key)
{
  loadSettings(0);
}

void pp4::loadSettings(size_t)
{
  setupGeneral();
  // Get input
  _one = setupDependency("HistName");
  bool ret (setupCondition());
  if (!(_one && ret)) return;

  // Set up output
  _result = new Histogram0DFloat();
  createHistList(2*cass::NbrOfWorkers);

  Log::add(Log::INFO,"PostProcessor '" + _key + "' will apply NOT to PostProcessor '" +
           _one->key() + "'. Condition is '" + _condition->key() + "'");
}

void pp4::process(const CASSEvent& evt)
{
  // Get the input data
  const HistogramFloatBase &one
      (dynamic_cast<const HistogramFloatBase&>((*_one)(evt)));

  // Lock both input and output, and perform negation
  one.lock.lockForRead();
  _result->lock.lockForWrite();
  *dynamic_cast<Histogram0DFloat*>(_result) = !one.isTrue();
  _result->nbrOfFills()=1;
  _result->lock.unlock();
  one.lock.unlock();
}











// ********** Postprocessor 9: Check if histogram is in given range ************

pp9::pp9(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key)
{
  loadSettings(0);
}

void pp9::loadSettings(size_t)
{
  CASSSettings s;

  s.beginGroup("PostProcessor");
  s.beginGroup(QString::fromStdString(_key));

  // Get the range
  _range = make_pair(s.value("LowerLimit",0).toFloat(),
                     s.value("UpperLimit",0).toFloat());
  setupGeneral();

  // Get the input
  _one = setupDependency("HistName");
  bool ret (setupCondition());
  if (!(_one && ret)) return;

  // Create the output
  _result = new Histogram0DFloat();
  createHistList(2*cass::NbrOfWorkers);

  Log::add(Log::INFO,"PostProcessor '" + _key
           + "' will check whether hist in PostProcessor '" + _one->key() +
      "' is between '" + toString(_range.first) + "' and '" + toString(_range.second) +
      "'. Condition is '" + _condition->key() + "'");
}

void pp9::process(const CASSEvent &evt)
{
  // Get the input
  const HistogramFloatBase &one
      (dynamic_cast<const HistogramFloatBase&>((*_one)(evt)));

  // Sum up the histogram under lock
  one.lock.lockForRead();
  float value (accumulate(one.memory().begin(),
                          one.memory().end(),
                          0.f));
  one.lock.unlock();

  _result->lock.lockForWrite();
  *dynamic_cast<Histogram0DFloat*>(_result) =
      _range.first < value &&  value < _range.second;
  _result->nbrOfFills()=1;
  _result->lock.unlock();
}






// ********** Postprocessor 12: PP with constant value ************

pp12::pp12(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key)
{
  loadSettings(0);
}

void pp12::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("PostProcessor");
  s.beginGroup(QString::fromStdString(_key));
  _result = new Histogram0DFloat();
  *dynamic_cast<Histogram0DFloat*>(_result) = s.value("Value",0).toFloat();
  createHistList(2*cass::NbrOfWorkers,true);
  _hide =true;
  _write =false;

  Log::add(Log::INFO,"PostProcessor '" +  _key + "' has constant value of '" +
           toString(dynamic_cast<Histogram0DFloat*>(_result)->getValue()) + "'");
}












// ********** Postprocessor 15: Check if value has changed ************

pp15::pp15(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key)
{
  loadSettings(0);
}

void pp15::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("PostProcessor");
  s.beginGroup(QString::fromStdString(_key));
  _previousVal = 0;
  setupGeneral();
  _hist = setupDependency("HistName");
  if (_hist->getHist(0).dimension() != 0 )
    throw std::runtime_error("PP type 15: Hist is not a 0D Hist");

  bool ret (setupCondition());
  if (!(_hist && ret)) return;
  _result = new Histogram0DFloat();
  createHistList(2*cass::NbrOfWorkers);
  Log::add(Log::INFO,"Postprocessor '" + _key +
           "' will check whether value of '" + _hist->key() +
           "'has changed. It will use condition '" + _condition->key() +"'");

}

void pp15::process(const CASSEvent &evt)
{
  const Histogram0DFloat &val
      (dynamic_cast<const Histogram0DFloat&>((*_hist)(evt)));
  val.lock.lockForRead();
  float value(val.getValue());
  val.lock.unlock();
  _result->lock.lockForWrite();
  *dynamic_cast<Histogram0DFloat*>(_result) = !(qFuzzyCompare(value,_previousVal));
  _result->nbrOfFills()=1;
  _previousVal = value;
  _result->lock.unlock();
}










// ****************** Postprocessor 40: Threshold histogram ********************

pp40::pp40(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key)
{
  loadSettings(0);
}

void pp40::loadSettings(size_t)
{
  CASSSettings s;

  s.beginGroup("PostProcessor");
  s.beginGroup(QString::fromStdString(_key));

  _threshold = s.value("Threshold", 0.0).toFloat();
  setupGeneral();

  // Get input
  _one = setupDependency("HistName");
  bool ret (setupCondition());
  if (!(_one && ret)) return;

  _result = _one->getHist(0).clone();
  createHistList(2*cass::NbrOfWorkers);

  Log::add(Log::INFO,"PostProcessor '" + _key +
      "' will threshold Histogram in PostProcessor '" + _one->key() +
      "' above '" + toString(_threshold) + "'. Condition is '" + _condition->key() + "'");
}

void pp40::histogramsChanged(const HistogramBackend* in)
{
  QWriteLocker lock(&_histLock);
  //return when there is no incomming histogram
  if(!in)
    return;
  //return when the incomming histogram is not a direct dependant
  if (find(_dependencies.begin(),_dependencies.end(),in->key()) == _dependencies.end())
    return;
  //the previous _result pointer is on the histlist and will be deleted
  //with the call to createHistList
  _result = in->clone();
  createHistList(2*cass::NbrOfWorkers);
  //notify all pp that depend on us that our histograms have changed
  PostProcessors::keyList_t dependands (_pp.find_dependant(_key));
  PostProcessors::keyList_t::iterator it (dependands.begin());
  for (; it != dependands.end(); ++it)
    _pp.getPostProcessor(*it).histogramsChanged(_result);
  Log::add(Log::VERBOSEINFO,"Postprocessor '" + _key +
             "': histograms changed => delete existing histo" +
             " and create new one from input");
}

void pp40::process(const CASSEvent &evt)
{
  // Get the input
  const HistogramFloatBase &one
      (dynamic_cast<const HistogramFloatBase&>((*_one)(evt)));

  // Subtract using transform (under locks)
  one.lock.lockForRead();
  _result->lock.lockForWrite();
  std::transform(one.memory().begin(), one.memory().end(),
                 (dynamic_cast<HistogramFloatBase *>(_result))->memory().begin(),
                 bind2nd(threshold(), _threshold));
  _result->nbrOfFills()=1;
  _result->lock.unlock();
  one.lock.unlock();
}





// ****************** Postprocessor 40: Threshold histogram ********************

pp41::pp41(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key)
{
  loadSettings(0);
}

void pp41::loadSettings(size_t)
{
  CASSSettings s;

  s.beginGroup("PostProcessor");
  s.beginGroup(QString::fromStdString(_key));

  setupGeneral();

  // Get input
  _one = setupDependency("HistName");
  _threshold = setupDependency("Threshold");
  bool ret (setupCondition());
  if (!(_one && _threshold && ret)) return;

  _result = _one->getHist(0).clone();
  createHistList(2*cass::NbrOfWorkers);

  Log::add(Log::INFO,"PostProcessor '" + _key +
           "' will threshold Histogram in PostProcessor '" + _one->key() +
           "' above pixels in image '" + _threshold->key() +
           "'. Condition is '" + _condition->key() + "'");
}

void pp41::histogramsChanged(const HistogramBackend* in)
{
  QWriteLocker lock(&_histLock);
  //return when there is no incomming histogram
  if(!in)
    return;
  //return when the incomming histogram is not a direct dependant
  if (find(_dependencies.begin(),_dependencies.end(),in->key()) == _dependencies.end())
    return;
  //the previous _result pointer is on the histlist and will be deleted
  //with the call to createHistList
  _result = in->clone();
  createHistList(2*cass::NbrOfWorkers);
  //notify all pp that depend on us that our histograms have changed
  PostProcessors::keyList_t dependands (_pp.find_dependant(_key));
  PostProcessors::keyList_t::iterator it (dependands.begin());
  for (; it != dependands.end(); ++it)
    _pp.getPostProcessor(*it).histogramsChanged(_result);
  Log::add(Log::VERBOSEINFO,"Postprocessor '" + _key +
             "': histograms changed => delete existing histo" +
             " and create new one from input");
}

void pp41::process(const CASSEvent &evt)
{
  // Get the input
  const HistogramFloatBase &image
      (dynamic_cast<const HistogramFloatBase&>((*_one)(evt)));
  const HistogramFloatBase &threshimage
      (dynamic_cast<const HistogramFloatBase&>((*_one)(evt)));
  HistogramFloatBase &result
      (*dynamic_cast<HistogramFloatBase*>(_result));

  // Subtract using transform (under locks)
  image.lock.lockForRead();
  threshimage.lock.lockForRead();
  _result->lock.lockForWrite();
  transform(image.memory().begin(), image.memory().end(),
            threshimage.memory().begin(),
            result.memory().begin(),
            threshold());
  _result->nbrOfFills()=1;
  _result->lock.unlock();
  threshimage.lock.unlock();
  image.lock.unlock();
}












// *** postprocessors 50 projects 2d hist to 1d histo for a selected region of the axis ***

pp50::pp50(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key)
{
  loadSettings(0);
}

void pp50::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("PostProcessor");
  s.beginGroup(QString::fromStdString(_key));
  _userRange = make_pair(s.value("LowerBound",-1e6).toFloat(),
                         s.value("UpperBound", 1e6).toFloat());
  _axis = static_cast<HistogramBackend::Axis>(s.value("Axis",HistogramBackend::xAxis).toUInt());
  _normalize = s.value("Normalize",false).toBool();
  setupGeneral();
  _pHist = setupDependency("HistName");
  bool ret (setupCondition());
  if (!(ret && _pHist))
    return;
  switch (_axis)
  {
  case (HistogramBackend::xAxis):
    _otherAxis = HistogramBackend::yAxis;
    break;
  case (HistogramBackend::yAxis):
    _otherAxis = HistogramBackend::xAxis;
    break;
  default:
    throw invalid_argument("pp50::loadSettings(): requested _axis '" + toString(_axis) +
                           "' does not exist.");
    break;
  }
  setupParameters(_pHist->getHist(0));
  Log::add(Log::INFO,"PostProcessor '" + _key +
      "' will project histogram of PostProcessor '" + _pHist->key() + "' from '" +
      toString(_range.first) + "' to '" + toString(_range.second) + "' on axis '" +
      toString(_axis) + "'. Normalize '" + toString(_normalize) +
      "'. Condition is '" + _condition->key() + "'");
}

void pp50::histogramsChanged(const HistogramBackend* in)
{
  QWriteLocker lock(&_histLock);
  //return when there is no incomming histogram
  if(!in)
    return;
  //return when the incomming histogram is not a direct dependant
  if (find(_dependencies.begin(),_dependencies.end(),in->key()) == _dependencies.end())
    return;
  setupParameters(*in);
}

void pp50::setupParameters(const HistogramBackend &hist)
{
  if (hist.dimension() != 2)
    throw invalid_argument("pp50::setupParameters()'" + _key +
                           "': Error the histogram we depend on '" + hist.key() +
                           "' is not a 2D Histogram.");
  const AxisProperty &projAxis(hist.axis()[_axis]);
  const AxisProperty &otherAxis(hist.axis()[_otherAxis]);
  _range = make_pair(max(_userRange.first, otherAxis.lowerLimit()),
                     min(_userRange.second, otherAxis.upperLimit()));
  _result = new Histogram1DFloat(projAxis.nbrBins(), projAxis.lowerLimit(), projAxis.upperLimit());
  createHistList(2*cass::NbrOfWorkers);
}

void pp50::process(const CASSEvent& evt)
{
  const Histogram2DFloat &one
      (dynamic_cast<const Histogram2DFloat&>((*_pHist)(evt)));
  one.lock.lockForRead();
  _result->lock.lockForWrite();
  *dynamic_cast<Histogram1DFloat*>(_result) = one.project(_range,_axis);
  _result->nbrOfFills()=1;
  _result->lock.unlock();
  one.lock.unlock();
}













// *** postprocessors 51 calcs integral over a region in 1d histo ***

pp51::pp51(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key)
{
  loadSettings(0);
}

void pp51::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("PostProcessor");
  s.beginGroup(_key.c_str());
  _area = make_pair(s.value("LowerBound",-1e6).toFloat(),
                    s.value("UpperBound", 1e6).toFloat());
  setupGeneral();
  _pHist = setupDependency("HistName");
  bool ret (setupCondition());
  if (!(ret && _pHist))
    return;
  const Histogram1DFloat &one
      (dynamic_cast<const Histogram1DFloat&>(_pHist->getHist(0)));
  _area.first  = max(_area.first, one.axis()[HistogramBackend::xAxis].lowerLimit());
  _area.second = min(_area.second,one.axis()[HistogramBackend::xAxis].upperLimit());
  _result = new Histogram0DFloat();
  createHistList(2*cass::NbrOfWorkers);
  Log::add(Log::INFO, "PostProcessor '" + _key +
      "' will create integral of 1d histogram in PostProcessor '" + _pHist->key() +
      "' from '" + toString(_area.first) + "' to '" + toString(_area.second) +
      "'. Condition is '" + _condition->key() + "'");
}

void pp51::process(const CASSEvent& evt)
{
  const Histogram1DFloat &one
      (dynamic_cast<const Histogram1DFloat&>((*_pHist)(evt)));
  one.lock.lockForRead();
  _result->lock.lockForWrite();
  *dynamic_cast<Histogram0DFloat*>(_result) = one.integral(_area);
  _result->nbrOfFills()=1;
  _result->lock.unlock();
  one.lock.unlock();
}










// *** postprocessors 52 calculate the radial average of a 2d hist given a centre
//     and 1 radius (in case the value is too large, the maximum reasonable value is used) ***

pp52::pp52(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key)
{
  loadSettings(0);
}

void pp52::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("PostProcessor");
  s.beginGroup(QString::fromStdString(_key));
  setupGeneral();
  _pHist = setupDependency("HistName");
  bool ret (setupCondition());
  if (!(ret && _pHist))
    return;
  const Histogram2DFloat &one
      (dynamic_cast<const Histogram2DFloat&>(_pHist->getHist(0)));
  const float center_x_user (s.value("XCenter",512).toFloat());
  const float center_y_user (s.value("YCenter",512).toFloat());
  _center = make_pair(one.axis()[HistogramBackend::xAxis].bin(center_x_user),
                      one.axis()[HistogramBackend::yAxis].bin(center_y_user));
  const size_t dist_center_x_right
      (one.axis()[HistogramBackend::xAxis].nbrBins()-_center.first);
  const size_t dist_center_y_top
      (one.axis()[HistogramBackend::yAxis].nbrBins()-_center.second);
  const size_t min_dist_x (min(dist_center_x_right, _center.first));
  const size_t min_dist_y (min(dist_center_y_top, _center.second));
  _radius = (min (min_dist_x, min_dist_y));
  _result = new Histogram1DFloat(_radius,0,_radius);
  createHistList(2*cass::NbrOfWorkers);
  Log::add(Log::INFO,"PostProcessor " + _key +
      ": will calculate the radial average of histogram of PostProcessor " + _pHist->key() +
      " with xcenter " + toString(s.value("XCenter",512).toFloat()) +
      " ycenter " + toString(s.value("YCenter",512).toFloat()) +
      " in histogram coordinates xcenter " + toString(_center.first) + " ycenter " +
      toString(_center.second) + " maximum radius calculated from the incoming histogram " +
      toString(_radius) + ". Condition is '" + _condition->key() + "'");
}

void pp52::process(const CASSEvent& evt)
{
  //retrieve the memory of the to be subtracted histograms//
  const Histogram2DFloat &one
      (dynamic_cast<const Histogram2DFloat&>((*_pHist)(evt)));
  one.lock.lockForRead();
  _result->lock.lockForWrite();
  *dynamic_cast<Histogram1DFloat*>(_result) = one.radial_project(_center,_radius);
  _result->nbrOfFills()=1;
  _result->lock.unlock();
  one.lock.unlock();
}













// *** postprocessors 53 calculate the radar plot of a 2d hist given a centre
//     and 2 radii (in case the value is too large, the maximum reasonable value is used) ***

pp53::pp53(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key)
{
  loadSettings(0);
}

void pp53::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("PostProcessor");
  s.beginGroup(QString::fromStdString(_key));
  _pHist = setupDependency("HistName");
  setupGeneral();
  bool ret (setupCondition());
  if (!(ret && _pHist))
    return;
  const Histogram2DFloat &one
      (dynamic_cast<const Histogram2DFloat&>(_pHist->getHist(0)));
  const float center_x_user (s.value("XCenter",512).toFloat());
  const float center_y_user (s.value("YCenter",512).toFloat());
  _nbrBins = s.value("NbrBins",360).toUInt();
  _center = make_pair(one.axis()[HistogramBackend::xAxis].bin(center_x_user),
                      one.axis()[HistogramBackend::yAxis].bin(center_y_user));
  const size_t dist_center_x_right
      (one.axis()[HistogramBackend::xAxis].nbrBins()-_center.first);
  const size_t dist_center_y_top
      (one.axis()[HistogramBackend::yAxis].nbrBins()-_center.second);
  const size_t min_dist_x (min(dist_center_x_right, _center.first));
  const size_t min_dist_y (min(dist_center_y_top, _center.second));
  const size_t max_radius (min(min_dist_x, min_dist_y));
  const float minrad_user(s.value("MinRadius",0.).toFloat());
  const float maxrad_user(s.value("MaxRadius",0.).toFloat());
  const size_t minrad (one.axis()[HistogramBackend::xAxis].user2hist(minrad_user));
  const size_t maxrad (one.axis()[HistogramBackend::xAxis].user2hist(maxrad_user));
  _range = make_pair(min(max_radius, minrad),
                     min(max_radius, maxrad));
  _result = new Histogram1DFloat(_nbrBins,0,360);
  createHistList(2*cass::NbrOfWorkers);
  Log::add(Log::INFO,"PostProcessor " + _key +
      ": angular distribution of hist " + _pHist->key() +
      " with xcenter " + toString(s.value("XCenter",512).toFloat()) +
      " ycenter " + toString(s.value("YCenter",512).toFloat()) +
      " in histogram coordinates xcenter " + toString(_center.first) + " ycenter " +
      toString(_center.second) + " minimum radius " +
      toString(s.value("MinRadius",0.).toFloat()) +
      " maximum radius " + toString(s.value("MaxRadius",512.).toFloat()) +
      " in histogram coordinates minimum radius " + toString(_range.first) +
      " maximum radius " + toString(_range.second) + " Histogram has " +
      toString(_nbrBins) + " Bins. Condition is '"+ _condition->key() + "'");
}

void pp53::process(const CASSEvent& evt)
{
  //retrieve the memory of the to be converted histograms//
  const Histogram2DFloat &one
      (dynamic_cast<const Histogram2DFloat&>((*_pHist)(evt)));
  // retrieve the projection from the 2d hist//
  one.lock.lockForRead();
  _result->lock.lockForWrite();
  *dynamic_cast<Histogram1DFloat*>(_result) = one.radar_plot(_center,_range, _nbrBins);
  _result->nbrOfFills()=1;
  _result->lock.unlock();
  one.lock.unlock();
}















// *** postprocessors 54 convert a 2d plot into a r-phi representation ***

pp54::pp54(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key)
{
  loadSettings(0);
}

void pp54::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("PostProcessor");
  s.beginGroup(QString::fromStdString(_key));
  setupGeneral();
  _pHist = setupDependency("HistName");
  bool ret (setupCondition());
  if (!(ret && _pHist))
    return;
  const Histogram2DFloat &one
      (dynamic_cast<const Histogram2DFloat&>(_pHist->getHist(0)));
  const float center_x_user (s.value("XCenter",512).toFloat());
  const float center_y_user (s.value("YCenter",512).toFloat());
  _nbrBins = s.value("NbrBins",360).toUInt();
  _center = make_pair(one.axis()[HistogramBackend::xAxis].bin(center_x_user),
                      one.axis()[HistogramBackend::yAxis].bin(center_y_user));
  const size_t dist_center_x_right
      (one.axis()[HistogramBackend::xAxis].nbrBins()-_center.first);
  const size_t dist_center_y_top
      (one.axis()[HistogramBackend::yAxis].nbrBins()-_center.second);
  const size_t min_dist_x (min(dist_center_x_right, _center.first));
  const size_t min_dist_y (min(dist_center_y_top, _center.second));
  _radius = min(min_dist_x, min_dist_y);
  _result = new Histogram2DFloat(_nbrBins,0,360,_radius,0,_radius);
  createHistList(2*cass::NbrOfWorkers);
  Log::add(Log::INFO,"PostProcessor " + _key +
      ": angular distribution with xcenter " + toString(s.value("XCenter",512).toFloat()) +
      " ycenter " + toString(s.value("YCenter",512).toFloat()) +
      " in histogram coordinates xcenter " + toString(_center.first) +
      " ycenter " + toString(_center.second) + " Histogram has " + toString(_nbrBins) +
      " Bins. The maximum Radius in histogram coordinates " + toString(_radius) +
      ". Condition is '" + _condition->key() + "'");
}

void pp54::process(const CASSEvent& evt)
{
  //retrieve the memory of the to be subtracted histograms//
  const Histogram2DFloat &one
      (dynamic_cast<const Histogram2DFloat&>((*_pHist)(evt)));
  // retrieve the projection from the 2d hist//
  one.lock.lockForRead();
  _result->lock.lockForWrite();
  *dynamic_cast<Histogram2DFloat*>(_result) = one.convert2RPhi(_center,_radius, _nbrBins);
  _result->nbrOfFills()=1;
  _result->lock.unlock();
  one.lock.unlock();
}










// *** postprocessor 56 stores previous version of another histogram ***

pp56::pp56(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key)
{
  loadSettings(0);
}

void pp56::loadSettings(size_t)
{
  setupGeneral();
  _pHist = setupDependency("HistName");
  bool ret (setupCondition());
  if (!(ret && _pHist))
    return;
  const HistogramBackend &one(_pHist->getHist(0));
  _result = one.clone();
  _storage.resize(dynamic_cast<const HistogramFloatBase&>(one).memory().size());

  createHistList(2*cass::NbrOfWorkers,true);
  Log::add(Log::INFO,"Postprocessor '" + _key +
           "' stores the previous histogram from PostProcessor '" + _pHist->key() +
           "'. Condition on postprocessor '" + _condition->key() +"'");
}

void pp56::histogramsChanged(const HistogramBackend* in)
{
  QWriteLocker lock(&_histLock);
  //return when there is no incomming histogram
  if(!in)
    return;
  //return when the incomming histogram is not a direct dependant
  if (find(_dependencies.begin(),_dependencies.end(),in->key()) == _dependencies.end())
    return;
  //the previous _result pointer is on the histlist and will be deleted
  //with the call to createHistList
  _result = in->clone();
  _storage.resize(dynamic_cast<const HistogramFloatBase*>(in)->memory().size());
  createHistList(2*cass::NbrOfWorkers,true);
  //notify all pp that depend on us that our histograms have changed
  PostProcessors::keyList_t dependands (_pp.find_dependant(_key));
  PostProcessors::keyList_t::iterator it (dependands.begin());
  for (; it != dependands.end(); ++it)
    _pp.getPostProcessor(*it).histogramsChanged(_result);
  Log::add(Log::DEBUG4,"Postprocessor '" + _key +
           "': histograms changed => delete existing histo" +
           " and create new one from input");
}

void cass::pp56::process(const CASSEvent& evt)
{
  using namespace std;
  const HistogramFloatBase& one
      (dynamic_cast<const HistogramFloatBase&>((*_pHist)(evt)));
  one.lock.lockForRead();
  _result->lock.lockForWrite();
  HistogramFloatBase::storage_t &histmem(dynamic_cast<HistogramFloatBase*>(_result)->memory());
  copy(_storage.begin(),_storage.end(),histmem.begin());
  copy(one.memory().begin(),one.memory().end(),_storage.begin());
  _result->nbrOfFills() = 1;
  _result->lock.unlock();
  one.lock.unlock();
}















// *** postprocessor 60 histograms 0D values ***

pp60::pp60(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key)
{
  loadSettings(0);
}

void pp60::loadSettings(size_t)
{
  setupGeneral();
  _pHist = setupDependency("HistName");
  bool ret (setupCondition());
  if (!(ret && _pHist))
    return;
  set1DHist(_result,_key);
  createHistList(2*cass::NbrOfWorkers,true);
  Log::add(Log::INFO,"Postprocessor '" + _key +
      "' histograms values from PostProcessor '" +  _pHist->key() +
      "'. Condition on PostProcessor '" + _condition->key() + "'");
}

void pp60::process(const CASSEvent& evt)
{
  const Histogram0DFloat &one
      (dynamic_cast<const Histogram0DFloat&>((*_pHist)(evt)));
  one.lock.lockForRead();
  _result->lock.lockForWrite();
  dynamic_cast<Histogram1DFloat*>(_result)->fill(one.getValue());
  ++_result->nbrOfFills();
  _result->lock.unlock();
  one.lock.unlock();
}











// *** postprocessor 61 averages histograms ***

pp61::pp61(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key)
{
  loadSettings(0);
}

void pp61::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("PostProcessor");
  s.beginGroup(QString::fromStdString(_key));
  setupGeneral();
  unsigned average = s.value("NbrOfAverages", 1).toUInt();
  _alpha =  average ? 2./static_cast<float>(average+1.) : 0.;
  _pHist = setupDependency("HistName");
  bool ret (setupCondition());
  if (!(ret && _pHist))
    return;
  const HistogramBackend &one(_pHist->getHist(0));
  _result = one.clone();
  createHistList(2*cass::NbrOfWorkers,true);
  Log::add(Log::INFO,"Postprocessor '" + _key +
      "' averages histograms from PostProcessor '" +  _pHist->key() +
      "' alpha for the averaging '" + toString(_alpha) +
      "'. Condition on postprocessor '" + _condition->key() + "'");
}

void pp61::histogramsChanged(const HistogramBackend* in)
{
  QWriteLocker lock(&_histLock);
  //return when there is no incomming histogram
  if(!in)
    return;
  //return when the incomming histogram is not a direct dependant
  if (find(_dependencies.begin(),_dependencies.end(),in->key()) == _dependencies.end())
    return;
  //the previous _result pointer is on the histlist and will be deleted
  //with the call to createHistList
  _result = in->clone();
  createHistList(2*cass::NbrOfWorkers,true);
  //notify all pp that depend on us that our histograms have changed
  PostProcessors::keyList_t dependands (_pp.find_dependant(_key));
  PostProcessors::keyList_t::iterator it (dependands.begin());
  for (; it != dependands.end(); ++it)
    _pp.getPostProcessor(*it).histogramsChanged(_result);
  Log::add(Log::VERBOSEINFO,"Postprocessor '" + _key +
             "': histograms changed => delete existing histo" +
             " and create new one from input");
}

void pp61::process(const CASSEvent& evt)
{
  const HistogramFloatBase& one
      (dynamic_cast<const HistogramFloatBase&>((*_pHist)(evt)));
  one.lock.lockForRead();
  _result->lock.lockForWrite();
  ++_result->nbrOfFills();
  float scale = (1./_result->nbrOfFills() < _alpha) ?
                _alpha :
                1./_result->nbrOfFills();
  transform(one.memory().begin(),one.memory().end(),
            dynamic_cast<HistogramFloatBase*>(_result)->memory().begin(),
            dynamic_cast<HistogramFloatBase*>(_result)->memory().begin(),
            Average(scale));
  _result->lock.unlock();
  one.lock.unlock();
}













// *** postprocessor 62 sums up histograms ***

pp62::pp62(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key)
{
  loadSettings(0);
}

void pp62::loadSettings(size_t)
{
  setupGeneral();
  _pHist = setupDependency("HistName");
  bool ret (setupCondition());
  if (!(ret && _pHist))
    return;
  const HistogramBackend &one(_pHist->getHist(0));
  _result = one.clone();
  createHistList(2*cass::NbrOfWorkers,true);
  Log::add(Log::INFO,"Postprocessor '" + _key +
      "' sums up histograms from PostProcessor '" +  _pHist->key() +
      "'. Condition on postprocessor '" + _condition->key() + "'");
}

void pp62::histogramsChanged(const HistogramBackend* in)
{
  QWriteLocker lock(&_histLock);
  //return when there is no incomming histogram
  if(!in)
    return;
  //return when the incomming histogram is not a direct dependant
  if (find(_dependencies.begin(),_dependencies.end(),in->key()) == _dependencies.end())
    return;
  //the previous _result pointer is on the histlist and will be deleted
  //with the call to createHistList
  _result = in->clone();
  createHistList(2*cass::NbrOfWorkers,true);
  //notify all pp that depend on us that our histograms have changed
  PostProcessors::keyList_t dependands (_pp.find_dependant(_key));
  PostProcessors::keyList_t::iterator it (dependands.begin());
  for (; it != dependands.end(); ++it)
    _pp.getPostProcessor(*it).histogramsChanged(_result);
  Log::add(Log::VERBOSEINFO,"Postprocessor '" + _key +
             "': histograms changed => delete existing histo" +
             " and create new one from input");
}

void pp62::process(const CASSEvent& evt)
{
  const HistogramFloatBase& one
      (dynamic_cast<const HistogramFloatBase&>((*_pHist)(evt)));
  one.lock.lockForRead();
  _result->lock.lockForWrite();
  ++_result->nbrOfFills();
  transform(one.memory().begin(),one.memory().end(),
            dynamic_cast<HistogramFloatBase*>(_result)->memory().begin(),
            dynamic_cast<HistogramFloatBase*>(_result)->memory().begin(),
            plus<float>());
  _result->lock.unlock();
  one.lock.unlock();
}








// *** postprocessors 63 calculate the time average of a 0d/1d/2d hist given the number
//     of samples that are going to be used in the calculation ***

pp63::pp63(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key), _num_seen_evt(0), _when_first_evt(0), _first_fiducials(0)
{
  loadSettings(0);
}

void pp63::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("PostProcessor");
  s.beginGroup(_key.c_str());
  const size_t min_time_user (s.value("MinTime",0).toUInt());
  const size_t max_time_user (s.value("MaxTime",300).toUInt());
  _timerange = make_pair(min_time_user,max_time_user);
  _nbrSamples=s.value("NumberOfSamples",5).toUInt();
  setupGeneral();
  _pHist = setupDependency("HistName");
  bool ret (setupCondition());
  if (!(ret && _pHist))
    return;
  const HistogramBackend &one(_pHist->getHist(0));
  _result = one.clone();
  createHistList(2*cass::NbrOfWorkers,true);
  Log::add(Log::INFO,"PostProcessor '" + _key +
      "' will calculate the time average of histogram of PostProcessor '" + _pHist->key() +
      "' from now '" + toString(s.value("MinTime",0).toUInt()) + "' to '" +
      toString(s.value("MaxTime",300).toUInt()) + "' seconds '" + toString(_timerange.first) +
      "' ; '" + toString(_timerange.second) + "' each bin is equivalent to up to '" +
      toString(_nbrSamples) + "' measurements," +
      " Condition on PostProcessor '" + _condition->key() + "'");
}

void pp63::process(const cass::CASSEvent& evt)
{
  //#define debug1
#ifdef debug1
  char timeandday[40];
  struct tm * timeinfo;
#endif
  uint32_t fiducials;
  time_t now_of_event;
  const HistogramFloatBase& one
      (dynamic_cast<const HistogramFloatBase&>((*_pHist)(evt)));
  one.lock.lockForRead();
  _result->lock.lockForWrite();
  // using docu at http://asg.mpimf-heidelberg.mpg.de/index.php/Howto_retrieve_the_Bunch-/Event-id
  //remember the time of the first event
  if(_when_first_evt==0)
  {
    _when_first_evt=static_cast<time_t>(evt.id()>>32);
#ifdef debug1
    timeinfo = localtime ( &_when_first_evt );
    strftime(timeandday,40,"%Y%m%d %H%M%S",timeinfo);
    cout<<"Starting now is "<< timeandday <<" "<<_num_seen_evt<< " "<<_nbrSamples <<endl;
#endif
    _num_seen_evt=1;
    _first_fiducials=static_cast<uint32_t>(evt.id() & 0x000000001FFFFF00);
  }
  now_of_event=static_cast<time_t>(evt.id()>>32);
#ifdef debug1
  timeinfo = localtime ( &now_of_event );
  strftime(timeandday,40,"%Y%m%d %H%M%S",timeinfo);
#endif
  fiducials=static_cast<uint32_t>(evt.id() & 0x000000001FFFFF00);
#ifdef debug1
  timeinfo = localtime ( &now_of_event );
  strftime(timeandday,40,"%Y%m%d %H%M%S",timeinfo);
  //      cout<<"Starting now is "<< timeandday <<" "<<_num_seen_evt<< " "<<_nbrSamples <<endl;
  cout<<"ora "<< timeandday<<" "<<_first_fiducials << " " <<fiducials << " " << _num_seen_evt <<endl;
#endif
  if(fiducials>_first_fiducials+(_nbrSamples-1)*4608 /*0x1200 ??why*/
     && now_of_event>=_when_first_evt)
  {
#ifdef debug1
    cout <<"time to forget"<<endl;
#endif
    //remember the time also whenever (_num_seen_evt-1)%_nbrSamples==0
    _first_fiducials=fiducials;
    _when_first_evt=now_of_event;
    _num_seen_evt=1;
  }

  //    if(now_of_event<_when_first_evt-100)
  if(_first_fiducials>fiducials)
  {
#ifdef debug1
    cout <<"extra time to forget"<<endl;
#endif
    //remember the time also whenever (_num_seen_evt-1)%_nbrSamples==0
    _first_fiducials=fiducials;
    _when_first_evt=now_of_event;
    _num_seen_evt=1;
  }

  ++_result->nbrOfFills();
  transform(one.memory().begin(),one.memory().end(),
            dynamic_cast<HistogramFloatBase*>(_result)->memory().begin(),
            dynamic_cast<HistogramFloatBase*>(_result)->memory().begin(),
            TimeAverage(float(_num_seen_evt)));
  ++_num_seen_evt;
  if(_num_seen_evt>_nbrSamples+1) cout<<"pp64::process(): How... it smells like fish! "
      <<_num_seen_evt
      <<" "<<_nbrSamples
      <<endl;
  _result->lock.unlock();
  one.lock.unlock();
}













// ***  pp 64 takes a 0d histogram (value) as input and writes it in the last bin of a 1d histogram
//    *** while shifting all other previously saved values one bin to the left.

pp64::pp64(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key)
{
  loadSettings(0);
}

void pp64::loadSettings(size_t)
{
  CASSSettings s;

  s.beginGroup("PostProcessor");
  s.beginGroup(_key.c_str());

  _hist = setupDependency("HistName");

  bool ret (setupCondition());
  if ( !(_hist && ret) ) return;

  setupGeneral();
  _size = s.value("Size", 10000).toUInt();

  _result = new Histogram1DFloat(_size, 0, _size-1,"Shots");
  createHistList(2*cass::NbrOfWorkers,true);

  Log::add(Log::INFO,"PostProcessor '" + _key +
           "' will make a history of values of histogram in pp '" +
           _hist->key() + ", size of history '" + toString(_size) +
           "' Condition on postprocessor '" + _condition->key() + "'");
}

void pp64::process(const cass::CASSEvent &evt)
{
  const HistogramFloatBase &hist
      (dynamic_cast<const HistogramFloatBase &>((*_hist)(evt)));
  const HistogramFloatBase::storage_t &values(hist.memory());
  HistogramFloatBase::storage_t::const_iterator value(values.begin());
  HistogramFloatBase::storage_t::const_iterator valueEnd(values.end());

  HistogramFloatBase::storage_t &mem
      ((dynamic_cast<Histogram1DFloat *>(_result))->memory());

  hist.lock.lockForRead();
  _result->lock.lockForWrite();
  for(; value != valueEnd ;++value)
  {
    rotate(mem.begin(), mem.begin()+1, mem.end());
    mem[_size-1] = *value;
  }
  _result->lock.unlock();
  hist.lock.unlock();
}







// *** postprocessor 65 histograms 2 0D values to 2D histogram ***

pp65::pp65(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key)
{
  loadSettings(0);
}

void pp65::loadSettings(size_t)
{
  setupGeneral();
  _one = setupDependency("HistOne");
  _two = setupDependency("HistTwo");
  bool ret (setupCondition());
  if ( !(_one && _two && ret) )
    return;
  if (_one->getHist(0).dimension() != 0 ||
      _two->getHist(0).dimension() != 0)
    throw std::runtime_error("PP type 65: Either HistOne or HistTwo is not a 0D Hist");
  set2DHist(_result,_key);
  createHistList(2*cass::NbrOfWorkers,true);
  Log::add(Log::INFO,"Postprocessor '" + _key +
      "': histograms values from PostProcessor '" +  _one->key() +"' and '" +
      _two->key() + ". condition on PostProcessor '" + _condition->key() + "'");
}

void pp65::process(const CASSEvent& evt)
{
  const Histogram0DFloat &one
      (dynamic_cast<const Histogram0DFloat&>((*_one)(evt)));
  const Histogram0DFloat &two
      (dynamic_cast<const Histogram0DFloat&>((*_two)(evt)));
  one.lock.lockForRead();
  two.lock.lockForRead();
  _result->lock.lockForWrite();
  dynamic_cast<Histogram2DFloat*>(_result)->fill(one.getValue(),two.getValue());
  ++_result->nbrOfFills();
  _result->lock.unlock();
  two.lock.unlock();
  one.lock.unlock();
}












// *** postprocessor 66 histograms 2 1D values to 2D histogram ***

pp66::pp66(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key)
{
  loadSettings(0);
}

void pp66::loadSettings(size_t)
{
  setupGeneral();
  _one = setupDependency("HistOne");
  _two = setupDependency("HistTwo");
  bool ret (setupCondition());
  if ( !(_one && _two && ret) )
    return;
  const HistogramFloatBase &one(dynamic_cast<const HistogramFloatBase&>(_one->getHist(0)));
  const HistogramFloatBase &two(dynamic_cast<const HistogramFloatBase&>(_two->getHist(0)));
  if (one.dimension() != 1 || two.dimension() != 1)
    throw runtime_error("pp66::loadSettings(): HistOne '" + one.key() +
                        "' with dimension '" + toString(one.dimension()) +
                        "' or HistTwo '" + two.key() + "' has dimension '" +
                        toString(two.dimension()) + "' does not have dimension 1");
  _result = new Histogram2DFloat(one.axis()[HistogramBackend::xAxis].nbrBins(),
                                 one.axis()[HistogramBackend::xAxis].lowerLimit(),
                                 one.axis()[HistogramBackend::xAxis].upperLimit(),
                                 two.axis()[HistogramBackend::xAxis].nbrBins(),
                                 two.axis()[HistogramBackend::xAxis].lowerLimit(),
                                 two.axis()[HistogramBackend::xAxis].upperLimit(),
                                 one.axis()[HistogramBackend::xAxis].title(),
                                 two.axis()[HistogramBackend::xAxis].title());
  createHistList(2*cass::NbrOfWorkers);
  Log::add(Log::INFO,"Postprocessor '" + _key +
           "' creates a two dim histogram from PostProcessor '" + _one->key() +
           "' and '" +  _two->key() + "'. condition on PostProcessor '" +
           _condition->key() + "'");
}

void pp66::histogramsChanged(const HistogramBackend* in)
{
  QWriteLocker lock(&_histLock);
  //return when there is no incomming histogram
  if(!in)
    return;
  //return when the incomming histogram is not a direct dependant
  PostProcessors::keyList_t::const_iterator itDep
      (find(_dependencies.begin(),_dependencies.end(),in->key()));
  if (_dependencies.end() == itDep)
    return;
  //the previous _result pointer is on the histlist and will be deleted
  //with the call to createHistList
  /** @todo right now we just resize our histogram depending on the histogram
   *        that cause the call to this function. How do we do this correctly?
   *        Either we make it such, that we do not accept two different histograms
   *        or the whole notifying must be done differently.
   */
  const HistogramFloatBase &one(dynamic_cast<const HistogramFloatBase&>(_one->getHist(0)));
  const HistogramFloatBase &two(dynamic_cast<const HistogramFloatBase&>(_two->getHist(0)));
  if ((*itDep) == one.key())
  {
    QReadLocker lock(&two.lock);
    _result = new Histogram2DFloat(in->axis()[HistogramBackend::xAxis].nbrBins(),
                                   in->axis()[HistogramBackend::xAxis].lowerLimit(),
                                   in->axis()[HistogramBackend::xAxis].upperLimit(),
                                   two.axis()[HistogramBackend::xAxis].nbrBins(),
                                   two.axis()[HistogramBackend::xAxis].lowerLimit(),
                                   two.axis()[HistogramBackend::xAxis].upperLimit(),
                                   in->axis()[HistogramBackend::xAxis].title(),
                                   two.axis()[HistogramBackend::xAxis].title());
  }
  else if ((*itDep) == two.key())
  {
    QReadLocker lock(&one.lock);
    _result = new Histogram2DFloat(one.axis()[HistogramBackend::xAxis].nbrBins(),
                                   one.axis()[HistogramBackend::xAxis].lowerLimit(),
                                   one.axis()[HistogramBackend::xAxis].upperLimit(),
                                   in->axis()[HistogramBackend::xAxis].nbrBins(),
                                   in->axis()[HistogramBackend::xAxis].lowerLimit(),
                                   in->axis()[HistogramBackend::xAxis].upperLimit(),
                                   one.axis()[HistogramBackend::xAxis].title(),
                                   in->axis()[HistogramBackend::xAxis].title());
  }
  else
  {
    throw logic_error("pp66::histogramsChanged()'" + _key + "': Error'" + *itDep +
                      "' is neither hist one '" + one.key() +"' nor hist two '" +
                      two.key() + "'");
  }
  createHistList(2*cass::NbrOfWorkers);
  //notify all pp that depend on us that our histograms have changed
  PostProcessors::keyList_t dependands (_pp.find_dependant(_key));
  PostProcessors::keyList_t::iterator it (dependands.begin());
  for (; it != dependands.end(); ++it)
    _pp.getPostProcessor(*it).histogramsChanged(_result);
  Log::add(Log::VERBOSEINFO,"Postprocessor '" + _key +
             "': histograms changed => delete existing histo" +
             " and create new one from input");
}

void pp66::process(const CASSEvent& evt)
{
  const Histogram1DFloat &one
      (dynamic_cast<const Histogram1DFloat&>((*_one)(evt)));
  const Histogram1DFloat &two
      (dynamic_cast<const Histogram1DFloat&>((*_two)(evt)));
  one.lock.lockForRead();
  two.lock.lockForRead();
  _result->lock.lockForWrite();
  HistogramFloatBase::storage_t &memory(dynamic_cast<Histogram2DFloat*>(_result)->memory());
  const size_t oneNBins(one.axis()[HistogramBackend::xAxis].nbrBins());
  const size_t twoNBins(two.axis()[HistogramBackend::xAxis].nbrBins());
  for (size_t j(0); j < twoNBins; ++j)
    for (size_t i(0); i < oneNBins; ++i)
      memory[j*oneNBins+i] = one.memory()[i]*two.memory()[j];
  _result->nbrOfFills()=1;
  _result->lock.unlock();
  two.lock.unlock();
  one.lock.unlock();
}















// *** postprocessor 67 histograms 2 0D values to 1D histogram ***

pp67::pp67(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key)
{
  loadSettings(0);
}

void pp67::loadSettings(size_t)
{
  setupGeneral();
  _one = setupDependency("HistOne");
  _two = setupDependency("HistTwo");
  bool ret (setupCondition());
  if ( !(_one && _two && ret) )
    return;
  if (_one->getHist(0).dimension() != 0 ||
      _two->getHist(0).dimension() != 0)
    throw std::runtime_error("PP type 67: Either HistOne or HistTwo is not a 0D Hist");
  set1DHist(_result,_key);
  createHistList(2*cass::NbrOfWorkers,true);
  Log::add(Log::INFO,"Postprocessor '" + _key +
      "' makes a 1D Histogram where '" + _one->key() +
      "' defines the x bin to fill and '" +  _two->key() +
      "' defines the weight of how much to fill the x bin" +
      ". Condition on PostProcessor '" + _condition->key() + "'");
}

void pp67::process(const CASSEvent& evt)
{
  const Histogram0DFloat &one
      (dynamic_cast<const Histogram0DFloat&>((*_one)(evt)));
  const Histogram0DFloat &two
      (dynamic_cast<const Histogram0DFloat&>((*_two)(evt)));
  one.lock.lockForRead();
  two.lock.lockForRead();
  _result->lock.lockForWrite();
  _result->clear();
  dynamic_cast<Histogram1DFloat*>(_result)->fill(one.getValue(),two.getValue());
  _result->lock.unlock();
  two.lock.unlock();
  one.lock.unlock();
}















// *** postprocessor 68 histograms 0D and 1d Histogram to 2D histogram ***

pp68::pp68(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key)
{
  loadSettings(0);
}

void pp68::loadSettings(size_t)
{
  setupGeneral();
  _one = setupDependency("HistOne");
  _two = setupDependency("HistTwo");
  bool ret (setupCondition());
  if ( !(_one && _two && ret) )
    return;
  if (_one->getHist(0).dimension() != 1 ||
      _two->getHist(0).dimension() != 0)
    throw std::runtime_error("PP type 68: Either HistOne is not a 1d histo or"
                             " HistTwo is not a 0D Hist");
  setup(dynamic_cast<const Histogram1DFloat&>(_one->getHist(0)));
  Log::add(Log::INFO,"Postprocessor '" + _key +
      "' makes a 2D Histogram where '" + _one->key() +
      "' defines the x axis to fill and '" + _two->key() +
       "' defines the y axis bin. Condition on PostProcessor '" + _condition->key() + "'");
}

void pp68::setup(const Histogram1DFloat &one)
{
  CASSSettings s;
  s.beginGroup("PostProcessor");
  s.beginGroup(QString::fromStdString(_key));
  const AxisProperty &xaxis(one.axis()[HistogramBackend::xAxis]);
  _result = new Histogram2DFloat(xaxis.nbrBins(),
                                 xaxis.lowerLimit(),
                                 xaxis.upperLimit(),
                                 s.value("YNbrBins",1).toUInt(),
                                 s.value("YLow",0).toFloat(),
                                 s.value("YUp",0).toFloat(),
                                 xaxis.title(),
                                 s.value("YTitle","y-axis").toString().toStdString());
  createHistList(2*cass::NbrOfWorkers);
}

void pp68::histogramsChanged(const HistogramBackend* in)
{
  using namespace std;
  QWriteLocker lock(&_histLock);
  //return when there is no incomming histogram
  if(!in)
    return;
  //return when the incomming histogram is not a direct dependant
  if (find(_dependencies.begin(),_dependencies.end(),in->key()) == _dependencies.end())
    return;
  //return when it is the 0d histogram (y axis)
  if (in->dimension() == 0)
    return;
  setup(*dynamic_cast<const Histogram1DFloat*>(in));
  //notify all pp that depend on us that our histograms have changed
  PostProcessors::keyList_t dependands (_pp.find_dependant(_key));
  PostProcessors::keyList_t::iterator it (dependands.begin());
  for (; it != dependands.end(); ++it)
    _pp.getPostProcessor(*it).histogramsChanged(_result);
}

void pp68::process(const CASSEvent& evt)
{
  using namespace std;
  const Histogram1DFloat &one
      (dynamic_cast<const Histogram1DFloat&>((*_one)(evt)));
  const Histogram0DFloat &two
      (dynamic_cast<const Histogram0DFloat&>((*_two)(evt)));
  one.lock.lockForRead();
  two.lock.lockForRead();
  _result->lock.lockForWrite();
  _result->clear();
  Histogram2DFloat& result(*dynamic_cast<Histogram2DFloat*>(_result));
  try
  {
    size_t bin(result.axis()[HistogramBackend::yAxis].bin(two.getValue()));
    HistogramFloatBase::storage_t::iterator memorypointer
        (result.memory().begin() + bin*result.axis()[HistogramBackend::xAxis].nbrBins());
    copy(one.memory().begin(),one.memory().end()-2,memorypointer);
  }
  catch (const out_of_range& error)
  {

  }
  _result->lock.unlock();
  two.lock.unlock();
  one.lock.unlock();
}









// *** postprocessor 69 histograms 2 0D values to 1D scatter plot ***

pp69::pp69(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key)
{
  loadSettings(0);
}

void pp69::loadSettings(size_t)
{
  setupGeneral();
  _one = setupDependency("HistOne");
  _two = setupDependency("HistTwo");
  bool ret (setupCondition());
  if ( !(_one && _two && ret) )
    return;
  if (_one->getHist(0).dimension() != 0 ||
      _two->getHist(0).dimension() != 0)
    throw std::runtime_error("PP type 67: Either HistOne or HistTwo is not a 0D Hist");
  set1DHist(_result,_key);
  createHistList(2*cass::NbrOfWorkers,true);
  Log::add(Log::INFO,"Postprocessor '" + _key +
           "' makes a 1D Histogram where '"+  _one->key() +
           "' defines the x bin to set and '" + _two->key() +
           "' defines the y value of the x bin"
           ". Condition on PostProcessor '" + _condition->key() + "'");
}

void pp69::process(const CASSEvent& evt)
{
  const Histogram0DFloat &one
      (dynamic_cast<const Histogram0DFloat&>((*_one)(evt)));
  const Histogram0DFloat &two
      (dynamic_cast<const Histogram0DFloat&>((*_two)(evt)));
  one.lock.lockForRead();
  two.lock.lockForRead();
  _result->lock.lockForWrite();
  Histogram1DFloat::storage_t &mem(dynamic_cast<Histogram1DFloat*>(_result)->memory());

  const float x(one.getValue());
  const float weight(two.getValue());
  const int nxBins(static_cast<const int>(_result->axis()[HistogramBackend::xAxis].nbrBins()));
  const float xlow(_result->axis()[HistogramBackend::xAxis].lowerLimit());
  const float xup(_result->axis()[HistogramBackend::xAxis].upperLimit());
  const int xBin(static_cast<int>( nxBins * (x - xlow) / (xup-xlow)));

  //check whether the fill is in the right range//
  const bool xInRange = 0<=xBin && xBin<nxBins;
  // if in range fill the memory otherwise figure out whether over of underflow occured//
  if (xInRange)
    mem[xBin] = weight;
  else if (xBin >= nxBins)
    mem[nxBins+HistogramBackend::Overflow] += 1;
  else if (xBin < 0)
    mem[nxBins+HistogramBackend::Underflow] += 1;
  //increase the number of fills//
  ++(_result->nbrOfFills());

  _result->lock.unlock();
  two.lock.unlock();
  one.lock.unlock();
}














// ***  pp 70 subsets a histogram ***

pp70::pp70(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key)
{
  loadSettings(0);
}

void pp70::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("PostProcessor");
  s.beginGroup(QString::fromStdString(_key));
  setupGeneral();
  _pHist = setupDependency("HistName");
  bool ret (setupCondition());
  if (!(ret && _pHist))
    return;
  const HistogramFloatBase &one
      (dynamic_cast<const HistogramFloatBase&>(_pHist->getHist(0)));
  if (0 == one.dimension())
    throw runtime_error("pp70::loadSettings(): Unknown dimension of incomming histogram");
  _userXRange = make_pair(s.value("XLow",0).toFloat(),
                          s.value("XUp",1).toFloat());
  const AxisProperty &xaxis(one.axis()[HistogramBackend::xAxis]);
  _inputOffset=(xaxis.bin(_userXRange.first));
  const size_t binXUp (xaxis.bin(_userXRange.second));
  const size_t nXBins (binXUp-_inputOffset);
  const float xLow (xaxis.position(_inputOffset));
  const float xUp (xaxis.position(binXUp));
  string output("PostProcessor '" + _key +
                "' returns a subset of histogram in pp '"  +  _pHist->key() +
                "' which has dimension '" + toString(one.dimension()) +
                "'. Subset is xLow:" + toString(xLow) + "(" + toString(_inputOffset) +
                "), xUp:" + toString(xUp) + "(" + toString(binXUp) +
                "), xNbrBins:" + toString(nXBins));
  if (1 == one.dimension())
  {
    _result = new Histogram1DFloat(nXBins,xLow,xUp);
  }
  else if (2 == one.dimension())
  {
    _userYRange = make_pair(s.value("YLow",0).toFloat(),
                            s.value("YUp",1).toFloat());
    const AxisProperty &yaxis (one.axis()[HistogramBackend::yAxis]);
    const size_t binYLow(yaxis.bin(_userYRange.first));
    const size_t binYUp (yaxis.bin(_userYRange.second));
    const size_t nYBins = (binYUp - binYLow);
    const float yLow (yaxis.position(binYLow));
    const float yUp (yaxis.position(binYUp));
    _inputOffset = static_cast<size_t>(binYLow*xaxis.nbrBins()+xLow + 0.1);
    _result = new Histogram2DFloat(nXBins,xLow,xUp,
                                   nYBins,yLow,yUp);
    output += ", yLow:"+ toString(yLow) + "(" + toString(binYLow) +
              "), yUp:" + toString(yUp) + "(" + toString(binYLow) +
              "), yNbrBins:"+ toString(nXBins) + ", linearized offset is now:" +
              toString(_inputOffset);
  }
  output += ". Condition on postprocessor '" + _condition->key() + "'";
  Log::add(Log::INFO,output);
  createHistList(2*cass::NbrOfWorkers);
}

void pp70::histogramsChanged(const HistogramBackend* in)
{
  QWriteLocker lock(&_histLock);
  //return when there is no incomming histogram
  if(!in)
    return;
  //return when the incomming histogram is not a direct dependant
  if (find(_dependencies.begin(),_dependencies.end(),in->key()) == _dependencies.end())
    return;
  const AxisProperty &xaxis(in->axis()[HistogramBackend::xAxis]);
  _inputOffset=(xaxis.bin(_userXRange.first));
  const size_t binXUp (xaxis.bin(_userXRange.second));
  const size_t nXBins (binXUp-_inputOffset);
  const float xLow (xaxis.position(_inputOffset));
  const float xUp (xaxis.position(binXUp));
  string output("PostProcessor '" + _key +
                "' histogramsChanged: returns a subset of histogram in pp '"  +  _pHist->key() +
                "' which has dimension '" + toString(in->dimension()) +
                "'. Subset is xLow:" + toString(xLow) + "(" + toString(_inputOffset) +
                "), xUp:" + toString(xUp) + "(" + toString(binXUp) +
                "), xNbrBins:" + toString(nXBins));
  if (1 == in->dimension())
  {
    _result = new Histogram1DFloat(nXBins,xLow,xUp);
  }
  else if (2 == in->dimension())
  {
    const AxisProperty &yaxis (in->axis()[HistogramBackend::yAxis]);
    const size_t binYLow(yaxis.bin(_userYRange.first));
    const size_t binYUp (yaxis.bin(_userYRange.second));
    const size_t nYBins = (binYUp - binYLow);
    _inputOffset = static_cast<size_t>(binYLow*xaxis.nbrBins()+xLow +0.1);
    const float yLow (yaxis.position(binYLow));
    const float yUp (yaxis.position(binYUp));
    _result = new Histogram2DFloat(nXBins,xLow,xUp,
                                   nYBins,yLow,yUp);
    output += ", yLow:"+ toString(yLow) + "(" + toString(binYLow) +
              "), yUp:" + toString(yUp) + "(" + toString(binYLow) +
              "), yNbrBins:"+ toString(nXBins) + ", linearized offset is now:" +
              toString(_inputOffset);
  }
  Log::add(Log::VERBOSEINFO,output);
  createHistList(2*cass::NbrOfWorkers);
  //notify all pp that depend on us that our histograms have changed
  PostProcessors::keyList_t dependands (_pp.find_dependant(_key));
  PostProcessors::keyList_t::iterator it (dependands.begin());
  for (; it != dependands.end(); ++it)
    _pp.getPostProcessor(*it).histogramsChanged(_result);
}


void pp70::process(const cass::CASSEvent& evt)
{
  const HistogramFloatBase& input
      (dynamic_cast<const HistogramFloatBase&>((*_pHist)(evt)));
  input.lock.lockForRead();
  _result->lock.lockForWrite();
  const HistogramFloatBase::storage_t &imem (input.memory());
  HistogramFloatBase::storage_t::const_iterator iit
      (imem.begin()+_inputOffset);
  HistogramFloatBase::storage_t &rmem
      (dynamic_cast<HistogramFloatBase*>(_result)->memory());
  HistogramFloatBase::storage_t::iterator rit(rmem.begin());
  const size_t &resultNbrXBins (_result->axis()[HistogramBackend::xAxis].nbrBins());
  const size_t resultNbrYBins = (_result->dimension() == 2) ?
                                _result->axis()[HistogramBackend::yAxis].nbrBins() :
                                1;
  const size_t &inputNbrXBins (input.axis()[HistogramBackend::xAxis].nbrBins());
  for (size_t yBins=0;yBins < resultNbrYBins; ++yBins)
  {
    copy(iit,iit+resultNbrXBins,rit);
    advance(iit,inputNbrXBins);
    advance(rit,resultNbrXBins);
  }
  _result->nbrOfFills()=1;
  _result->lock.unlock();
  input.lock.unlock();
}







// ***  pp 71 returns the maximum value of a Histogram ***

pp71::pp71(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key)
{
  loadSettings(0);
}

void pp71::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("PostProcessor");
  s.beginGroup(QString::fromStdString(_key));
  setupGeneral();
  _pHist = setupDependency("HistName");
  bool ret (setupCondition());
  if (!(ret && _pHist))
    return;
  string functype(s.value("RetrieveType","max").toString().toStdString());
  if (functype == "max")
    _func = &max_element<HistogramFloatBase::storage_t::const_iterator>;
  else if (functype == "min")
    _func = &min_element<HistogramFloatBase::storage_t::const_iterator>;
  else
    throw invalid_argument("pp71::loadSettings(): RetrieveType '" + functype + "' unknown.");

  _result = new Histogram0DFloat();
  createHistList(2*cass::NbrOfWorkers);
  Log::add(Log::INFO,"PostProcessor '" + _key +
           "' returns the value in '" + _pHist->key() +
           "' that is retrieved by using function type '" + functype +
           "' .Condition on postprocessor '" + _condition->key() + "'");
}

void pp71::process(const cass::CASSEvent& evt)
{
  const HistogramFloatBase& one
      (dynamic_cast<const HistogramFloatBase&>((*_pHist)(evt)));
  one.lock.lockForRead();
  _result->lock.lockForWrite();
  dynamic_cast<Histogram0DFloat*>(_result)->
      fill(*(_func(one.memory().begin(), one.memory().end())));
  _result->nbrOfFills()=1;
  _result->lock.unlock();
  one.lock.unlock();
}






// ***  pp 72 returns column of a table ***

pp72::pp72(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key)
{
  loadSettings(0);
}

void pp72::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("PostProcessor");
  s.beginGroup(QString::fromStdString(_key));
  setupGeneral();
  _table = setupDependency("TableName");
  bool ret (setupCondition());
  if (!(ret && _table))
    return;
  _colIdx = s.value("ColumnIndex",0).toUInt();

  size_t maxIdx(_table->getHist(0).axis()[HistogramBackend::xAxis].size());
  if (_colIdx >= maxIdx)
    throw runtime_error("pp72::loadSettings(): '" + _key + "' The requested " +
                        "column index '" + toString(_colIdx) + " 'exeeds the " +
                        "maximum possible index value '" + toString(maxIdx) + "'");

  _result = new Histogram1DFloat();
  createHistList(2*cass::NbrOfWorkers);
  Log::add(Log::INFO,"PostProcessor '" + _key +
           "' retrieves column with index '" + toString(_colIdx) +
           "' from table " + _table->key() + "' .Condition on postprocessor '" +
           _condition->key() + "'");
}

void pp72::process(const cass::CASSEvent& evt)
{
  const Histogram2DFloat& table
      (dynamic_cast<const Histogram2DFloat&>((*_table)(evt)));
  const HistogramFloatBase::storage_t &tableContents(table.memory());
  Histogram1DFloat &col(dynamic_cast<Histogram1DFloat&>(*_result));

  table.lock.lockForRead();
  col.lock.lockForWrite();

  col.clearline();

  size_t nCols(table.axis()[HistogramBackend::xAxis].size());
  size_t nRows(table.axis()[HistogramBackend::yAxis].size());

  for (size_t row=0; row < nRows; ++row)
    col.append(tableContents[row*nCols + _colIdx]);

  col.nbrOfFills()=1;
  col.lock.unlock();
  table.lock.unlock();
}







// ***  pp 75 clears a histogram ***

pp75::pp75(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key)
{
  loadSettings(0);
}

void pp75::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("PostProcessor");
  s.beginGroup(QString::fromStdString(_key));
  setupGeneral();
  _hist = setupDependency("HistName");
  bool ret (setupCondition());
  if (!(ret && _hist))
    return;
  _result = new Histogram0DFloat();
  createHistList(2*cass::NbrOfWorkers);
  Log::add(Log::INFO,"PostProcessor '" + _key +
           "' clears the histogram in pp '" + _hist->key() +
           "'. Condition is "+ _condition->key());
}

void cass::pp75::process(const cass::CASSEvent& evt)
{
  HistogramBackend& input(const_cast<HistogramBackend&>((*_hist)(evt)));
  _result->lock.lockForWrite();
  input.clear();
  _result->lock.unlock();
}




// ***  pp 76 quit program

pp76::pp76(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key)
{
  loadSettings(0);
}

void pp76::loadSettings(size_t)
{
  setupGeneral();
  if (!setupCondition())
    return;
  _result = new Histogram0DFloat();
  createHistList(2*cass::NbrOfWorkers);
  Log::add(Log::INFO,"PostProcessor '" + _key +
           "' will quit CASS.. Condition is "+ _condition->key());
}

void pp76::process(const cass::CASSEvent& /*evt*/)
{
  _result->lock.lockForWrite();
  InputBase::reference().end();
  _result->lock.unlock();
}











// ***  pp 77 checks ids

pp77::pp77(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key)
{
  loadSettings(0);
}

void cass::pp77::loadSettings(size_t)
{
  setupGeneral();
  if (!setupCondition())
    return;
  CASSSettings s;
  s.beginGroup("PostProcessor");
  s.beginGroup(QString::fromStdString(_key));
  _list.clear();
  string filename(s.value("List","").toString().toStdString());
  ifstream file(filename.c_str(),ios::in);
  if (!file.is_open())
    throw invalid_argument("pp77::loadSettings(): list file '" + filename +
                           "' could not be opened.");
  uint64_t id;
  while (file.good())
  {
    file >> id;
    _list.push_back(id);
  }
  _result = new Histogram0DFloat();
  createHistList(2*cass::NbrOfWorkers);
  Log::add(Log::INFO,"PostProcessor '" + _key +
           "' will check whether eventid in file '" + filename +
           "'. Condition is "+ _condition->key());
}

void cass::pp77::process(const cass::CASSEvent& evt)
{
  _result->lock.lockForWrite();
  *dynamic_cast<Histogram0DFloat*>(_result) =
      (find(_list.begin(),_list.end(),evt.id()) != _list.end());
  _result->nbrOfFills()=1;
  _result->lock.unlock();
}











// ***  pp 80 returns the number of fills of a given histogram ***

pp80::pp80(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key)
{
  loadSettings(0);
}

void pp80::loadSettings(size_t)
{
  setupGeneral();
  _pHist = setupDependency("HistName");
  bool ret (setupCondition());
  if (!(ret && _pHist))
    return;
  _result = new Histogram0DFloat();
  createHistList(2*cass::NbrOfWorkers,true);
  Log::add(Log::INFO,"PostProcessor '" + _key +
           "' returns the number of fills of histogram in pp '" +  _pHist->key() +
           "'. Condition on PostProcessor '" + _condition->key() + "'");
}

void pp80::process(const cass::CASSEvent& evt)
{
  const HistogramFloatBase& one
      (dynamic_cast<const HistogramFloatBase&>((*_pHist)(evt)));
  one.lock.lockForRead();
  _result->lock.lockForWrite();
  dynamic_cast<Histogram0DFloat*>(_result)->fill( one.nbrOfFills() );
  _result->nbrOfFills()=1;
  _result->lock.unlock();
  one.lock.unlock();
}












// ***  pp 81 returns the highest bin of a 1D Histogram ***

pp81::pp81(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key)
{
  loadSettings(0);
}

void pp81::loadSettings(size_t)
{
  setupGeneral();
  _pHist = setupDependency("HistName");
  bool ret (setupCondition());
  if (!(ret && _pHist))
    return;
  if (_pHist->getHist(0).dimension() != 1)
    throw runtime_error("PP type 81: HistName does not contain a 1D histogram.");
  _result = new Histogram0DFloat();
  createHistList(2*cass::NbrOfWorkers);
  Log::add(Log::INFO,"PostProcessor '" + _key +
           "' returns the maximum bin in '" + _pHist->key() +
           "' .Condition on postprocessor '" + _condition->key() + "'");
}

void pp81::process(const cass::CASSEvent& evt)
{
  const Histogram1DFloat& one
      (dynamic_cast<const Histogram1DFloat&>((*_pHist)(evt)));
  one.lock.lockForRead();
  _result->lock.lockForWrite();
  HistogramFloatBase::storage_t::const_iterator maxElementIt
      (max_element(one.memory().begin(), one.memory().end()-2));
  size_t bin(distance(one.memory().begin(),maxElementIt));
  dynamic_cast<Histogram0DFloat*>(_result)->
      fill(one.axis()[HistogramBackend::xAxis].position(bin));
  _result->nbrOfFills()=1;
  _result->lock.unlock();
  one.lock.unlock();
}
















// ***  pp 82 returns mean value of all bins in a Histogram ***

pp82::pp82(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key)
{
  loadSettings(0);
}

void pp82::loadSettings(size_t)
{
  setupGeneral();
  _pHist = setupDependency("HistName");
  bool ret (setupCondition());
  if (!(ret && _pHist))
    return;
  _result = new Histogram0DFloat();
  createHistList(2*cass::NbrOfWorkers);
  Log::add(Log::INFO,"PostProcessor '" + _key +
           "' returns the mean of all bins in '" + _pHist->key() +
           "' .Condition on postprocessor '" + _condition->key() + "'");
}

void pp82::process(const cass::CASSEvent& evt)
{
  const HistogramFloatBase &one
      (dynamic_cast<const HistogramFloatBase&>((*_pHist)(evt)));
  one.lock.lockForRead();
  _result->lock.lockForWrite();
  const float sum(accumulate(one.memory().begin(),one.memory().end(),0.));
  const float nElements(one.memory().size());
  dynamic_cast<Histogram0DFloat*>(_result)->fill(sum/nElements);
  _result->nbrOfFills()=1;
  _result->lock.unlock();
  one.lock.unlock();
}













// ***  pp 83 returns standard deviation of all bins in a Histogram ***

pp83::pp83(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key)
{
  loadSettings(0);
}

void pp83::loadSettings(size_t)
{
  setupGeneral();
  _pHist = setupDependency("HistName");
  bool ret (setupCondition());
  if (!(ret && _pHist))
    return;
  _result = new Histogram0DFloat();
  createHistList(2*cass::NbrOfWorkers);
  Log::add(Log::INFO,"PostProcessor '" + _key +
           "' returns the standart deviation of all bins in '" + _pHist->key() +
           "' .Condition on postprocessor '" + _condition->key() + "'");
}

void pp83::process(const cass::CASSEvent& evt)
{
  const HistogramFloatBase &one
      (dynamic_cast<const HistogramFloatBase&>((*_pHist)(evt)));
  one.lock.lockForRead();
  _result->lock.lockForWrite();
  HistogramFloatBase::storage_t::const_iterator element(one.memory().begin());
  HistogramFloatBase::storage_t::const_iterator end(one.memory().end());
  size_t accumulatedValues(0);
  float tmp_mean(0.);
  float tmp_variance(0.);
  for(; element != end ; ++element)
  {
    ++accumulatedValues;
    const float old_mean(tmp_mean);
    tmp_mean += ((*element - tmp_mean) / accumulatedValues);
    tmp_variance  += ((*element - old_mean)*(*element - tmp_mean));
  }
  if (tmp_variance < 0)
    throw logic_error("pp83::process:Variance is negative '" + toString(tmp_variance) +
                      "'. This is wrong");
  const float standartdeviation(sqrt(tmp_variance/(accumulatedValues-1)));

  dynamic_cast<Histogram0DFloat*>(_result)->fill(standartdeviation);
  _result->nbrOfFills()=1;
  _result->lock.unlock();
  one.lock.unlock();
}









// ***  pp 84 returns the sum of all bins in a Histogram ***

pp84::pp84(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key)
{
  loadSettings(0);
}

void pp84::loadSettings(size_t)
{
  setupGeneral();
  _pHist = setupDependency("HistName");
  bool ret (setupCondition());
  if (!(ret && _pHist))
    return;
  _result = new Histogram0DFloat();
  createHistList(2*cass::NbrOfWorkers);
  Log::add(Log::INFO,"PostProcessor '" + _key +
           "' returns the sum of all bins in '" + _pHist->key() +
           "' .Condition on postprocessor '" + _condition->key() + "'");
}

void pp84::process(const cass::CASSEvent& evt)
{
  using namespace std;
  const HistogramFloatBase &one
      (dynamic_cast<const HistogramFloatBase&>((*_pHist)(evt)));
  one.lock.lockForRead();
  _result->lock.lockForWrite();
  const float sum(accumulate(one.memory().begin(),one.memory().end(),0.));
  dynamic_cast<Histogram0DFloat*>(_result)->fill(sum);
  _result->nbrOfFills()=1;
  _result->lock.unlock();
  one.lock.unlock();
}



















// ***  pp 85 return full width at half maximum in given range of 1D histgoram ***

pp85::pp85(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key)
{
  loadSettings(0);
}

void pp85::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("PostProcessor");
  s.beginGroup(QString::fromStdString(_key));
  setupGeneral();
  _pHist = setupDependency("HistName");
  bool ret (setupCondition());
  if (!(ret && _pHist))
    return;
  _userXRange = make_pair(s.value("XLow",0).toFloat(),
                          s.value("XUp",1).toFloat());
  setupParameters(_pHist->getHist(0));
  _result = new Histogram0DFloat();
  createHistList(2*cass::NbrOfWorkers);
  Log::add(Log::INFO,"PostProcessor '" + _key +
           "' returns the full width at half maximum in '" + _pHist->key() +
           "' of the range from xlow '" + toString(_userXRange.first) +
           "' to xup '" + toString(_userXRange.second) +
           "' .Condition on postprocessor '" + _condition->key() + "'");
}

void pp85::histogramsChanged(const HistogramBackend* in)
{
  QWriteLocker lock(&_histLock);
  //return when there is no incomming histogram
  if(!in)
    return;
  //return when the incomming histogram is not a direct dependant
  if (find(_dependencies.begin(),_dependencies.end(),in->key()) == _dependencies.end())
    return;
  setupParameters(*in);
}

void pp85::setupParameters(const HistogramBackend &hist)
{
  if (hist.dimension() != 1)
    throw invalid_argument("pp85::setupParameters()'" + _key +
                           "': Error the histogram we depend on '" + hist.key() +
                           "' is not a 1D Histogram.");
  const AxisProperty &xaxis(hist.axis()[HistogramBackend::xAxis]);
  _xRange = make_pair(xaxis.bin(_userXRange.first),
                      xaxis.bin(_userXRange.second));
}

void pp85::process(const cass::CASSEvent& evt)
{
  const Histogram1DFloat& one
      (dynamic_cast<const Histogram1DFloat&>((*_pHist)(evt)));
  one.lock.lockForRead();
  _result->lock.lockForWrite();
  HistogramFloatBase::storage_t::const_iterator xRangeBegin
      (one.memory().begin()+_xRange.first);
  HistogramFloatBase::storage_t::const_iterator xRangeEnd
      (one.memory().begin()+_xRange.second);
  HistogramFloatBase::storage_t::const_iterator maxElementIt
      (max_element(xRangeBegin, xRangeEnd));
  HistogramFloatBase::storage_t::const_iterator minElementIt
      (min_element(xRangeBegin, xRangeEnd));
  const float halfMax((*maxElementIt+*minElementIt) * 0.5 );
  HistogramFloatBase::storage_t::const_iterator leftSide;
  HistogramFloatBase::storage_t::const_iterator rightSide;
  for(HistogramFloatBase::storage_t::const_iterator iVal(maxElementIt);
      (iVal != xRangeBegin) && (*iVal > halfMax);
      --iVal)
  {
    leftSide = iVal;
  }
  for(HistogramFloatBase::storage_t::const_iterator iVal(maxElementIt);
      (iVal != xRangeEnd) && (*iVal > halfMax);
      ++iVal)
  {
    rightSide = iVal;
  }
  const float lowerdist (one.axis()[HistogramBackend::xAxis].hist2user(distance(leftSide,rightSide)));
  const float upperdist (one.axis()[HistogramBackend::xAxis].hist2user(distance(leftSide-1,rightSide+1)));
  const float fwhm((upperdist+lowerdist)*0.5);
  dynamic_cast<Histogram0DFloat*>(_result)->fill(fwhm);
  _result->nbrOfFills()=1;
  _result->lock.unlock();
  one.lock.unlock();
}






















// ***  pp 86 return x position of a step in 1D histgoram ***

pp86::pp86(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key)
{
  loadSettings(0);
}

void pp86::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("PostProcessor");
  s.beginGroup(QString::fromStdString(_key));
  setupGeneral();
  _pHist = setupDependency("HistName");
  bool ret (setupCondition());
  if (!(ret && _pHist))
    return;
  _userXRangeStep = make_pair(s.value("XLow",0).toFloat(),
                              s.value("XUp",1).toFloat());
  _userXRangeBaseline = make_pair(s.value("BaselineLow",0).toFloat(),
                              s.value("BaselineUp",1).toFloat());
  _userFraction = s.value("Fraction",0.5).toFloat();
  setupParameters(_pHist->getHist(0));
  _result = new Histogram0DFloat();
  createHistList(2*cass::NbrOfWorkers);
  Log::add(Log::INFO, "PostProcessor '" + _key +
           "' returns the postion of the step in '" + _pHist->key() +
           "' in the range from xlow '" + toString(_userXRangeStep.first) +
           "' to xup '" + toString(_userXRangeStep.second) +
           "'. Where the baseline is defined in range '" + toString(_xRangeBaseline.first) +
           "' to '" + toString(_xRangeBaseline.second) + "'. The Fraction is '" +
           toString(_userFraction) + "' .Condition on postprocessor '" +
           _condition->key() + "'");
}

void pp86::histogramsChanged(const HistogramBackend* in)
{
  QWriteLocker lock(&_histLock);
  //return when there is no incomming histogram
  if(!in)
    return;
  //return when the incomming histogram is not a direct dependant
  if (find(_dependencies.begin(),_dependencies.end(),in->key()) == _dependencies.end())
    return;
  setupParameters(*in);
}

void pp86::setupParameters(const HistogramBackend &hist)
{
  if (hist.dimension() != 1)
    throw invalid_argument("pp86::setupParameters()'" + _key +
                           "': Error the histogram we depend on '" + hist.key() +
                           "' is not a 1D Histogram.");
  const AxisProperty &xaxis(hist.axis()[HistogramBackend::xAxis]);
  _xRangeStep = make_pair(xaxis.bin(_userXRangeStep.first),
                          xaxis.bin(_userXRangeStep.second));
  _xRangeBaseline = make_pair(xaxis.bin(_userXRangeBaseline.first),
                              xaxis.bin(_userXRangeBaseline.second));
}

void pp86::process(const cass::CASSEvent& evt)
{
  using namespace std;
  const Histogram1DFloat& one
      (dynamic_cast<const Histogram1DFloat&>((*_pHist)(evt)));
  one.lock.lockForRead();
  _result->lock.lockForWrite();

  HistogramFloatBase::storage_t::const_iterator baselineBegin
      (one.memory().begin()+_xRangeBaseline.first);
  HistogramFloatBase::storage_t::const_iterator baselineEnd
      (one.memory().begin()+_xRangeBaseline.second);
  const float baseline(accumulate(baselineBegin,baselineEnd,0.f) /
                       static_cast<float>(distance(baselineBegin,baselineEnd)));

  HistogramFloatBase::storage_t::const_iterator stepRangeBegin
      (one.memory().begin()+_xRangeStep.first);
  HistogramFloatBase::storage_t::const_iterator stepRangeEnd
      (one.memory().begin()+_xRangeStep.second);

  HistogramFloatBase::storage_t::const_iterator maxElementIt
      (max_element(stepRangeBegin, stepRangeEnd));
  const float halfMax((*maxElementIt+baseline) * _userFraction);

  HistogramFloatBase::storage_t::const_iterator stepIt(stepRangeBegin+1);
  for ( ; stepIt != maxElementIt ; stepIt++ )
    if ( *(stepIt-1) <= halfMax && halfMax < *stepIt )
      break;
  const size_t steppos(distance(one.memory().begin(),stepIt));

  dynamic_cast<Histogram0DFloat*>(_result)->fill(steppos);
  _result->nbrOfFills()=1;
  _result->lock.unlock();
  one.lock.unlock();
}
















// ***  pp 87 return center of mass in range of 1D histgoram ***

pp87::pp87(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key)
{
  loadSettings(0);
}

void pp87::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("PostProcessor");
  s.beginGroup(QString::fromStdString(_key));
  setupGeneral();
  _pHist = setupDependency("HistName");
  bool ret (setupCondition());
  if (!(ret && _pHist))
    return;
  _userXRange = make_pair(s.value("XLow",0).toFloat(),
                          s.value("XUp",1).toFloat());
  setupParameters(_pHist->getHist(0));
  _result = new Histogram0DFloat();
  createHistList(2*cass::NbrOfWorkers);
  Log::add(Log::INFO, "PostProcessor '" + _key +
           "' returns the center of mass in '" + _pHist->key() +
           "' in the range from xlow '" + toString(_userXRange.first) +
           "' to xup '" + toString(_userXRange.second) +
           "' .Condition on postprocessor '"+_condition->key()+"'");
}

void pp87::histogramsChanged(const HistogramBackend* in)
{
  QWriteLocker lock(&_histLock);
  //return when there is no incomming histogram
  if(!in)
    return;
  //return when the incomming histogram is not a direct dependant
  if (find(_dependencies.begin(),_dependencies.end(),in->key()) == _dependencies.end())
    return;
  setupParameters(*in);
}

void pp87::setupParameters(const HistogramBackend &hist)
{
  if (hist.dimension() != 1)
    throw invalid_argument("pp87::setupParameters()'" + _key +
                           "': Error the histogram we depend on '" + hist.key() +
                           "' is not a 1D Histogram.");
  const AxisProperty &xaxis(hist.axis()[HistogramBackend::xAxis]);
  _xRange = make_pair(xaxis.bin(_userXRange.first),
                      xaxis.bin(_userXRange.second));
}

void pp87::process(const cass::CASSEvent& evt)
{
  const Histogram1DFloat& hist
      (dynamic_cast<const Histogram1DFloat&>((*_pHist)(evt)));
  hist.lock.lockForRead();
  _result->lock.lockForWrite();

  const AxisProperty &xAxis(hist.axis()[HistogramBackend::xAxis]);
  const HistogramFloatBase::storage_t &data(hist.memory());

  float integral(0);
  float weight(0);
  for (size_t i(_xRange.first); i < _xRange.second; ++i)
  {
    integral += (data[i]);
    const float pos(xAxis.position(i));
    weight += (data[i]*pos);
  }
  const float com(weight/integral);

  dynamic_cast<Histogram0DFloat*>(_result)->fill(com);
  _result->nbrOfFills()=1;
  _result->lock.unlock();
  hist.lock.unlock();
}
