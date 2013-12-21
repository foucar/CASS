// Copyright (C) 2013 Lutz Foucar

/**
 * @file tow_d_viewer_data.cpp contains the wrappe of the data for the 2d viewer
 *
 * @author Lutz Foucar
 */

#include <algorithm>

#include "two_d_viewer_data.h"

using namespace jocassview;

TwoDViewerData::TwoDViewerData()
  : QwtRasterData(QwtDoubleRect(0.,0.,500.,500.)),
    _xRange(QwtDoubleInterval(0.,1.)),
    _yRange(QwtDoubleInterval(0.,1.)),
    _zRange(QwtDoubleInterval(0.,1.)),
    _shape(0,0)
{

}

TwoDViewerData::TwoDViewerData(const std::vector<float> &data,  const shape_t &shape,
                               const QwtDoubleInterval &xrange, const QwtDoubleInterval &yrange,
                               const QwtDoubleInterval &zrange, const QwtDoubleRect &boundingRect)
  : QwtRasterData(boundingRect),
    _xRange(xrange),
    _yRange(yrange),
    _zRange(zrange),
    _data(data),
    _shape(shape)
{

}

void TwoDViewerData::setData(const std::vector<float> &data, const shape_t &shape,
                             const QwtDoubleInterval &xrange, const QwtDoubleInterval &yrange)
{
  _data = data;
  _shape = shape;
  _xRange = xrange;
  _yRange = yrange;
  _zRange = QwtDoubleInterval(*min_element(data.begin(),data.end()),
                              *max_element(data.begin(),data.end()));
  setBoundingRect(QwtDoubleRect(_xRange.minValue(),_yRange.maxValue(),
                                _xRange.maxValue(),_yRange.minValue()));
}

void TwoDViewerData::setZRange(const QwtDoubleInterval &zrange)
{
  _zRange = zrange;
}

double TwoDViewerData::value(double x, double y) const
{
  const int binx(_shape.first  * ((x - _xRange.minValue()) / (_xRange.maxValue() - _xRange.minValue())));
  const int biny(_shape.second * ((y - _yRange.minValue()) / (_yRange.maxValue() - _yRange.minValue())));
  const int globalbin(biny * _shape.first + binx);
  return (globalbin < 0 || _data.size() <= globalbin) ? 0. : _data[globalbin];
}

QwtDoubleInterval TwoDViewerData::range() const
{
  return _zRange;
}

QwtRasterData * TwoDViewerData::copy() const
{
  return new TwoDViewerData(_data,_shape,_xRange, _yRange, _zRange,boundingRect());
}
