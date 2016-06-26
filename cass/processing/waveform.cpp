//Copyright (C) 2010-2011 Lutz Foucar

/**
 * @file waveform.cpp file contains acqiris data retrieval processor
 *                    definition
 *
 * @author Lutz Foucar
 */

#include <stdexcept>
#include <cmath>
#include <algorithm>
#include <string>
#include <tr1/functional>
#include <utility>

#include "waveform.h"
#include "result.hpp"
#include "cass_event.h"
#include "cass.h"
#include "acqiris_device.hpp"
#include "convenience_functions.h"
#include "cass_settings.h"
#include "log.h"
#include "cass_exceptions.hpp"

using namespace cass;
using namespace ACQIRIS;
using std::runtime_error;
using std::logic_error;
using std::minus;
using std::multiplies;
using std::cout;
using std::endl;
using std::invalid_argument;
using std::tr1::shared_ptr;
using std::tr1::bind;
using std::tr1::placeholders::_1;
using std::make_pair;

//the last wavefrom processor
pp110::pp110(const name_t &name)
  :Processor(name)
{
  loadSettings(0);
}

void pp110::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("Processor");
  s.beginGroup(QString::fromStdString(name()));
  _instrument = s.value("InstrumentId",8).toUInt();
  _channel    = s.value("ChannelNbr",0).toUInt();
  int wsize(s.value("NbrSamples",40000).toInt());
  _sampleInterval = s.value("SampleInterval",1e-9).toDouble();
  setupGeneral();
  if (!setupCondition())
    return;
  createHistList
      (result_t::shared_pointer
        (new result_t
         (result_t::axe_t(wsize,0,wsize*_sampleInterval,"Time [s]"))));
  Log::add(Log::INFO,"Processor '" + name() + "' is showing channel '" +
           toString(_channel) + "' of acqiris '" + toString(_instrument) +
           "'. Condition is '" + _condition->name() + "'");
}

void pp110::process(const CASSEvent &evt, result_t &result)
{
  const Device &dev
      (dynamic_cast<const Device&>(*(evt.devices().find(CASSEvent::Acqiris)->second)));
  Device::instruments_t::const_iterator instrIt (dev.instruments().find(_instrument));
  if (dev.instruments().end() == instrIt)
    throw logic_error("pp110::process() '" + name() +
                        "': Data doesn't contain Instrument '"+toString(_instrument)
                        + "'");
  const Instrument &instr(instrIt->second);
  if (instr.channels().size() <= _channel)
    throw runtime_error("pp110::process() '" + name() + "': Instrument '"+
                        toString(_instrument) + "' doesn't contain channel '" +
                        toString(_channel)+ "'");
  const Channel &channel (instr.channels()[_channel]);
  const Channel::waveform_t &waveform (channel.waveform());
  if (result.shape().first != waveform.size())
  {
    throw invalid_argument("processor '" + name() +
                           "' incomming waveforms NbrSamples '" + toString(waveform.size()) +
                           "'. User set NbrSamples '" +
                           toString(result.shape().first) +
                           "'");
  }
  if (!std::isfinite(channel.gain()))
  {
    throw InvalidData("pp110::process(): Processor '"  + name() +
                      "': The provided gain '" + toString(channel.gain()) +
                      "' is not a number");
  }
  if (!std::isfinite(channel.sampleInterval()))
  {
    throw InvalidData("pp110::process(): Processor '"  + name() +
                      "': The provided sampleInterval '" +
                      toString(channel.sampleInterval()) + "' is not a number");
  }
  if (!std::isfinite(channel.offset()))
  {
    throw InvalidData("pp110::process(): Processor '"  + name() +
                      "': The provided vertical offset '" +
                      toString(channel.offset()) + "' is not a number");
  }
  if (!(std::abs(channel.sampleInterval()-_sampleInterval) < sqrt(std::numeric_limits<double>::epsilon())))
  {
    throw invalid_argument("processor '" + name() +
                           "' incomming waveforms SampleInterval '" + toString(channel.sampleInterval()) +
                           "'. User set SampleInterval '" + toString(_sampleInterval) + "'");
  }
  transform(waveform.begin(), waveform.end(), result.begin(),
            bind(minus<float>(),
                 bind(multiplies<float>(),channel.gain(),_1),channel.offset()));
}




