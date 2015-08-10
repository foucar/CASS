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
#include "cass_event.h"
#include "cass_settings.h"
#include "log.h"

using namespace cass;
using namespace std;


pp310::pp310(const name_t &name)
  : Processor(name)
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
  const result_t &srcImageHist(_hist->result());
  if (srcImageHist.dim() != 2)
    throw invalid_argument("pp310:setup: '" + name() +
                           "' The input histogram is not a 2d histogram");
  createHistList(srcImageHist.clone());

  Log::add(Log::INFO,"Processor '" + name() +
           "' will calculate the autocorrelation of '" + _hist->name() +
           "'. Condition is '" + _condition->name() + "'");
}

void pp310::process(const CASSEvent &evt, result_t &result)
{
  typedef result_t::const_iterator const_iterator;
  const result_t& hist(_hist->result(evt.id()));
  QReadLocker lock2(&hist.lock);

  size_t nCols(hist.axis(result_t::xAxis).nBins);
  size_t nRows(hist.axis(result_t::yAxis).nBins);

  result_t::storage_t row_buffer(nCols);
  /** go through each row (radius) of input */
  for (size_t row(0); row < nRows; ++row)
  {
    const_iterator rowStart(hist.begin() + row*nCols);
    const_iterator rowEnd(hist.begin() + row*nCols + nCols);
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
      result[row*nCols + col] = autocorval;
    }
  }
}



//*** autocorrelation from image in kartesian coordinates ***
pp311::pp311(const name_t &name)
  : Processor(name)
{
  loadSettings(0);
}

void pp311::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("Processor");
  s.beginGroup(QString::fromStdString(name()));
  setupGeneral();
  _hist = setupDependency("HistName");
  bool ret (setupCondition());
  if (!(ret && _hist))
    return;

  std::pair<int,int> usercenter = make_pair(s.value("CenterX",0).toInt(),
                                            s.value("CenterY",0).toInt());
  const int maxradius(s.value("MaximumRadius",0).toInt());

  const result_t &srcImageHist(_hist->result());
  if (srcImageHist.dim() != 2)
    throw invalid_argument("pp311:setup: '" + name() +
                           "' The input histogram is not a 2d histogram");
  /** determine the center in histogram coordinates */
  const result_t::axe_t &xaxis(srcImageHist.axis(result_t::xAxis));
  const result_t::axe_t &yaxis(srcImageHist.axis(result_t::yAxis));
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

  createHistList(srcImageHist.clone());

  Log::add(Log::INFO,"Processor '" + name() +
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

void pp311::fillRing(const result_t &image,
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

void pp311::process(const CASSEvent &evt, result_t &result)
{
  const result_t& hist(_hist->result(evt.id()));
  QReadLocker lock1(&hist.lock);

  const int nxx(hist.shape().first);

  ring_t ring;
  for(int rad=0; rad<_maxrad; ++rad)
  {
    const int ringsize(getCircleLength(rad));
    ring.resize(ringsize);
    fillRing(hist,rad,_center.first,_center.second,nxx,ring);
#ifdef _OPENMP
#pragma omp parallel for
#endif
    for (int d_phi_pix=0; d_phi_pix < ringsize; ++d_phi_pix)
    {
      float res = 0.0;
      for (int ii=0; ii<ringsize; ++ii)
        res += ring[ii].second * ring[(ii+d_phi_pix)%ringsize].second;
      res /= static_cast<float>(ringsize);
      result[ring[d_phi_pix].first] = res;
    }
  }
}
