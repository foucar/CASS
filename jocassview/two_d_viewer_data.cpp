// Copyright (C) 2013 Lutz Foucar

/**
 * @file two_d_viewer_data.cpp contains the wrappe of the data for the 2d viewer
 *
 * @author Lutz Foucar
 */

#include <algorithm>

#include <QtCore/QDebug>

#include "two_d_viewer_data.h"

#include "histogram.h"

using namespace jocassview;
using namespace cass;

TwoDViewerData::TwoDViewerData()
  : _hist(0)
{

}

TwoDViewerData::~TwoDViewerData()
{
  delete _hist;
}

void TwoDViewerData::setResult(HistogramBackend *hist)
{
  if (!hist || dynamic_cast<Histogram2DFloat*>(hist) == _hist)
    return;

  delete _hist;
  _hist = dynamic_cast<Histogram2DFloat*>(hist);
  const AxisProperty &xaxis(result()->axis()[Histogram2DFloat::xAxis]);
  setInterval(Qt::XAxis,QwtInterval(xaxis.lowerLimit(),xaxis.upperLimit()));
  const AxisProperty &yaxis(result()->axis()[Histogram2DFloat::yAxis]);
  setInterval(Qt::YAxis,QwtInterval(yaxis.lowerLimit(),yaxis.upperLimit()));
  setInterval(Qt::ZAxis,origZInterval());
}

HistogramBackend* TwoDViewerData::result()
{
  return _hist;
}

const HistogramBackend* TwoDViewerData::result()const
{
  return _hist;
}

QwtInterval TwoDViewerData::origZInterval()const
{
  if (!_hist)
    return (QwtInterval(0,0));

  QwtInterval zRange(1e30,-1e30);
  Histogram2DFloat::storage_t::const_iterator it(_hist->memory().begin());
  Histogram2DFloat::storage_t::const_iterator End(_hist->memory().end());
  for (;it != End;++it)
  {
    if (!std::isfinite(*it))
      continue;
    if (*it < zRange.minValue())
      zRange.setMinValue(*it);
    if (zRange.maxValue() < *it)
      zRange.setMaxValue(*it);
  }
  return zRange;
}

double TwoDViewerData::value(double x, double y) const
{
  if(!_hist)
    return 0;

  const int xSize(_hist->shape().first);
  const int xMin(interval(Qt::XAxis).minValue());
  const int xWidth(interval(Qt::XAxis).width());
  const int binx(xSize  * (x - xMin) / xWidth);

  const int ySize(_hist->shape().second);
  const int yMin(interval(Qt::YAxis).minValue());
  const int yWidth(interval(Qt::YAxis).width());
  const int biny(ySize  * (y - yMin) / yWidth);

  const int globalbin(biny*xSize + binx);
  const Histogram2DFloat::storage_t &d(_hist->memory());
  return (globalbin < 0 || static_cast<int>(d.size()) <= globalbin) ? 0. : d[globalbin];
}
