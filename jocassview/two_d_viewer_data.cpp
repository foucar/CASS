// Copyright (C) 2013 Lutz Foucar

/**
 * @file two_d_viewer_data.cpp contains the wrappe of the data for the 2d viewer
 *
 * @author Lutz Foucar
 */

#include <algorithm>

#include <QtCore/QDebug>

#include "two_d_viewer_data.h"

#include "result.hpp"

using namespace jocassview;
using namespace cass;

TwoDViewerData::TwoDViewerData()
{

}

TwoDViewerData::~TwoDViewerData()
{

}

void TwoDViewerData::setResult(result_t::shared_pointer result)
{
  if (!result)
    return;

  _result = result;
  const result_t::axe_t &xaxis(_result->axis(result_t::xAxis));
  setInterval(Qt::XAxis,QwtInterval(xaxis.low,xaxis.up));
  const result_t::axe_t &yaxis(_result->axis(result_t::yAxis));
  setInterval(Qt::YAxis,QwtInterval(yaxis.low,yaxis.up));
  setInterval(Qt::ZAxis,origZInterval(false));
  _wasUpdated = true;
}

Data::result_t::shared_pointer TwoDViewerData::result()
{
  _wasUpdated = false;
  return _result;
}

QwtInterval TwoDViewerData::origZInterval(bool log)const
{
  if (!_result)
    return (QwtInterval(0,0));

  QwtInterval zRange(1e30,-1e30);
  result_t::const_iterator it(_result->begin());
  result_t::const_iterator End(_result->end());
  for (;it != End;++it)
  {
    if (!std::isfinite(*it))
      continue;
    if (log && !std::isfinite(log10(*it)))
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
  if(!_result)
    return 0;

  const int xSize(_result->shape().first);
  const int xMin(interval(Qt::XAxis).minValue());
  const int xWidth(interval(Qt::XAxis).width());
  const int binx(xSize  * (x - xMin) / xWidth);
  if (binx < 0 || xSize <= binx)
    return 0;

  const int ySize(_result->shape().second);
  const int yMin(interval(Qt::YAxis).minValue());
  const int yWidth(interval(Qt::YAxis).width());
  const int biny(ySize  * (y - yMin) / yWidth);
  if (biny < 0 || ySize <= biny)
    return 0;

  const int globalbin(biny*xSize + binx);
  return (*_result)[globalbin];
}
