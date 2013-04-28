// Copyright (C) 2010 Jochen Küpper
// Copyright (C) 2010 Lutz Foucar

#include <algorithm>
#include <cassert>
#include <cmath>
#include <list>
#include <map>
#include <cmath>
#include <cstdlib>

#include "alignment.h"
#include "cass_event.h"
#include "histogram.h"
#include "convenience_functions.h"
#include "cass_settings.h"
#include "log.h"


using namespace cass;


//---postprocessor calculating cos2theta of requested averaged image----------
cass::pp200::pp200(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key)
{
  loadSettings(0);
}

void cass::pp200::loadSettings(size_t)
{
  using namespace std;
  CASSSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(QString::fromStdString(_key));
  setupGeneral();
  _image = setupDependency("HistName");
  bool ret = setupCondition();
  if (!(_image && ret))
    return;
  const HistogramBackend &hist (_image->getHist(0));
  // Width of image - we assum the images are square
  _imageWidth = hist.axis()[HistogramBackend::xAxis].size();
  // center of the image -- this is the center of the angluar distribution of the signal
  _userCenter = make_pair(settings.value("ImageXCenter", 500).toFloat(),
                          settings.value("ImageYCenter", 500).toFloat());
  // symmetry angle is the angle of in-plane rotation of the image with respect to its vertical axis
  _symAngle = settings.value("SymmetryAngle", 0).toFloat() * M_PI / 180.;
  // Set the minimum radius within range - must be within image
  _radiusRangeUser = make_pair(min(settings.value("MaxIncludedRadius",10).toFloat(),
                                   settings.value("MinIncludedRadius",0).toFloat()),
                               max(settings.value("MaxIncludedRadius",10).toFloat(),
                                   settings.value("MinIncludedRadius",0).toFloat()));
  setupParameters(hist);
  _result = new Histogram0DFloat();
  createHistList(2*cass::NbrOfWorkers);
  Log::add(Log::INFO,"PostProcessor '" + _key +
           "' calculates cos2theta of image from PostProcessor '" + _image->key() +
           "' Center is x'"+ toString(_center.first) + "' y'" + toString(_center.second) +
           "' Symmetry angle in radiants is '" + toString(_symAngle) +
           "' Min radius the user requested is '" + toString(_radiusRangeUser.first) +
           "' Max radius the user requested is '" + toString(_radiusRangeUser.second) +
           "' Image width is '" + toString(_imageWidth) +
           "' This results in Number of radial Points '" + toString(_nbrRadialPoints) +
           "'. Condition is '" + _condition->key() + "'");
}

void pp200::histogramsChanged(const HistogramBackend *in)
{
  using namespace std;
  setupParameters(*in);
  Log::add(Log::DEBUG0,"pp200::histogramsChanged(): hist has changed. The new settings for '" + _key +
           "' are: Image width is '" + toString(_imageWidth) +
           "' This results in Number of radial Points '" + toString(_nbrRadialPoints) + "'");
}

void pp200::setupParameters(const HistogramBackend &hist)
{
  using namespace std;
  if (hist.dimension() != 2)
    throw invalid_argument("pp200::setupParameters()'" + _key + "': Error the histogram we depend on '" + hist.key() +
                           "' is not a 2D Histogram.");
  const AxisProperty &xaxis(hist.axis()[HistogramBackend::xAxis]);
  const AxisProperty &yaxis(hist.axis()[HistogramBackend::yAxis]);
  _center = make_pair(xaxis.bin(_userCenter.first),
                      yaxis.bin(_userCenter.second));
  const size_t imagewidth (xaxis.nbrBins());
  const size_t imageheight (yaxis.nbrBins());
  const size_t dist_center_x_right (imagewidth - _center.first);
  const size_t dist_center_y_top (imageheight - _center.second);
  const size_t min_dist_x (min(dist_center_x_right, _center.first));
  const size_t min_dist_y (min(dist_center_y_top, _center.second));
  const size_t max_allowed_radius (min(min_dist_x, min_dist_y));
  const size_t user_maxradius_hist_coord (xaxis.bin(_radiusRangeUser.second)- xaxis.bin(0));
  const size_t maxRadius (min(max_allowed_radius, user_maxradius_hist_coord));
  const size_t user_minradius_hist_coord (xaxis.bin(_radiusRangeUser.first)- xaxis.bin(0));
  const size_t minRadius (max(size_t(1) , user_minradius_hist_coord));

  _radiusRange = make_pair(minRadius , maxRadius);
  _imageWidth = imagewidth;
  _nbrRadialPoints = maxRadius - minRadius;
  _nbrAngularPoints = 360;
}