// ***cfd trace from waveform

pp111::pp111(const name_t &name)
  :Processor(name)
{
  loadSettings(0);
}

void pp111::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("Processor");
  s.beginGroup(QString::fromStdString(name()));
  _waveform = setupDependency("Waveform");
  setupGeneral();
  if (!setupCondition())
    return;
  if (_waveform->result().dim() != 1)
    throw invalid_argument("pp111 '" + name() + "' histogram '" + _waveform->name() +
                           "' is not a 1D histogram");

  _fraction = s.value("Fraction",0.6).toFloat();
  _walk = s.value("Walk_V",0).toFloat();
  const float delay(s.value("Delay_ns",5).toFloat());
  const size_t nBins(_waveform->result().axis(result_t::xAxis).nBins);
  const float Up(_waveform->result().axis(result_t::xAxis).up);
  const float samplInter(Up/nBins);
  _delay = static_cast<size_t>(delay/samplInter);

  createHistList(_waveform->result().clone());

  Log::add(Log::INFO,"Processor '" + name() + "' is converting waveform '" +
           _waveform->name() + "' to a CFD Trace using delay '" + toString(delay) +
           "', Fraction '" + toString(_fraction) + "', Walk '" + toString(_walk) +
           "'. Condition is '" + _condition->name() + "'");
}

void pp111::process(const CASSEvent &evt, result_t &result)
{
  const result_t& waveform(_waveform->result(evt.id()));
  QReadLocker lock(&waveform.lock);

  const size_t wLength(waveform.shape().first);
  /** set all points before the delay to 0 */
  fill(result.begin(),result.begin()+_delay,0);
  for (size_t i=_delay; i<wLength; ++i)
  {
    /** get the original value at i */
    const float fx  = waveform[i];
    /** get the delayed value at i-delay */
    const float fxd = waveform[i-_delay];
    /** the constant fraction value at i is \f fx*fraction + fx_{delayed}\f */
    const float fsx = -fx*_fraction + fxd;
    /** now remove the walk from the constant fraction to get the real cfd value */
    result[i] = fsx - _walk;
  }
}




// ***cfd analysis of waveform

pp112::pp112(const name_t &name)
  :Processor(name)
{
  loadSettings(0);
}

void pp112::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("Processor");
  s.beginGroup(QString::fromStdString(name()));
  _waveform = setupDependency("Waveform");
  setupGeneral();
  if (!setupCondition())
    return;
  if (_waveform->result().dim() != 1)
    throw invalid_argument("pp111 '" + name() + "' histogram '" + _waveform->name() +
                           "' is not a 1D histogram");

  _threshold = s.value("Threshold_V",0.1).toFloat();
  _fraction = s.value("Fraction",0.6).toFloat();
  _walk = s.value("Walk_V",0).toFloat();
  const float delay(s.value("Delay_ns",5).toFloat());
  const size_t nBins(_waveform->result().axis(result_t::xAxis).nBins);
  const float Up(_waveform->result().axis(result_t::xAxis).up);
  const float samplInter(Up/nBins);
  _delay = static_cast<size_t>(delay/samplInter);

  /** Create the result output */
  createHistList(result_t::shared_pointer(new result_t(nbrOf,0)));

  Log::add(Log::INFO,"Processor '" + name() + "' is converting waveform '" +
           _waveform->name() + "' to a CFD Trace using delay '" + toString(delay) +
           "', Fraction '" + toString(_fraction) + "', Walk '" + toString(_walk) +
           "'. Condition is '" + _condition->name() + "'");
}

pp112::fitparam_t pp112::linearRegression(points_t::const_iterator first,
                                          points_t::const_iterator last)
{
  float SumXsq(0.f),SumX(0.f),SumY(0.f),SumXY(0.f);
  size_t nPoints(0);
  while (first != last)
  {
    SumX    +=  first->first;
    SumY    +=  first->second;
    SumXY   += (first->first*first->second);
    SumXsq  += (first->first*first->first);
    ++nPoints;
  }
  const float a1 = ((SumX*SumX) - (nPoints*SumXsq));
  return make_pair(((SumX*SumXY) - (SumY*SumXsq)) / a1,
                   ((SumX*SumY) - (nPoints*SumXY)) / a1);
}

void pp112::createNewtonPolynomial(const float *x, const float *y, float *coeff)
{
  double f_x0_x1 = (y[1]-y[0]) / (x[1]-x[0]);
  double f_x1_x2 = (y[2]-y[1]) / (x[2]-x[1]);
  double f_x2_x3 = (y[3]-y[2]) / (x[3]-x[2]);

  double f_x0_x1_x2 = (f_x1_x2 - f_x0_x1) / (x[2]-x[0]);
  double f_x1_x2_x3 = (f_x2_x3 - f_x1_x2) / (x[3]-x[1]);

  double f_x0_x1_x2_x3 = (f_x1_x2_x3 - f_x0_x1_x2) / (x[3]-x[0]);

  coeff[0] = y[0];
  coeff[1] = f_x0_x1;
  coeff[2] = f_x0_x1_x2;
  coeff[3] = f_x0_x1_x2_x3;
}

float pp112::evalNewtonPolynomial(const float *x, const float *coeff, float X)
{
  double returnValue = coeff[3];
  returnValue = returnValue * (X - x[2]) + coeff[2];
  returnValue = returnValue * (X - x[1]) + coeff[1];
  returnValue = returnValue * (X - x[0]) + coeff[0];

  return returnValue;
}

float pp112::findXForGivenY(const float *x, const float *coeff, const float Y, const float Start)
{
  typedef std::pair<double,double> punkt_t;
  /** initialize the boundaries */
  punkt_t Low(x[1], evalNewtonPolynomial(x,coeff,x[1]));
  punkt_t Up (x[2], evalNewtonPolynomial(x,coeff,x[2]));

  /** intialize the starting value */
  punkt_t p (Start, evalNewtonPolynomial(x,coeff,Start));

  /** right value? then return the correspoinding x value */
  if (p.second == Y)
    return p.first;

  /** find the type of crossing */
  bool Neg = (Low.second > Up.second)?true:false;

  /** if its a negative crossing, and the y-value is bigger than the requested
   *  then point is the new lower boundary. If the y-value is smaller than the
   *  requested value, then the value is the new upper boundary. If we hit the
   *  spot, return the corresponding x-value
   */
  if (Neg)
  {
    if (p.second > Y)
      Low = p;
    else if (p.second < Y)
      Up = p;
    else
      return p.first;
  }
  /** if its a positive crossing, and the y-value is bigger than the requested
   *  then point is the new upper boundary. If the y-value is smaller than the
   *  requested value, then the value is the new lower boundary. If we hit the
   *  spot, return the corresponding x-value
   */
  else
  {
    if (p.second > Y)
      Up = p;
    else if (p.second < Y)
      Low = p;
    else
      return p.first;
  }

  /** find new boundaries until the difference between the x-values of the
   *  boundaries is samller than 0.005
   */
  while((Up.first-Low.first) > 0.005)
  {
    /** the new x value is the arithmetic mean between the two boundaries  and
     *  determines the new x-values to be checked (with the corresponding y-value)
     */
    p.first = 0.5 * (Up.first+Low.first);
    p.second = evalNewtonPolynomial(x,coeff,p.first);

  /** if its a negative crossing, and the y-value is bigger than the requested
   *  then point is the new lower boundary. If the y-value is smaller than the
   *  requested value, then the value is the new upper boundary. If we hit the
   *  spot, return the corresponding x-value
   */
    if (Neg)
    {
      if (p.second > Y)
        Low = p;
      else if (p.second < Y)
        Up = p;
      else
        return p.first;
    }
  /** if its a positive crossing, and the y-value is bigger than the requested
   *  then point is the new upper boundary. If the y-value is smaller than the
   *  requested value, then the value is the new lower boundary. If we hit the
   *  spot, return the corresponding x-value
   */
    else
    {
      if (p.second > Y)
        Up = p;
      else if (p.second < Y)
        Low = p;
      else
        return p.first;
    }
  }
  /** return the arithmentic mean between the two boundaries */
  return ((Up.first + Low.first)*0.5);
}

void pp112::process(const CASSEvent &evt, result_t &result)
{
  const result_t& waveform(_waveform->result(evt.id()));
  QReadLocker lock(&waveform.lock);

  /** clear the resulting table to fill it with the values of this image */
  result.resetTable();

  /** get a table row that we can later add to the table */
  table_t peak(nbrOf,0);

  const size_t wLength(waveform.shape().first);
  for (size_t i=_delay+1; i<wLength-2; ++i)
  {
    /** calculate the constant fraction at i */
    const float fx(waveform[i]);
    const float fxd(waveform[i-_delay]);
    const float fsx(-fx*_fraction + fxd);

    /** calculate the constant fraction at i+1 */
    const float fx_1(waveform[i+1]);
    const float fxd_1(waveform[i+1-_delay]);
    const float fsx_1(-fx_1*_fraction + fxd_1);

    /** check wether the criteria for a Peak are fullfilled:
     *  one point above one below the walk
     *  original point above the threshold
     */
    if ((((fsx-_walk) * (fsx_1-_walk)) <= 0) && (fabs(fx) > _threshold))
    {
      /** it could be that the first criteria is 0 because one of the
       *  Constant Fraction Signal Points or both are exactly where the walk is
       */
      /** both points are on the walk:
       *  go to next loop until at least one is over or under the walk
       */
      if (fuzzycompare(fsx,fsx_1))
        continue;
      /** only first is on walk:
       *  this is what we want, so do nothing
       */
      else if ((fsx-_walk) == 0);
      /** only second is on walk:
       *  we want that the first point will be on the walk so in the next loop
       *  iteration this point will be the first
       */
      else if ((fsx_1-_walk) == 0)
        continue;

      /** check the polarity */
      /** if two pulses are close together then the cfsignal goes through the
       *  walk three times, where only two crossings are good. So we need to
       *  check for the one where it is not good
       */
      /** negative polarity, but positive Puls -> skip */
      if ((fsx > fsx_1) && (waveform[i] > _baseline))
          continue;
      /** positive polarity, but negative Puls -> skip */
      if ((fsx < fsx_1) && (waveform[i] < _baseline))
          continue;

      /** to find the position of the crossing more precisely we need two more
       *  points, so create them here
       */
      /** calculate the constant fraction at i+1 */
      const float fx_m1(waveform[i-1] - _baseline);
      const float fxd_m1(waveform[i-1-_delay] -_baseline);
      const float fsx_m1(-fx_m1*_fraction + fxd_m1);

      /** calculate the constant fraction at i+2 */
      const float fx_2(waveform[i+2] - _baseline);
      const float fxd_2(waveform[i+2-_delay] - _baseline);
      const float fsx_2(-fx_2*_fraction + fxd_2);

      /** find x with a linear interpolation between the two points */
      const float m(fsx_1-fsx);                    //(fsx-fsx_1)/(i-(i+1));
      const float xLin(i + (_walk - fsx)/m);        //PSF fx = (x - i)*m + cfs[i]

//      /** make a linear regression to find the slope of the leading edge */
//      const float SumX((i-_delay) + (i+1-_delay) + (i+2-_delay));
//      const float SumY(fxd + fxd_1 + fxd_2);
//      const float SumXY((i-_delay)*fxd + (i+1-_delay)*fxd_1 + (i+2-_delay)*fxd_2);
//      const float SumXsq((i-_delay)*(i-_delay) +
//                         (i+1-_delay)*(i+1-_delay) +
//                         (i+2-_delay)*(i+2-_delay));
//      const float a1((SumX*SumX) - (3*SumXsq));
//      const float m(((SumX*SumY) - (3*SumXY)) / a1);
//      const float c(((SumX*SumXY) - (SumY*SumXsq)) / a1);

      /** Find x with a cubic polynomial interpolation between four points
       *  do this with the Newtons interpolation Polynomial.
       *  Numericaly solve the Newton Polynomial, give the linear approach for
       *  x as Start Value
       */
      const float x[4] = {static_cast<float>(i-1), static_cast<float>(i),
                          static_cast<float>(i+1), static_cast<float>(i+2)};
      const float y[4] = {fsx_m1,fsx,fsx_1,fsx_2};
      float coeff[4] = {0.f,0.f,0.f,0.f};
      createNewtonPolynomial(x,y,coeff);
      const float xPoly(findXForGivenY(x,coeff,_walk,xLin));

      /** add info of the peak */
      const float low(waveform.axis(result_t::xAxis).low);
      const float up(waveform.axis(result_t::xAxis).up);
      const float nBins(waveform.axis(result_t::xAxis).nBins);
      peak[position] = low +(xPoly*(up-low)/nBins);
      if (fsx > fsx_1)
        peak[polarity] = 0;
      if (fsx < fsx_1)
        peak[polarity] = 1;
      if (fuzzycompare(fsx,fsx_1))
        peak[polarity] = 2;
      /** go left from center until either i == 0, or the datapoint is inside
       *  the noise or we go from the previous one (i+1) to the actual one (i)
       *  through the baseline
       */
      int start(i);
      for (; start>=0; --start)
        if ((fabs(waveform[start]-_baseline) < _threshold))
          break;
      peak[startpos] = start;
      /** go right form center until either i < pulslength, or the datapoint
       *  is inside the noise or we go from the previous one (i-1) to the
       *  actual one (i) through the baseline
       */
      size_t stop(i);
      for (; stop < wLength; ++stop)
        if ((fabs(waveform[stop]-_baseline) < _threshold))
          break;
      peak[endpos] = stop;
      peak[width] = stop - start;

      /** go through whole peak and determine the integral and center of mass,
       *  while finding the maximum.
       */
      peak[height] = 0;
      float weight(0);
      peak[integral] = 0;
      for (size_t j(start); j<=stop;++j)
      {
        const float y(fabs(waveform[j] - _baseline));
        peak[integral] += y;
        weight += y * j;
        if (y > peak[height])
        {
          peak[height] = y;
          peak[maxpos]  = j;
        }
      }
      peak[CoM] = weight / peak[integral];

      /** determine the fwhm of the peak with linear regression */
      const float halfmax(peak[height]*0.5);
      /** go from maxpos to left until last point that is above 0.5*height */
      size_t fwhm_l(peak[maxpos]);
      while (fabs(waveform[fwhm_l]-_baseline) > halfmax)
        --fwhm_l;
      /** go from maxpos to right until last point that is above  0.5*height */
      size_t fwhm_r(peak[maxpos]);
      while (fabs(waveform[fwhm_r]-_baseline) > halfmax)
        ++fwhm_r;
      /** make linear regression through 4 points */
      points_t points(4,make_pair(0,0));
      points[0].first = fwhm_l-2;    points[0].second = fabs(waveform[fwhm_l-2]-_baseline);
      points[1].first = fwhm_l-1;    points[1].second = fabs(waveform[fwhm_l-1]-_baseline);
      points[2].first = fwhm_l-0;    points[2].second = fabs(waveform[fwhm_l-0]-_baseline);
      points[3].first = fwhm_l+1;    points[3].second = fabs(waveform[fwhm_l+1]-_baseline);
      fitparam_t left_fitparam(linearRegression(points.begin(),points.end()));
      points[0].first = fwhm_r-1;    points[0].second = fabs(waveform[fwhm_r-1]-_baseline);
      points[1].first = fwhm_r-0;    points[1].second = fabs(waveform[fwhm_r-0]-_baseline);
      points[2].first = fwhm_r+1;    points[2].second = fabs(waveform[fwhm_r+1]-_baseline);
      points[3].first = fwhm_r+2;    points[3].second = fabs(waveform[fwhm_r+2]-_baseline);
      fitparam_t right_fitparam(linearRegression(points.begin(),points.end()));

      //y = m*x+c => x = (y-c)/m;
      const float fwhm_L((halfmax-left_fitparam.first)/left_fitparam.second);
      const float fwhm_R((halfmax-right_fitparam.first)/right_fitparam.second);
      peak[fwhm] = fwhm_R-fwhm_L;

      /** add peak to table */
      result.appendRows(peak);
    }
  }
}
