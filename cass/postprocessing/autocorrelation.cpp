//Copyright (C) 2013 Lutz Foucar

/**
 * @file autocorrelation.cpp containing the class to calculate the
 *                         autocorrelation of a 2d histogram
 *
 * @author Lutz Foucar
 */

#include <algorithm>
#include <functional>
#include <numeric>

#include "autocorrelation.h"
#include "histogram.h"
#include "cass_event.h"
#include "cass_settings.h"
#include "log.h"
#include "postprocessor.h"

using namespace cass;
using namespace std;


pp310::pp310(const name_t &name)
  : PostProcessor(name)
{
  loadSettings(0);
}

void pp310::loadSettings(size_t)
{
  setupGeneral();
  _hist = setupDependency("HistName");
  bool ret (setupCondition());
  if (!(ret && _hist))
    return;
  const Histogram2DFloat &srcImageHist(dynamic_cast<const Histogram2DFloat&>(_hist->result()));
  if (srcImageHist.dimension() != 2)
    throw invalid_argument("pp310:setup: '" + name() +
                           "' The input histogram is not a 2d histogram");
  createHistList(srcImageHist.copy_sptr());

  Log::add(Log::INFO,"PostProcessor '" + name() +
           "' will calculate the autocorrelation of '" + _hist->name() +
           "'. Condition is '" + _condition->name() + "'");
}

void pp310::process(const CASSEvent &evt, HistogramBackend& res)
{
  typedef HistogramFloatBase::storage_t::const_iterator const_iterator;
  const Histogram2DFloat& hist
      (dynamic_cast<const Histogram2DFloat&>(_hist->result(evt.id())));
  const Histogram2DFloat::storage_t& histdata( hist.memory() );

  Histogram2DFloat &result(dynamic_cast<Histogram2DFloat&>(res));
  Histogram2DFloat::storage_t& resultdata( result.memory() );

  QWriteLocker(&result.lock);
  QReadLocker(&hist.lock);

  size_t nCols(hist.axis()[HistogramBackend::xAxis].size());
  size_t nRows(hist.axis()[HistogramBackend::yAxis].size());

  HistogramFloatBase::storage_t row_buffer(nCols);
  /** go through each row (radius) of input */
  for (size_t row(0); row < nRows; ++row)
  {
    const_iterator rowStart(histdata.begin() + row*nCols);
    const_iterator rowEnd(histdata.begin() + row*nCols + nCols);
    /** go through each col (phi) of input */
    for (size_t col(0); col < nCols; ++col)
    {
      /** rotate the input to get the current d_phi and put result into buffer */
      rotate_copy(rowStart, rowStart+col, rowEnd, row_buffer.begin());

      /** multiply the original phi with the rotated phi and put result into buffer */
      transform(rowStart,rowEnd,row_buffer.begin(),row_buffer.begin(),multiplies<float>());

      /** sum up the contents of the buffer which gives the correlation value */
      const float autocorval(accumulate(row_buffer.begin(),row_buffer.end(),0));

      /** put value into correlatoin value at position radius, d_phi */
      resultdata[row*nCols + col] = autocorval;
    }
  }
}



//*** autocorrelation from image in kartesian coordinates ***
pp311::pp311(const name_t &name)
  : PostProcessor(name)
{
  loadSettings(0);
}

void pp311::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("PostProcessor");
  s.beginGroup(QString::fromStdString(name()));
  setupGeneral();
  _hist = setupDependency("HistName");
  bool ret (setupCondition());
  if (!(ret && _hist))
    return;

  std::pair<int,int> usercenter = make_pair(s.value("CenterX",0).toInt(),
                                            s.value("CenterY",0).toInt());
  const int maxradius(s.value("MaximumRadius",0).toInt());

  const Histogram2DFloat &srcImageHist(dynamic_cast<const Histogram2DFloat&>(_hist->result()));
  if (srcImageHist.dimension() != 2)
    throw invalid_argument("pp311:setup: '" + name() +
                           "' The input histogram is not a 2d histogram");
  /** determine the center in histogram coordinates */
  const AxisProperty &xaxis(srcImageHist.axis()[HistogramBackend::xAxis]);
  const AxisProperty &yaxis(srcImageHist.axis()[HistogramBackend::yAxis]);
  _center = make_pair(xaxis.bin(usercenter.first),
                      yaxis.bin(usercenter.second));

  /** check if coordinates are ok */
  if ((static_cast<int>(srcImageHist.shape().first) < _center.first + maxradius) ||
      (_center.first - maxradius < 0) )
    throw out_of_range("pp311:loadSettings: '" + name() + "'. Center in lab X '" +
                       toString(_center.first) + "' and maximum radius '" +
                       toString(maxradius) + "' do not fit in image with width '" +
                       toString(srcImageHist.shape().first) + "'");
  if ((static_cast<int>(srcImageHist.shape().second) < _center.second + maxradius) ||
      (_center.second - maxradius < 0))
    throw out_of_range("pp311:loadSettings: '" + name() + "'. Center in lab Y '" +
                       toString(_center.second) + "' and maximum radius '" +
                       toString(maxradius) + "' do not fit in image with height '" +
                       toString(srcImageHist.shape().second) + "'");
  _maxrad = maxradius;

  createHistList(srcImageHist.copy_sptr());

  Log::add(Log::INFO,"PostProcessor '" + name() +
           "' will calculate the autocorrelation of '" + _hist->name() +
           "'. Condition is '" + _condition->name() + "'");
}