void pp200::process(const CASSEvent& evt)
{
  using namespace std;
  const Histogram2DFloat &image
      (dynamic_cast<const Histogram2DFloat&>((*_image)(evt)));
  image.lock.lockForRead();
  const HistogramFloatBase::storage_t &imageMemory(image.memory());
  float nom(0), denom(0), maxval(0);
  for(size_t jr = 0; jr<_nbrRadialPoints; jr++)
  {
    for(size_t jth = 1; jth<_nbrAngularPoints; jth++)
    {
      const float radius(_radiusRange.first + jr);
      const float angle(2.*M_PI * float(jth) / float(_nbrAngularPoints));
      size_t col(size_t(_center.first  + radius*sin(angle + _symAngle)));
      size_t row(size_t(_center.second + radius*cos(angle + _symAngle)));
      float val = imageMemory[col + row * _imageWidth];
      denom += val * square(radius);
      nom   += val * square(cos(angle)) * square(radius);
      maxval = max(val,maxval);
    }
  }
  image.lock.unlock();
  _result->lock.lockForWrite();
  *dynamic_cast<Histogram0DFloat*>(_result) = (abs(denom) < 1e-15)?0.5:nom/denom;
  _result->lock.unlock();
}







// *** postprocessors 201 projects 2d hist to the radius for a selected center ***

cass::pp201::pp201(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key)
{
  loadSettings(0);
}

void cass::pp201::loadSettings(size_t)
{
  using namespace std;
  CASSSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  _userCenter = make_pair(settings.value("ImageXCenter", 500).toFloat(),
                          settings.value("ImageYCenter", 500).toFloat());
  _radiusRangeUser = make_pair(settings.value("MinIncludedRadius",10).toFloat(),
                               settings.value("MaxIncludedRadius",0).toFloat());
  _nbrAngularPoints = settings.value("NbrAngularPoints",360.).toUInt();
  setupGeneral();
  _image = setupDependency("HistName");
  bool ret (setupCondition());
  if (!(ret && _image))
    return;
  const Histogram2DFloat &one
      (dynamic_cast<const Histogram2DFloat&>(_image->getHist(0)));
  setupParameters(one);
  _result = new Histogram1DFloat(_nbrAngularPoints, 0., 360.);
  createHistList(2*cass::NbrOfWorkers);
  Log::add(Log::INFO,"PostProcessor '" + _key +
           "' will calculate the angular distribution of '" + _image->key() +
           "' from radia '" + toString(_radiusRange.first) + "' to '" +
           toString(_radiusRange.second) + "' with center x:" + toString(_center.first) +
           " y:" + toString(_center.second) + ". Number of Points on the axis '" +
           toString(_nbrAngularPoints) + "'. Condition is '" + _condition->key() + "'");
}

void pp201::histogramsChanged(const HistogramBackend *in)
{
  using namespace std;
  //return when there is no incomming histogram
  if(!in)
    return;
  //return when the incomming histogram is not a direct dependant
  if (find(_dependencies.begin(),_dependencies.end(),in->key()) == _dependencies.end())
    return;
  setupParameters(*in);
  Log::add(Log::DEBUG0,"pp201::histogramsChanged(): hist has changed. The new settings for '" + _key +
           "' are: Min radius is '" + toString(_radiusRange.first) + "' Max radius is '" +
           toString(_radiusRange.second) + "' This results in Number of radial Points '" +
           toString(_nbrRadialPoints) + "'");
}

