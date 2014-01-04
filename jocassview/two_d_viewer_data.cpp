// Copyright (C) 2013 Lutz Foucar

/**
 * @file two_d_viewer_data.cpp contains the wrappe of the data for the 2d viewer
 *
 * @author Lutz Foucar
 */

#include <algorithm>

#include <QtCore/QDebug>

#include "two_d_viewer_data.h"

using namespace jocassview;
using namespace std;

void TwoDViewerData::setData(const std::vector<float> &data, const shape_t &shape,
                             const QwtInterval &xrange, const QwtInterval &yrange)
{
  _data = data;
  _shape = shape;
  setInterval(Qt::XAxis,xrange);
  setInterval(Qt::YAxis,yrange);
  setInterval(Qt::ZAxis,origZInterval());
}

QwtInterval TwoDViewerData::origZInterval()const
{
 return QwtInterval(*min_element(_data.begin(),_data.end()),
                    *max_element(_data.begin(),_data.end()));

}

double TwoDViewerData::value(double x, double y) const
{
  const QwtInterval xRange(interval(Qt::XAxis));
  const QwtInterval yRange(interval(Qt::YAxis));
  const int binx(_shape.first  * ((x - xRange.minValue()) / (xRange.maxValue() - xRange.minValue())));
  const int biny(_shape.second * ((y - yRange.minValue()) / (yRange.maxValue() - yRange.minValue())));
  const int globalbin(biny * _shape.first + binx);
  return (globalbin < 0 || _data.size() <= globalbin) ? 0. : _data[globalbin];
}