int pp311::getCircleLength(const int rad)
{
  int npix = 0;
  int x = 0;
  int y = rad;
  int f = 1-rad;
  int ddF_x = 0;
  int ddF_y = -2 * rad;
  npix += 4;
  while (x<(y-1))
  {
    if (f>=0)
    {
      --y;
      ddF_y += 2;
      f += ddF_y;
    }
    ++x;
    ddF_x += 2;
    f += ddF_x + 1;
    npix += 4;
    if (x!=y)
      npix += 4;
  }
  return npix;
}

void pp311::fillRing(const HistogramFloatBase::storage_t &image,
                     const int rad, const int x0, const int y0, const int nxx,
                     ring_t &ring)
{
  const int nAngles(ring.size());
  int f = 1-rad;
  int ddF_x = 0;
  int ddF_y = -2 * rad;
  int x = 0;
  int y = rad;
  int angle = 0;
  const int angsym2 = nAngles*2*0.125;
  const int angsym4 = nAngles*4*0.125;
  const int angsym6 = nAngles*6*0.125;
  const int angsym8 = nAngles;

  ring[angle].first= x0 + nxx*(y0+rad);
  ring[angle].second = image[ring[angle].first];

  ring[angle+angsym4].first= x0 + nxx*(y0 - rad);
  ring[angle+angsym4].second = image[ring[angle+angsym4].first];

  ring[angle+angsym2].first= x0 + rad + nxx*(y0);
  ring[angle+angsym2].second = image[ring[angle+angsym2].first];

  ring[angle+angsym6].first= x0 - rad + nxx*(y0);
  ring[angle+angsym6].second = image[ring[angle+angsym6].first];

  while (x < (y-1))
  {
    ++angle;
    if (f>=0)
    {
      --y;
      ddF_y += 2;
      f += ddF_y;
    }
    ++x;
    ddF_x += 2;
    f += ddF_x + 1;

    ring[angle].first= x0 + x + nxx*(y0 + y);
    ring[angle].second = image[ring[angle].first];

    ring[angsym8-angle].first= x0 - x + nxx*(y0 + y);
    ring[angsym8-angle].second = image[ring[angsym8-angle].first];

    ring[angsym4-angle].first= x0 + x + nxx*(y0 - y);
    ring[angsym4-angle].second = image[ring[angsym4-angle].first];

    ring[angsym4+angle].first= x0 - x + nxx*(y0 - y);
    ring[angsym4+angle].second = image[ring[angsym4+angle].first];

    if ( x!=y )
    {
      ring[angsym2-angle].first= x0 + y + nxx*(y0 + x);
      ring[angsym2-angle].second = image[ring[angsym2-angle].first];

      ring[angsym6+angle].first= x0 - y + nxx*(y0 + x);
      ring[angsym6+angle].second = image[ring[angsym6+angle].first];

      ring[angsym2+angle].first= x0 + y + nxx*(y0 - x);
      ring[angsym2+angle].second = image[ring[angsym2+angle].first];

      ring[angsym6-angle].first= x0 - y + nxx*(y0 - x);
      ring[angsym6-angle].second = image[ring[angsym6-angle].first];
    }
  }
}

void pp311::process(const CASSEvent &evt,HistogramBackend &res)
{
  const Histogram2DFloat& hist
      (dynamic_cast<const Histogram2DFloat&>(_hist->result(evt.id())));
  const Histogram2DFloat::storage_t& histdata( hist.memory() );

  Histogram2DFloat &result(dynamic_cast<Histogram2DFloat&>(res));
  Histogram2DFloat::storage_t& resultdata( result.memory() );

  QReadLocker (&hist.lock);
  QWriteLocker(&result.lock);

  const int nxx(hist.shape().first);

  ring_t ring;
  for(int rad=0; rad<_maxrad; ++rad)
  {
    const int ringsize(getCircleLength(rad));
    ring.resize(ringsize);
    fillRing(histdata,rad,_center.first,_center.second,nxx,ring);
#pragma omp parallel for
    for (int d_phi_pix=0; d_phi_pix < ringsize; ++d_phi_pix)
    {
      float result = 0.0;
      for (int ii=0; ii<ringsize; ++ii)
        result += ring[ii].second * ring[(ii+d_phi_pix)%ringsize].second;
      result /= static_cast<float>(ringsize);
      resultdata[ring[d_phi_pix].first] = result;
    }
  }
}