void pp201::setupParameters(const HistogramBackend &hist)
{
  using namespace std;
  const AxisProperty &xaxis(hist.axis()[HistogramBackend::xAxis]);
  const AxisProperty &yaxis(hist.axis()[HistogramBackend::yAxis]);
  size_t imagewidth (xaxis.nbrBins());
  _center = make_pair(xaxis.bin(_userCenter.first),
                      yaxis.bin(_userCenter.second));
  _radiusRange = _radiusRangeUser;
  _radiusRange.second = min(min(_radiusRange.second, _center.first  + 0.5f), imagewidth - _center.first - 0.5f);
  _radiusRange.second = min(min(_radiusRange.second, _center.second + 0.5f), imagewidth - _center.second - 0.5f);
  _radiusRange.first = max(0.1f, min(_radiusRange.second - 1.0f , _radiusRangeUser.first));
  // Set number of points on grid
  _nbrRadialPoints = size_t(floor(_radiusRange.second - _radiusRange.first));
}

void pp201::process(const CASSEvent& evt)
{
  using namespace std;
  const Histogram2DFloat &image
      (dynamic_cast<const Histogram2DFloat&>((*_image)(evt)));
  image.lock.lockForRead();
  _result->lock.lockForWrite();
  const HistogramFloatBase::storage_t &imagememory(image.memory());
  size_t width(image.axis()[HistogramBackend::xAxis].nbrBins());
  dynamic_cast<Histogram1DFloat*>(_result)->clear();
  HistogramFloatBase::storage_t &histmemory
      (dynamic_cast<Histogram1DFloat*>(_result)->memory());
  for(size_t jr = 0; jr<_nbrRadialPoints ; jr++)
  {
    for(size_t jth = 0; jth<_nbrAngularPoints; jth++)
    {
      const float radius(jr+_radiusRange.first);
      const float angle_deg(jth*360/_nbrAngularPoints);
      const float angle(angle_deg * M_PI/ 180.f);
      const float x(_center.first  + radius*sin(angle));
      const float y(_center.second + radius*cos(angle));
      const size_t x1(static_cast<size_t>(x));
      const size_t x2(x1 + 1);
      const size_t y1(static_cast<size_t>(y));
      const size_t y2(y1 + 1);
      const float f11 (imagememory[y1 * width + x1]);
      const float f21 (imagememory[y1 * width + x2]);
      const float f12 (imagememory[y2 * width + x1]);
      const float f22 (imagememory[y2 * width + x2]);
      const float interpolateValue = f11*(x2 - x )*(y2 - y )+
                                     f21*(x   -x1)*(y2 - y )+
                                     f12*(x2 - x )*(y  - y1)+
                                     f22*(x  - x1)*(y  - y1);
      histmemory[jth] += interpolateValue;
    }
  }
  _result->nbrOfFills()=1;
  _result->lock.unlock();
  image.lock.unlock();
}





// *** postprocessor 202 transform 2d kartisian hist to polar coordinates ***

cass::pp202::pp202(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key)
{
  loadSettings(0);
}

void cass::pp202::loadSettings(size_t)
{
  using namespace std;
  CASSSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  _userCenter = make_pair(settings.value("ImageXCenter", 500).toFloat(),
                          settings.value("ImageYCenter", 500).toFloat());
  _nbrAngularPoints = settings.value("NbrAngularPoints",360.).toUInt();
  _nbrRadialPoints  = settings.value("NbrRadialPoints",500.).toUInt();
  setupGeneral();
  _image = setupDependency("HistName");
  bool ret (setupCondition());
  if (!(ret && _image))
    return;
  const Histogram2DFloat &one
      (dynamic_cast<const Histogram2DFloat&>(_image->getHist(0)));
  setupParameters(one);
  createHistList(2*cass::NbrOfWorkers);
  Log::add(Log::INFO,"PostProcessor '" + _key + "' will transform '" +_image->key() +
           "' to polar coordinates. Center x'"+ toString(_center.first) +"' y'" +
           toString(_center.second) + "'. Maximum radius is '" + toString(_maxRadius) +
           "'. Number of Points on the phi '" + toString(_nbrAngularPoints) +
           "'. Number of Points on the radius '" + toString(_nbrRadialPoints) +
           "'. Condition is '" + _condition->key() + "'");
}

void pp202::histogramsChanged(const HistogramBackend *in)
{
  using namespace std;
  //return when there is no incomming histogram
  if(!in)
    return;
  //return when the incomming histogram is not a direct dependant
  if (find(_dependencies.begin(),_dependencies.end(),in->key()) == _dependencies.end())
    return;
  setupParameters(*in);
  createHistList(2*cass::NbrOfWorkers);
  //notify all pp that depend on us that our histograms have changed
  PostProcessors::keyList_t dependands (_pp.find_dependant(_key));
  PostProcessors::keyList_t::iterator it (dependands.begin());
  for (; it != dependands.end(); ++it)
    _pp.getPostProcessor(*it).histogramsChanged(_result);
  Log::add(Log::INFO,"pp202::histogramsChanged(): hist has changed. '" + _key +
           "' maxradius is '" + toString(_maxRadius) + "'");
}

void pp202::setupParameters(const HistogramBackend &hist)
{
  using namespace std;
  try
  {
    if (hist.dimension() != 2)
      throw invalid_argument("pp202::setupParameters()'" + _key +
                             "': Error the histogram we depend on '" +
                             hist.key() + "' is not a 2D Histogram.");
    const AxisProperty &xaxis(hist.axis()[HistogramBackend::xAxis]);
    const AxisProperty &yaxis(hist.axis()[HistogramBackend::yAxis]);
    _center = make_pair(xaxis.bin(_userCenter.first),
                        yaxis.bin(_userCenter.second));
    const size_t imagewidth (xaxis.nbrBins());
    const size_t imageheight (yaxis.nbrBins());
    const size_t dist_center_x_right(imagewidth - _center.first);
    const size_t dist_center_y_top(imageheight - _center.second);
    const size_t min_dist_x (min(dist_center_x_right, _center.first));
    const size_t min_dist_y (min(dist_center_y_top, _center.second));
    _maxRadius = min(min_dist_x, min_dist_y);
  }
  /** catch the out of range errors and intialize center and max radius
   *  with 0. Hopefully once everything resizes to the correct image
   *  size, no errors will be thrown anymore
   */
  catch(const out_of_range &error)
  {
    Log::add(Log::DEBUG0,"Postprocessor 202 '" + _key +
             "' setupParameters: Out of Range Error is '" + error.what() +
             "'. Initializing center and radius with 0.");
    _center = make_pair(0,0);
    _maxRadius = 0;
  }
  _result = new Histogram2DFloat(_nbrAngularPoints, 0., 360.,
                                 _nbrRadialPoints,0., _maxRadius,
                                 "#phi","r");

}

void pp202::process(const CASSEvent& evt)
{
  using namespace std;
  const Histogram2DFloat &image
      (dynamic_cast<const Histogram2DFloat&>((*_image)(evt)));
  image.lock.lockForRead();
  _result->lock.lockForWrite();
  const HistogramFloatBase::storage_t &imagememory(image.memory());
  const size_t width(image.axis()[HistogramBackend::xAxis].nbrBins());
  dynamic_cast<Histogram2DFloat*>(_result)->clear();
  HistogramFloatBase::storage_t &resulthistmemory
      (dynamic_cast<Histogram2DFloat*>(_result)->memory());
  for(size_t jr = 0; jr<_nbrRadialPoints ; jr++)
  {
    for(size_t jth = 0; jth<_nbrAngularPoints; jth++)
    {
      const float radius(jr*_maxRadius/_nbrRadialPoints);
      const float angle_deg(jth*360/_nbrAngularPoints);
      const float angle(angle_deg * M_PI/ 180.f);
      const float x(_center.first  + radius*sin(angle));
      const float y(_center.second + radius*cos(angle));
      const size_t x1(static_cast<size_t>(x));
      const size_t x2(x1 + 1);
      const size_t y1(static_cast<size_t>(y));
      const size_t y2(y1 + 1);
      const float f11 (imagememory[y1 * width + x1]);
      const float f21 (imagememory[y1 * width + x2]);
      const float f12 (imagememory[y2 * width + x1]);
      const float f22 (imagememory[y2 * width + x2]);
      const float interpolateValue = f11*(x2 - x )*(y2 - y )+
                                     f21*(x   -x1)*(y2 - y )+
                                     f12*(x2 - x )*(y  - y1)+
                                     f22*(x  - x1)*(y  - y1);
      resulthistmemory[jr*_nbrAngularPoints + jth] += interpolateValue;
    }
  }
  _result->nbrOfFills()=1;
  _result->lock.unlock();
  image.lock.unlock();
}





// Local Variables:
// coding: utf-8
// mode: C++
// c-file-style: "gnu"
// c-file-offsets: ((c . 0) (innamespace . 0))
// fill-column: 100
// End:
