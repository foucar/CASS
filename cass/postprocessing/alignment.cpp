// Copyright (C) 2010 Jochen Kuepper
// Copyright (C) 2010-2013 Lutz Foucar

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
using namespace std;


//---postprocessor calculating cos2theta of requested averaged image----------

pp200::pp200(const name_t &name)
  : PostProcessor(name)
{
  loadSettings(0);
}

void pp200::loadSettings(size_t)
{
  using namespace std;
  CASSSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(QString::fromStdString(name()));
  setupGeneral();
  _image = setupDependency("HistName");
  bool ret = setupCondition();
  if (!(_image && ret))
    return;
  const HistogramBackend &hist(_image->result());
  // Width of image - we assume the images are square
  _imageWidth = hist.axis()[HistogramBackend::xAxis].size();
  // center of the image -- this is the center of the angluar distribution of the signal
  pair<float,float>_userCenter(make_pair(settings.value("ImageXCenter", 500).toFloat(),
                                         settings.value("ImageYCenter", 500).toFloat()));
  // symmetry angle is the angle of in-plane rotation of the image with respect to its vertical axis
  _symAngle = settings.value("SymmetryAngle", 0).toFloat() * M_PI / 180.;
  // Set the minimum radius within range - must be within image
  pair<float,float>_radiusRangeUser(make_pair(min(settings.value("MaxIncludedRadius",10).toFloat(),
                                                  settings.value("MinIncludedRadius",0).toFloat()),
                                              max(settings.value("MaxIncludedRadius",10).toFloat(),
                                                  settings.value("MinIncludedRadius",0).toFloat())));
  if (hist.dimension() != 2)
    throw invalid_argument("pp200::loadSettings()'" + name() +
                           "': Error the histogram we depend on '" + _image->name() +
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

  createHistList(tr1::shared_ptr<Histogram0DFloat>(new Histogram0DFloat()));
  Log::add(Log::INFO,"PostProcessor '" + name() +
           "' calculates cos2theta of image from PostProcessor '" + _image->name() +
           "' Center is x'"+ toString(_center.first) + "' y'" + toString(_center.second) +
           "' Symmetry angle in radiants is '" + toString(_symAngle) +
           "' Min radius the user requested is '" + toString(_radiusRangeUser.first) +
           "' Max radius the user requested is '" + toString(_radiusRangeUser.second) +
           "' Image width is '" + toString(_imageWidth) +
           "' This results in Number of radial Points '" + toString(_nbrRadialPoints) +
           "'. Condition is '" + _condition->name() + "'");
}

void pp200::process(const CASSEvent& evt, HistogramBackend &res)
{
  const Histogram2DFloat &image
      (dynamic_cast<const Histogram2DFloat&>(_image->result(evt.id())));
  Histogram0DFloat &result(dynamic_cast<Histogram0DFloat&>(res));

  QReadLocker lock(&image.lock);

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
  result = (abs(denom) < 1e-15)?0.5:nom/denom;
  result.nbrOfFills() = 1;
}







// *** postprocessors 201 projects 2d hist to the radius for a selected center ***

pp201::pp201(const name_t &name)
  : PostProcessor(name)
{
  loadSettings(0);
}

void pp201::loadSettings(size_t)
{
  using namespace std;
  CASSSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(QString::fromStdString(name()));
  pair<float,float> _userCenter(make_pair(settings.value("ImageXCenter", 500).toFloat(),
                                          settings.value("ImageYCenter", 500).toFloat()));
  pair<float,float> _radiusRangeUser(make_pair(settings.value("MinIncludedRadius",10).toFloat(),
                                               settings.value("MaxIncludedRadius",0).toFloat()));
  _nbrAngularPoints = settings.value("NbrAngularPoints",360.).toUInt();
  setupGeneral();
  _image = setupDependency("HistName");
  bool ret (setupCondition());
  if (!(ret && _image))
    return;
  if (_image->result().dimension() != 2)
    throw invalid_argument("pp201::loadSettings()'" + name() +
                           "': Error the histogram we depend on '" + _image->name() +
                           "' is not a 2D Histogram.");
  const Histogram2DFloat &hist
      (dynamic_cast<const Histogram2DFloat&>(_image->result()));
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
  createHistList(tr1::shared_ptr<Histogram1DFloat>
                  (new Histogram1DFloat(_nbrAngularPoints, 0., 360.)));
  Log::add(Log::INFO,"PostProcessor '" + name() +
           "' will calculate the angular distribution of '" + _image->name() +
           "' from radia '" + toString(_radiusRange.first) + "' to '" +
           toString(_radiusRange.second) + "' with center x:" + toString(_center.first) +
           " y:" + toString(_center.second) + ". Number of Points on the axis '" +
           toString(_nbrAngularPoints) + "'. Condition is '" + _condition->name() + "'");
}

void pp201::process(const CASSEvent& evt, HistogramBackend &res)
{
  const Histogram2DFloat &image
      (dynamic_cast<const Histogram2DFloat&>(_image->result(evt.id())));
  Histogram1DFloat &result(dynamic_cast<Histogram1DFloat&>(res));

  QReadLocker lock(&image.lock);

  const HistogramFloatBase::storage_t &imagememory(image.memory());
  size_t width(image.axis()[HistogramBackend::xAxis].nbrBins());

  result.clear();
  HistogramFloatBase::storage_t &histmemory(result.memory());

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
  result.nbrOfFills()=1;
}





// *** postprocessor 202 transform 2d kartisian hist to polar coordinates ***

pp202::pp202(const name_t &name)
  : PostProcessor(name)
{
  loadSettings(0);
}

void pp202::loadSettings(size_t)
{
  using namespace std;
  CASSSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(QString::fromStdString(name()));
  _userCenter = make_pair(settings.value("ImageXCenter", 500).toFloat(),
                          settings.value("ImageYCenter", 500).toFloat());
  _nbrAngularPoints = settings.value("NbrAngularPoints",360.).toUInt();
  _nbrRadialPoints  = settings.value("NbrRadialPoints",500.).toUInt();
  setupGeneral();
  _image = setupDependency("HistName");
  bool ret (setupCondition());
  if (!(ret && _image))
    return;
  if (_image->result().dimension() != 2)
    throw invalid_argument("pp202::loadSettings()'" + name() +
                           "': Error the histogram we depend on '" + _image->name() +
                           "' is not a 2D Histogram.");
  const Histogram2DFloat &hist
      (dynamic_cast<const Histogram2DFloat&>(_image->result()));
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

  createHistList(
        tr1::shared_ptr<Histogram2DFloat>
        (new Histogram2DFloat(_nbrAngularPoints, 0., 360.,
                              _nbrRadialPoints,0., _maxRadius,
                              "#phi","r")));

  Log::add(Log::INFO,"PostProcessor '" + name() + "' will transform '" +_image->name() +
           "' to polar coordinates. Center x'"+ toString(_center.first) +"' y'" +
           toString(_center.second) + "'. Maximum radius is '" + toString(_maxRadius) +
           "'. Number of Points on the phi '" + toString(_nbrAngularPoints) +
           "'. Number of Points on the radius '" + toString(_nbrRadialPoints) +
           "'. Condition is '" + _condition->name() + "'");
}

void pp202::process(const CASSEvent& evt, HistogramBackend &res)
{
  const Histogram2DFloat &image
      (dynamic_cast<const Histogram2DFloat&>(_image->result(evt.id())));
  Histogram2DFloat &result(dynamic_cast<Histogram2DFloat&>(res));

  QReadLocker lock(&image.lock);

  const HistogramFloatBase::storage_t &imagememory(image.memory());
  const size_t width(image.axis()[HistogramBackend::xAxis].nbrBins());
  result.clear();
  HistogramFloatBase::storage_t &resulthistmemory(result.memory());
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
  result.nbrOfFills()=1;
}

