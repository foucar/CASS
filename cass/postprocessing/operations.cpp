// Copyright (C) 2010 Lutz Foucar
// (C) 2010 Thomas White - Updated to new PP framework

#include <QtCore/QSettings>
#include <QtCore/QString>

#include "cass.h"
#include "operations.h"
#include "postprocessor.h"
#include "histogram.h"
#include "convenience_functions.h"



// ************ Postprocessor 4: Apply boolean NOT to 0D histogram *************
cass::pp4::pp4(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key)
{
  loadSettings(0);
}

void cass::pp4::loadSettings(size_t)
{
  QSettings settings;

  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());

  // Get input
  _one = setupDependency("HistName");
  bool ret (setupCondition());
  if (!(_one && ret)) return;

  // Set up output
  _result = new Histogram0DFloat();
  createHistList(2*cass::NbrOfWorkers);

  std::cout << "PostProcessor " << _key
      << ": will apply NOT to PostProcessor " << _one->key()
      <<". Condition is"<<_condition->key()
      << std::endl;
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
  QSettings settings;

  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());

  // Get the range
  _range = std::make_pair(settings.value("LowerLimit",0).toFloat(),
                          settings.value("UpperLimit",0).toFloat());

  // Get the input
  _one = setupDependency("HistName");
  bool ret (setupCondition());
  if (!(_one && ret)) return;

  // Create the output
  _result = new Histogram0DFloat();
  createHistList(2*cass::NbrOfWorkers);

  std::cout << "PostProcessor " << _key
      << ": will check whether hist in PostProcessor " << _one->key()
      << " is between " << _range.first
      << " and " << _range.second
      <<". Condition is "<<_condition->key()
      << std::endl;
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
  QSettings settings;

  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());

  _threshold = settings.value("Threshold", 0.0).toFloat();

  // Get input
  _one = setupDependency("HistName");
  bool ret (setupCondition());
  if (!(_one && ret)) return;

  _result = _one->getHist(0).clone();
  createHistList(2*cass::NbrOfWorkers);

  std::cout << "PostProcessor " << _key
      << ": will threshold Histogram in PostProcessor " << _one->key()
      << " above " << _threshold
      <<". Condition is"<<_condition->key()
      << std::endl;
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
  QSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  _range = make_pair(settings.value("LowerBound",-1e6).toFloat(),
                     settings.value("UpperBound", 1e6).toFloat());
  _axis = settings.value("Axis",HistogramBackend::xAxis).toUInt();
  _normalize = settings.value("Normalize",false).toBool();
  _pHist = setupDependency("HistName");
  bool ret (setupCondition());
  if (!(ret && _pHist))
    return;
  const Histogram2DFloat &one
      (dynamic_cast<const Histogram2DFloat&>(_pHist->getHist(0)));
  switch (_axis)
  {
  case (HistogramBackend::xAxis):
    _range.first  = max(_range.first, one.axis()[HistogramBackend::yAxis].lowerLimit());
    _range.second = min(_range.second,one.axis()[HistogramBackend::yAxis].upperLimit());
    _result = new Histogram1DFloat(one.axis()[HistogramBackend::xAxis].nbrBins(),
                                   one.axis()[HistogramBackend::xAxis].lowerLimit(),
                                   one.axis()[HistogramBackend::xAxis].upperLimit());
    break;
  case (HistogramBackend::yAxis):
    _range.first  = max(_range.first, one.axis()[HistogramBackend::xAxis].lowerLimit());
    _range.second = min(_range.second,one.axis()[HistogramBackend::xAxis].upperLimit());
    _result = new Histogram1DFloat(one.axis()[HistogramBackend::yAxis].nbrBins(),
                                   one.axis()[HistogramBackend::yAxis].lowerLimit(),
                                   one.axis()[HistogramBackend::yAxis].upperLimit());
    break;
  }
  createHistList(2*cass::NbrOfWorkers);
  std::cout << "PostProcessor "<<_key
      <<" will project histogram of PostProcessor "<<_pHist->key()
      <<" from "<<_range.first
      <<" to "<<_range.second
      <<" on axis "<<_axis
      <<boolalpha<<" normalize "<<_normalize
      <<". Condition is"<<_condition->key()
      <<std::endl;
}

void cass::pp50::process(const CASSEvent& evt)
{
  using namespace std;
  const Histogram2DFloat &one
      (dynamic_cast<const Histogram2DFloat&>((*_pHist)(evt)));
  one.lock.lockForRead();
  _result->lock.lockForWrite();
  *dynamic_cast<Histogram1DFloat*>(_result) = one.project(_range,static_cast<HistogramBackend::Axis>(_axis));
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
  QSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  _area = make_pair(settings.value("LowerBound",-1e6).toFloat(),
                    settings.value("UpperBound", 1e6).toFloat());
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
  std::cout << "PostProcessor "<<_key
      <<": will create integral of 1d histogram in PostProcessor "<<_pHist->key()
      <<" from "<<_area.first
      <<" to "<<_area.second
      <<". Condition is"<<_condition->key()
      <<std::endl;
}

void cass::pp51::process(const CASSEvent& evt)
{
  using namespace std;
  const Histogram1DFloat &one
      (dynamic_cast<const Histogram1DFloat&>((*_pHist)(evt)));
  one.lock.lockForRead();
  _result->lock.lockForWrite();
  *dynamic_cast<Histogram0DFloat*>(_result) = one.integral(_area);
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
  QSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
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
      <<". Condition is"<<_condition->key()
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
  QSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
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
      <<". Condition is"<<_condition->key()
      <<std::endl;
}

void cass::pp53::process(const CASSEvent& evt)
{
  using namespace std;
  //retrieve the memory of the to be subtracted histograms//
  const Histogram2DFloat &one
      (dynamic_cast<const Histogram2DFloat&>((*_pHist)(evt)));
  // retrieve the projection from the 2d hist//
  one.lock.lockForRead();
  _result->lock.lockForWrite();
  *dynamic_cast<Histogram1DFloat*>(_result) = one.radar_plot(_center,_range, _nbrBins);
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
  QSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
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
  QSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  _pHist = setupDependency("HistName");
  bool ret (setupCondition());
  if (!(ret && _pHist))
    return;
  set1DHist(_result,_key);
  createHistList(2*cass::NbrOfWorkers,true);
  std::cout<<"Postprocessor "<<_key
      <<": histograms values from PostProcessor "<< _pHist->key()
      <<" condition on PostProcessor "<<_condition->key()
      <<std::endl;
}

void cass::pp60::process(const CASSEvent& evt)
{
  using namespace std;
  const Histogram0DFloat &one
      (dynamic_cast<const Histogram0DFloat&>((*_pHist)(evt)));
  one.lock.lockForRead();
  _result->lock.lockForWrite();
  dynamic_cast<Histogram1DFloat*>(_result)->fill(one.getValue());
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
  QSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  unsigned average = settings.value("average", 1).toUInt();
  _alpha =  average ? 2./static_cast<float>(average+1.) : 0.;
  _pHist = setupDependency("HistName");
  bool ret (setupCondition());
  if (!(ret && _pHist))
    return;
  const HistogramBackend &one(_pHist->getHist(0));
  _result = one.clone();
  createHistList(2*cass::NbrOfWorkers,true);
  std::cout<<"Postprocessor "<<_key
      <<": averages histograms from PostProcessor "<< _pHist->key()
      <<" alpha for the averaging:"<<_alpha
      <<" condition on postprocessor:"<<_condition->key()
      <<std::endl;
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
  QSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  _pHist = setupDependency("HistName");
  bool ret (setupCondition());
  if (!(ret && _pHist))
    return;
  const HistogramBackend &one(_pHist->getHist(0));
  _result = one.clone();
  createHistList(2*cass::NbrOfWorkers,true);
  std::cout<<"Postprocessor "<<_key
      <<": sums up histograms from PostProcessor "<< _pHist->key()
      <<" condition on postprocessor:"<<_condition->key()
      <<std::endl;
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
  QSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  const size_t min_time_user (settings.value("MinTime",0).toUInt());
  const size_t max_time_user (settings.value("MaxTime",300).toUInt());
  _timerange = make_pair(min_time_user,max_time_user);
  _nbrSamples=settings.value("NumberOfSamples",5).toUInt();
  _pHist = setupDependency("HistName");
  bool ret (setupCondition());
  if (!(ret && _pHist))
    return;
  const HistogramBackend &one(_pHist->getHist(0));
  _result = one.clone();
  createHistList(2*cass::NbrOfWorkers,true);
  std::cout << "PostProcessor "<<_key
      <<" will calculate the time average of histogram of PostProcessor_"<<_pHist->key()
      <<" from now "<<settings.value("MinTime",0).toUInt()
      <<" to "<<settings.value("MaxTime",300).toUInt()
      <<" seconds   "<<_timerange.first
      <<" ; "<<_timerange.second
      <<" each bin is equivalent to up to "<< _nbrSamples
      <<" measurements,"
      <<" condition on postprocessor:"<<_condition->key()
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






// ***  pp 70 takes a 0d histogram (value) as input and writes it in the last bin of a 1d histogram
//    *** while shifting all other previously saved values one bin to the left.

cass::pp70::pp70(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key)
{
  loadSettings(0);
}

void cass::pp70::loadSettings(size_t)
{
  using namespace std;
  QSettings settings;

  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());

  _one = setupDependency("HistName");

  bool ret (setupCondition());
  if ( !(_one && ret) ) return;

  _size = settings.value("Size", 10000).toUInt();

  _result = new Histogram1DFloat(_size, 0, _size-1);
  createHistList(2*cass::NbrOfWorkers,true);

  std::cout << "PostProcessor " << _key
      << ": will make a history of 0d histogram in pp "<< _one->key()
            << ", condition on postprocessor:" << _condition
            << ", size of history: " << _size
            << std::endl;
}

void cass::pp70::process(const cass::CASSEvent &evt)
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







// Local Variables:
// coding: utf-8
// mode: C++
// c-file-style: "gnu"
// c-file-offsets: ((c . 0) (innamespace . 0))
// fill-column: 100
// End:
