// Copyright (C) 2010-2011 Lutz Foucar
// (C) 2010 Thomas White - Updated to new PP framework

/** @file operations.cpp file contains definition of postprocessors that will
 *                       operate on histograms of other postprocessors
 * @author Lutz Foucar
 */

#include <QtCore/QString>
#include <iterator>
#include <algorithm>

#include "cass.h"
#include "operations.h"
#include "postprocessor.h"
#include "histogram.h"
#include "convenience_functions.h"
#include "cass_settings.h"


// ************ Postprocessor 4: Apply boolean NOT to 0D histogram *************

cass::pp4::pp4(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key)
{
  loadSettings(0);
}

void cass::pp4::loadSettings(size_t)
{
  using namespace std;
  setupGeneral();
  // Get input
  _one = setupDependency("HistName");
  bool ret (setupCondition());
  if (!(_one && ret)) return;

  // Set up output
  _result = new Histogram0DFloat();
  createHistList(2*cass::NbrOfWorkers);

  cout<<endl<< "PostProcessor '" << _key
      <<"' will apply NOT to PostProcessor '" << _one->key()
      <<"'. Condition is '"<<_condition->key()<<"'"
      <<endl;
}

void cass::pp4::process(const CASSEvent& evt)
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

cass::pp9::pp9(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key)
{
  loadSettings(0);
}

void cass::pp9::loadSettings(size_t)
{
  using namespace std;
  CASSSettings settings;

  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());

  // Get the range
  _range = make_pair(settings.value("LowerLimit",0).toFloat(),
                     settings.value("UpperLimit",0).toFloat());
  setupGeneral();

  // Get the input
  _one = setupDependency("HistName");
  bool ret (setupCondition());
  if (!(_one && ret)) return;

  // Create the output
  _result = new Histogram0DFloat();
  createHistList(2*cass::NbrOfWorkers);

  cout<<endl << "PostProcessor '" << _key
      <<"' will check whether hist in PostProcessor '" << _one->key()
      <<"' is between '" << _range.first
      <<"' and '" << _range.second
      <<"'. Condition is '"<<_condition->key()<<"'"
      <<endl;
}

void cass::pp9::process(const CASSEvent &evt)
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

  _result->lock.lockForWrite();
  *dynamic_cast<Histogram0DFloat*>(_result) =
      _range.first < value &&  value < _range.second;
  _result->nbrOfFills()=1;
  _result->lock.unlock();
}












// ****************** Postprocessor 40: Threshold histogram ********************

cass::pp40::pp40(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key)
{
  loadSettings(0);
}

void cass::pp40::loadSettings(size_t)
{
  using namespace std;
  CASSSettings settings;

  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());

  _threshold = settings.value("Threshold", 0.0).toFloat();
  setupGeneral();

  // Get input
  _one = setupDependency("HistName");
  bool ret (setupCondition());
  if (!(_one && ret)) return;

  _result = _one->getHist(0).clone();
  createHistList(2*cass::NbrOfWorkers);

  cout<<endl << "PostProcessor '" << _key
      <<"' will threshold Histogram in PostProcessor '" << _one->key()
      <<"' above '" << _threshold
      <<"'. Condition is '"<<_condition->key()<<"'"
      <<endl;
}

void cass::pp40::histogramsChanged(const HistogramBackend* in)
{
  using namespace std;
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
  VERBOSEOUT(cout<<"Postprocessor '"<<_key
             <<"': histograms changed => delete existing histo"
             <<" and create new one from input"<<endl);
}

void cass::pp40::process(const CASSEvent &evt)
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












// *** postprocessors 50 projects 2d hist to 1d histo for a selected region of the axis ***

cass::pp50::pp50(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key)
{
  loadSettings(0);
}

void cass::pp50::loadSettings(size_t)
{
  using namespace std;
  CASSSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  _userRange = make_pair(settings.value("LowerBound",-1e6).toFloat(),
                         settings.value("UpperBound", 1e6).toFloat());
  _axis = static_cast<HistogramBackend::Axis>(settings.value("Axis",HistogramBackend::xAxis).toUInt());
  _normalize = settings.value("Normalize",false).toBool();
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
    stringstream ss;
    ss << "pp50::loadSettings(): requested _axis '"<<_axis<<"' does not exist.";
    throw invalid_argument(ss.str());
    break;
  }
  setupParameters(_pHist->getHist(0));
  cout<<endl << "PostProcessor '"<<_key
      <<"' will project histogram of PostProcessor '"<<_pHist->key()
      <<"' from '"<<_range.first
      <<"' to '"<<_range.second
      <<"' on axis '"<<_axis
      <<boolalpha<<"'. Normalize '"<<_normalize
      <<"'. Condition is '"<<_condition->key()<<"'"
      <<endl;
}

void cass::pp50::histogramsChanged(const HistogramBackend* in)
{
  using namespace std;
  QWriteLocker lock(&_histLock);
  //return when there is no incomming histogram
  if(!in)
    return;
  //return when the incomming histogram is not a direct dependant
  if (find(_dependencies.begin(),_dependencies.end(),in->key()) == _dependencies.end())
    return;
  setupParameters(*in);
}

void cass::pp50::setupParameters(const HistogramBackend &hist)
{
  using namespace std;
  if (hist.dimension() != 2)
  {
    stringstream ss;
    ss <<"pp50::setupParameters()'"<<_key<<"': Error the histogram we depend on '"<<hist.key()
        <<"' is not a 2D Histogram.";
    throw invalid_argument(ss.str());
  }
  const AxisProperty &projAxis(hist.axis()[_axis]);
  const AxisProperty &otherAxis(hist.axis()[_otherAxis]);
  _range = make_pair(max(_userRange.first, otherAxis.lowerLimit()),
                     min(_range.second, otherAxis.upperLimit()));
  _result = new Histogram1DFloat(projAxis.nbrBins(), projAxis.lowerLimit(), projAxis.upperLimit());
  createHistList(2*cass::NbrOfWorkers);
}

void cass::pp50::process(const CASSEvent& evt)
{
  using namespace std;
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

cass::pp51::pp51(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key)
{
  loadSettings(0);
}

void cass::pp51::loadSettings(size_t)
{
  using namespace std;
  CASSSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  _area = make_pair(settings.value("LowerBound",-1e6).toFloat(),
                    settings.value("UpperBound", 1e6).toFloat());
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
  cout<<endl << "PostProcessor '"<<_key
      <<"' will create integral of 1d histogram in PostProcessor '"<<_pHist->key()
      <<"' from '"<<_area.first
      <<"' to '"<<_area.second
      <<"'. Condition is '"<<_condition->key()
      <<endl;
}

void cass::pp51::process(const CASSEvent& evt)
{
  using namespace std;
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

cass::pp52::pp52(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key)
{
  loadSettings(0);
}

void cass::pp52::loadSettings(size_t)
{
  using namespace std;
  CASSSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  setupGeneral();
  _pHist = setupDependency("HistName");
  bool ret (setupCondition());
  if (!(ret && _pHist))
    return;
  const Histogram2DFloat &one
      (dynamic_cast<const Histogram2DFloat&>(_pHist->getHist(0)));
  const float center_x_user (settings.value("XCenter",512).toFloat());
  const float center_y_user (settings.value("YCenter",512).toFloat());
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
  std::cout << "PostProcessor "<<_key
      <<": will calculate the radial average of histogram of PostProcessor "<<_pHist->key()
      <<" with xcenter "<<settings.value("XCenter",512).toFloat()
      <<" ycenter "<<settings.value("YCenter",512).toFloat()
      <<" in histogram coordinates xcenter "<<_center.first
      <<" ycenter "<<_center.second
      <<" maximum radius calculated from the incoming histogram "<<_radius
      <<". Condition is "<<_condition->key()
      <<std::endl;
}

void cass::pp52::process(const CASSEvent& evt)
{
  using namespace std;
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

cass::pp53::pp53(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key)
{
  loadSettings(0);
}

void cass::pp53::loadSettings(size_t)
{
  using namespace std;
  CASSSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  _pHist = setupDependency("HistName");
  setupGeneral();
  bool ret (setupCondition());
  if (!(ret && _pHist))
    return;
  const Histogram2DFloat &one
      (dynamic_cast<const Histogram2DFloat&>(_pHist->getHist(0)));
  const float center_x_user (settings.value("XCenter",512).toFloat());
  const float center_y_user (settings.value("YCenter",512).toFloat());
  _nbrBins = settings.value("NbrBins",360).toUInt();
  _center = make_pair(one.axis()[HistogramBackend::xAxis].bin(center_x_user),
                      one.axis()[HistogramBackend::yAxis].bin(center_y_user));
  const size_t dist_center_x_right
      (one.axis()[HistogramBackend::xAxis].nbrBins()-_center.first);
  const size_t dist_center_y_top
      (one.axis()[HistogramBackend::yAxis].nbrBins()-_center.second);
  const size_t min_dist_x (min(dist_center_x_right, _center.first));
  const size_t min_dist_y (min(dist_center_y_top, _center.second));
  const size_t max_radius (min(min_dist_x, min_dist_y));
  const float minrad_user(settings.value("MinRadius",0.).toFloat());
  const float maxrad_user(settings.value("MaxRadius",0.).toFloat());
  const size_t minrad (one.axis()[HistogramBackend::xAxis].user2hist(minrad_user));
  const size_t maxrad (one.axis()[HistogramBackend::xAxis].user2hist(maxrad_user));
  _range = make_pair(min(max_radius, minrad),
                     min(max_radius, maxrad));
  _result = new Histogram1DFloat(_nbrBins,0,360);
  createHistList(2*cass::NbrOfWorkers);
  std::cout << "PostProcessor "<<_key
      <<": angular distribution of hist "<<_pHist->key()
      <<" with xcenter "<<settings.value("XCenter",512).toFloat()
      <<" ycenter "<<settings.value("YCenter",512).toFloat()
      <<" in histogram coordinates xcenter "<<_center.first
      <<" ycenter "<<_center.second
      <<" minimum radius "<<settings.value("MinRadius",0.).toFloat()
      <<" maximum radius "<<settings.value("MaxRadius",512.).toFloat()
      <<" in histogram coordinates minimum radius "<<_range.first
      <<" maximum radius "<<_range.second
      <<" Histogram has "<<_nbrBins<<" Bins"
      <<". Condition is "<<_condition->key()
      <<std::endl;
}

void cass::pp53::process(const CASSEvent& evt)
{
  using namespace std;
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

cass::pp54::pp54(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key)
{
  loadSettings(0);
}

void cass::pp54::loadSettings(size_t)
{
  using namespace std;
  CASSSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  setupGeneral();
  _pHist = setupDependency("HistName");
  bool ret (setupCondition());
  if (!(ret && _pHist))
    return;
  const Histogram2DFloat &one
      (dynamic_cast<const Histogram2DFloat&>(_pHist->getHist(0)));
  const float center_x_user (settings.value("XCenter",512).toFloat());
  const float center_y_user (settings.value("YCenter",512).toFloat());
  _nbrBins = settings.value("NbrBins",360).toUInt();
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
  std::cout << "PostProcessor "<<_key
      <<": angular distribution with xcenter "<<settings.value("XCenter",512).toFloat()
      <<" ycenter "<<settings.value("YCenter",512).toFloat()
      <<" in histogram coordinates xcenter "<<_center.first
      <<" ycenter "<<_center.second
      <<" Histogram has "<<_nbrBins<<" Bins"
      <<" the maximum Radius in histogram coordinates "<<_radius
      <<". Condition is "<<_condition->key()
      <<std::endl;
}

void cass::pp54::process(const CASSEvent& evt)
{
  using namespace std;
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









// *** postprocessor 60 histograms 0D values ***

cass::pp60::pp60(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key)
{
  loadSettings(0);
}

void cass::pp60::loadSettings(size_t)
{
  using namespace std;
  setupGeneral();
  _pHist = setupDependency("HistName");
  bool ret (setupCondition());
  if (!(ret && _pHist))
    return;
  set1DHist(_result,_key);
  createHistList(2*cass::NbrOfWorkers,true);
  cout<<endl<<"Postprocessor '"<<_key
      <<"' histograms values from PostProcessor '"<< _pHist->key()
      <<"'. Condition on PostProcessor '"<<_condition->key()<<"'"
      <<endl;
}

void cass::pp60::process(const CASSEvent& evt)
{
  using namespace std;
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

cass::pp61::pp61(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key)
{
  loadSettings(0);
}

void cass::pp61::loadSettings(size_t)
{
  using namespace std;
  CASSSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  setupGeneral();
  unsigned average = settings.value("NbrOfAverages", 1).toUInt();
  _alpha =  average ? 2./static_cast<float>(average+1.) : 0.;
  _pHist = setupDependency("HistName");
  bool ret (setupCondition());
  if (!(ret && _pHist))
    return;
  const HistogramBackend &one(_pHist->getHist(0));
  _result = one.clone();
  createHistList(2*cass::NbrOfWorkers,true);
  cout<<endl<<"Postprocessor '"<<_key
      <<"' averages histograms from PostProcessor '"<< _pHist->key()
      <<"' alpha for the averaging '"<<_alpha
      <<"'. Condition on postprocessor '"<<_condition->key()<<"'"
      <<endl;
}

void cass::pp61::histogramsChanged(const HistogramBackend* in)
{
  using namespace std;
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
  VERBOSEOUT(cout<<"Postprocessor '"<<_key
             <<"': histograms changed => delete existing histo"
             <<" and create new one from input"<<endl);
}

void cass::pp61::process(const CASSEvent& evt)
{
  using namespace std;
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

cass::pp62::pp62(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key)
{
  loadSettings(0);
}

void cass::pp62::loadSettings(size_t)
{
  using namespace std;
  setupGeneral();
  _pHist = setupDependency("HistName");
  bool ret (setupCondition());
  if (!(ret && _pHist))
    return;
  const HistogramBackend &one(_pHist->getHist(0));
  _result = one.clone();
  createHistList(2*cass::NbrOfWorkers,true);
  cout<<endl<<"Postprocessor '"<<_key
      <<"' sums up histograms from PostProcessor '"<< _pHist->key()
      <<"'. Condition on postprocessor '"<<_condition->key()<<"'"
      <<endl;
}

void cass::pp62::histogramsChanged(const HistogramBackend* in)
{
  using namespace std;
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
  VERBOSEOUT(cout<<"Postprocessor '"<<_key
             <<"': histograms changed => delete existing histo"
             <<" and create new one from input"<<endl);
}

void cass::pp62::process(const CASSEvent& evt)
{
  using namespace std;
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

cass::pp63::pp63(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key), _num_seen_evt(0), _when_first_evt(0), _first_fiducials(0)
{
  loadSettings(0);
}

void cass::pp63::loadSettings(size_t)
{
  using namespace std;
  CASSSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  const size_t min_time_user (settings.value("MinTime",0).toUInt());
  const size_t max_time_user (settings.value("MaxTime",300).toUInt());
  _timerange = make_pair(min_time_user,max_time_user);
  _nbrSamples=settings.value("NumberOfSamples",5).toUInt();
  setupGeneral();
  _pHist = setupDependency("HistName");
  bool ret (setupCondition());
  if (!(ret && _pHist))
    return;
  const HistogramBackend &one(_pHist->getHist(0));
  _result = one.clone();
  createHistList(2*cass::NbrOfWorkers,true);
  cout<<endl<< "PostProcessor '"<<_key
      <<"' will calculate the time average of histogram of PostProcessor '"<<_pHist->key()
      <<"' from now '"<<settings.value("MinTime",0).toUInt()
      <<"' to '"<<settings.value("MaxTime",300).toUInt()
      <<"' seconds '"<<_timerange.first
      <<"' ; '"<<_timerange.second
      <<"' each bin is equivalent to up to '"<< _nbrSamples
      <<"' measurements,"
      <<" condition on postprocessor '"<<_condition->key()<<"'"
      <<std::endl;
}

void cass::pp63::process(const cass::CASSEvent& evt)
{
  using namespace std;

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

cass::pp64::pp64(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key)
{
  loadSettings(0);
}

void cass::pp64::loadSettings(size_t)
{
  using namespace std;
  CASSSettings settings;

  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());

  _one = setupDependency("HistName");

  bool ret (setupCondition());
  if ( !(_one && ret) ) return;

  setupGeneral();
  _size = settings.value("Size", 10000).toUInt();

  _result = new Histogram1DFloat(_size, 0, _size-1,"Shots");
  createHistList(2*cass::NbrOfWorkers,true);

  cout<<endl<< "PostProcessor '" << _key
      <<"' will make a history of 0d histogram in pp '"<< _one->key()
      <<", size of history '" << _size
      <<"' Condition on postprocessor '" << _condition->key()<<"'"
      <<endl;
}

void cass::pp64::process(const cass::CASSEvent &evt)
{
  const Histogram0DFloat &one
      (dynamic_cast<const Histogram0DFloat &>((*_one)(evt)));
  one.lock.lockForRead();
  _result->lock.lockForWrite();
  ++_result->nbrOfFills();
  HistogramFloatBase::storage_t &mem((dynamic_cast<Histogram1DFloat *>(_result))->memory());
  std::rotate(mem.begin(), mem.begin()+1, mem.end());
  mem[_size-1] = one.getValue();
  _result->lock.unlock();
  one.lock.unlock();
}







// *** postprocessor 65 histograms 2 0D values to 2D histogram ***

cass::pp65::pp65(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key)
{
  loadSettings(0);
}

void cass::pp65::loadSettings(size_t)
{
  using namespace std;
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
  std::cout<<"Postprocessor '"<<_key
      <<"': histograms values from PostProcessor '"<< _one->key()
      <<"' and '"<< _two->key()
      <<". condition on PostProcessor '"<<_condition->key()<<"'"
      <<std::endl;
}

void cass::pp65::process(const CASSEvent& evt)
{
  using namespace std;
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

cass::pp66::pp66(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key)
{
  loadSettings(0);
}

void cass::pp66::loadSettings(size_t)
{
  using namespace std;
  setupGeneral();
  _one = setupDependency("HistOne");
  _two = setupDependency("HistTwo");
  bool ret (setupCondition());
  if ( !(_one && _two && ret) )
    return;
  const HistogramFloatBase &one(dynamic_cast<const HistogramFloatBase&>(_one->getHist(0)));
  const HistogramFloatBase &two(dynamic_cast<const HistogramFloatBase&>(_two->getHist(0)));
  if (one.dimension() != 1 || two.dimension() != 1)
  {
    stringstream ss;
    ss << "pp66::loadSettings(): HistOne '"<<one.key()
        <<"' with dimension '"<< one.dimension()
        <<"' or HistTwo '"<<two.key()
        <<"' has dimension '"<< two.dimension()
        <<"' does not have dimension 1";
    throw runtime_error(ss.str());
  }
  _result = new Histogram2DFloat(one.axis()[HistogramBackend::xAxis].nbrBins(),
                                 one.axis()[HistogramBackend::xAxis].lowerLimit(),
                                 one.axis()[HistogramBackend::xAxis].upperLimit(),
                                 two.axis()[HistogramBackend::xAxis].nbrBins(),
                                 two.axis()[HistogramBackend::xAxis].lowerLimit(),
                                 two.axis()[HistogramBackend::xAxis].upperLimit(),
                                 one.axis()[HistogramBackend::xAxis].title(),
                                 two.axis()[HistogramBackend::xAxis].title());
  createHistList(2*cass::NbrOfWorkers);
  cout<<endl<<"Postprocessor '"<<_key
      <<"' creates a two dim histogram from PostProcessor '"<< _one->key()
      <<"' and '"<< _two->key()
      <<"'. condition on PostProcessor '"<<_condition->key()<<"'"
      <<endl;
}

void cass::pp66::histogramsChanged(const HistogramBackend* in)
{
  using namespace std;
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
    stringstream ss;
    ss<<"pp66::histogramsChanged()'"<<_key
        <<"': Error'"<<*itDep
        <<"' is neither hist one '"<<one.key()
        <<"' nor hist two '"<<two.key()<<"'";
    throw logic_error(ss.str());
  }
  createHistList(2*cass::NbrOfWorkers);
  //notify all pp that depend on us that our histograms have changed
  PostProcessors::keyList_t dependands (_pp.find_dependant(_key));
  PostProcessors::keyList_t::iterator it (dependands.begin());
  for (; it != dependands.end(); ++it)
    _pp.getPostProcessor(*it).histogramsChanged(_result);
  VERBOSEOUT(cout<<"Postprocessor '"<<_key
             <<"': histograms changed => delete existing histo"
             <<" and create new one from input"<<endl);
}

void cass::pp66::process(const CASSEvent& evt)
{
  using namespace std;
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
//  cout <<"one "<< oneNBins<<" "<<one.memory().size()<<endl;
//  cout <<"two "<< twoNBins<<" "<<two.memory().size()<<endl;
//  cout <<"result "<< memory.size()<<endl;
  for (size_t j(0); j < twoNBins; ++j)
    for (size_t i(0); i < oneNBins; ++i)
      memory[i*oneNBins+j] = one.memory()[i]*two.memory()[j];
  _result->nbrOfFills()=1;
  _result->lock.unlock();
  two.lock.unlock();
  one.lock.unlock();
}















// *** postprocessor 67 histograms 2 0D values to 1D histogram ***

cass::pp67::pp67(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key)
{
  loadSettings(0);
}

void cass::pp67::loadSettings(size_t)
{
  using namespace std;
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
  std::cout<<"Postprocessor '"<<_key
      <<"' makes a 1D Histogram where '"<< _one->key()
      <<"' defines the x bin to fill and '"<< _two->key()
      <<"' defines the weight of how much to fill the x bin"
      <<". Condition on PostProcessor '"<<_condition->key()<<"'"
      <<std::endl;
}

void cass::pp67::process(const CASSEvent& evt)
{
  using namespace std;
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

cass::pp68::pp68(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key)
{
  loadSettings(0);
}

void cass::pp68::loadSettings(size_t)
{
  using namespace std;
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
  cout<<endl<<"Postprocessor '"<<_key
      <<"' makes a 2D Histogram where '"<< _one->key()
      <<"' defines the x axis to fill and '"<< _two->key()
      <<"' defines the y axis bin"
      <<". Condition on PostProcessor '"<<_condition->key()<<"'"
      <<endl;
}

void cass::pp68::setup(const Histogram1DFloat &one)
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

void cass::pp68::histogramsChanged(const HistogramBackend* in)
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

void cass::pp68::process(const CASSEvent& evt)
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














// ***  pp 70 subsets a histogram ***

cass::pp70::pp70(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key)
{
  loadSettings(0);
}

void cass::pp70::loadSettings(size_t)
{
  using namespace std;
  CASSSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  setupGeneral();
  _pHist = setupDependency("HistName");
  bool ret (setupCondition());
  if (!(ret && _pHist))
    return;
  const HistogramFloatBase &one
      (dynamic_cast<const HistogramFloatBase&>(_pHist->getHist(0)));
  if (0 == one.dimension())
    throw runtime_error("pp70::loadSettings(): Unknown dimension of incomming histogram");
  _userXRange = make_pair(settings.value("XLow",0).toFloat(),
                          settings.value("XUp",1).toFloat());
  const AxisProperty &xaxis(one.axis()[HistogramBackend::xAxis]);
  _inputOffset=(xaxis.bin(_userXRange.first));
  const size_t binXUp (xaxis.bin(_userXRange.second));
  const size_t nXBins (binXUp-_inputOffset);
  const float xLow (xaxis.position(_inputOffset));
  const float xUp (xaxis.position(binXUp));
  cout<<endl<<"PostProcessor '"<<_key
      <<"' returns a subset of histogram in pp '" << _pHist->key()
      <<"' which has dimension '"<<one.dimension()
      <<"'. Subset is xLow:"<<xLow<<"("<<_inputOffset<<")"
      <<", xUp:"<<xUp<<"("<<binXUp<<")"
      <<", xNbrBins:"<<nXBins;
  if (1 == one.dimension())
  {
    _result = new Histogram1DFloat(nXBins,xLow,xUp);
  }
  else if (2 == one.dimension())
  {
    _userYRange = make_pair(settings.value("YLow",0).toFloat(),
                            settings.value("YUp",1).toFloat());
    const AxisProperty &yaxis (one.axis()[HistogramBackend::yAxis]);
    const size_t binYLow(yaxis.bin(_userYRange.first));
    const size_t binYUp (yaxis.bin(_userYRange.second));
    const size_t nYBins = (binYUp - binYLow);
    const float yLow (yaxis.position(binYLow));
    const float yUp (yaxis.position(binYUp));
    _inputOffset = static_cast<size_t>(binYLow*xaxis.nbrBins()+xLow + 0.1);
    _result = new Histogram2DFloat(nXBins,xLow,xUp,
                                   nYBins,yLow,yUp);
    cout<<", yLow:"<<yLow<<"("<<binYLow<<")"
        <<", yUp:"<<yUp<<"("<<binYLow<<")"
        <<", yNbrBins:"<<nXBins
        <<", linearized offset is now:"<<_inputOffset;
  }
  cout <<". Condition on postprocessor '"<<_condition->key()<<"'"
      <<endl;
  createHistList(2*cass::NbrOfWorkers);
}

void cass::pp70::histogramsChanged(const HistogramBackend* in)
{
  using namespace std;
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
  cout<<endl<<"PostProcessor '"<<_key
      <<"' histogramsChanged: returns a subset of histogram in pp '" << _pHist->key()
      <<"' which has dimension '"<<in->dimension()
      <<"'. Subset is xLow:"<<xLow<<"("<<_inputOffset<<")"
      <<", xUp:"<<xUp<<"("<<binXUp<<")"
      <<", xNbrBins:"<<nXBins;
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
    cout<<", yLow:"<<yLow<<"("<<binYLow<<")"
        <<", yUp:"<<yUp<<"("<<binYLow<<")"
        <<", yNbrBins:"<<nXBins
        <<", linearized offset is now:"<<_inputOffset;
  }
  cout <<". Condition on postprocessor '"<<_condition->key()<<"'"
      <<endl;
  createHistList(2*cass::NbrOfWorkers);
  //notify all pp that depend on us that our histograms have changed
  PostProcessors::keyList_t dependands (_pp.find_dependant(_key));
  PostProcessors::keyList_t::iterator it (dependands.begin());
  for (; it != dependands.end(); ++it)
    _pp.getPostProcessor(*it).histogramsChanged(_result);
}


void cass::pp70::process(const cass::CASSEvent& evt)
{
  using namespace std;
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







// ***  pp 80 returns the number of fills of a given histogram ***

cass::pp80::pp80(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key)
{
  loadSettings(0);
}

void cass::pp80::loadSettings(size_t)
{
  using namespace std;
  setupGeneral();
  _pHist = setupDependency("HistName");
  bool ret (setupCondition());
  if (!(ret && _pHist))
    return;
  _result = new Histogram0DFloat();
  createHistList(2*cass::NbrOfWorkers,true);
  cout << "PostProcessor "<<_key
      <<": returns the number of fills of histogram in pp " << _pHist->key()
      <<" condition on postprocessor:"<<_condition->key()
      <<endl;
}

void cass::pp80::process(const cass::CASSEvent& evt)
{
  using namespace std;
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

cass::pp81::pp81(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key)
{
  loadSettings(0);
}

void cass::pp81::loadSettings(size_t)
{
  using namespace std;
  setupGeneral();
  _pHist = setupDependency("HistName");
  bool ret (setupCondition());
  if (!(ret && _pHist))
    return;
  if (_pHist->getHist(0).dimension() != 1)
    throw runtime_error("PP type 81: HistName does not contain a 1D histogram.");
  _result = new Histogram0DFloat();
  createHistList(2*cass::NbrOfWorkers);
  cout<<endl<< "PostProcessor '"<<_key
      <<"' returns the maximum bin in '" << _pHist->key()
      <<"' .Condition on postprocessor '"<<_condition->key()<<"'"
      <<endl;
}

void cass::pp81::process(const cass::CASSEvent& evt)
{
  using namespace std;
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



















// ***  pp 85 return full width at half maximum in given range of 1D histgoram ***

cass::pp85::pp85(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key)
{
  loadSettings(0);
}

void cass::pp85::loadSettings(size_t)
{
  using namespace std;
  CASSSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(QString::fromStdString(_key));
  setupGeneral();
  _pHist = setupDependency("HistName");
  bool ret (setupCondition());
  if (!(ret && _pHist))
    return;
  _userXRange = make_pair(settings.value("XLow",0).toFloat(),
                          settings.value("XUp",1).toFloat());
  setupParameters(_pHist->getHist(0));
  _result = new Histogram0DFloat();
  createHistList(2*cass::NbrOfWorkers);
  cout<<endl<< "PostProcessor '"<<_key
      <<"' returns the full width at half maximum in '" << _pHist->key()
      <<"' for the xlow '"<< _userXRange.first
      <<"' and xup '"<< _userXRange.second
      <<"' .Condition on postprocessor '"<<_condition->key()<<"'"
      <<endl;
}

void cass::pp85::histogramsChanged(const HistogramBackend* in)
{
  using namespace std;
  QWriteLocker lock(&_histLock);
  //return when there is no incomming histogram
  if(!in)
    return;
  //return when the incomming histogram is not a direct dependant
  if (find(_dependencies.begin(),_dependencies.end(),in->key()) == _dependencies.end())
    return;
  setupParameters(*in);
}

void cass::pp85::setupParameters(const HistogramBackend &hist)
{
  using namespace std;
  if (hist.dimension() != 1)
  {
    stringstream ss;
    ss <<"pp85::setupParameters()'"<<_key<<"': Error the histogram we depend on '"<<hist.key()
        <<"' is not a 1D Histogram.";
    throw invalid_argument(ss.str());
  }
  const AxisProperty &xaxis(hist.axis()[HistogramBackend::xAxis]);
  _xRange = make_pair(xaxis.bin(_userXRange.first),
                      xaxis.bin(_userXRange.second));
}

void cass::pp85::process(const cass::CASSEvent& evt)
{
  using namespace std;
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





















// Local Variables:
// coding: utf-8
// mode: C++
// c-file-style: "gnu"
// c-file-offsets: ((c . 0) (innamespace . 0))
// fill-column: 100
// End:
