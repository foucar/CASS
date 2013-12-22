// Copyright (C) 2013 Lutz Foucar

/**
 * @file tow_d_viewer_data.cpp contains the wrappe of the data for the 2d viewer
 *
 * @author Lutz Foucar
 */

#include <algorithm>

#include <QtCore/QDebug>

#include "two_d_viewer_data.h"

using namespace jocassview;
using namespace std;

TwoDViewerData::TwoDViewerData()
  : QwtRasterData(QwtDoubleRect(-100.,-200.,400.,300.)),
    _xRange(QwtDoubleInterval(0.,1.)),
    _yRange(QwtDoubleInterval(0.,1.)),
    _zRange(QwtDoubleInterval(0.,1.)),
    _shape(0,0),
    _name("")
{

}

TwoDViewerData::TwoDViewerData(const TwoDViewerData &other)
  : QwtRasterData(other.boundingRect()),
    _xRange(other._xRange),
    _yRange(other._yRange),
    _zRange(other._zRange),
    _data(other._data),
    _shape(other._shape),
    _name(other._name)
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
  setBoundingRect(QwtDoubleRect(_xRange.minValue(),_yRange.minValue(),
                                _xRange.maxValue()-_xRange.minValue(),
                                _yRange.maxValue()-_yRange.minValue()));
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

QwtDoubleInterval TwoDViewerData::zRange() const
{
  return QwtDoubleInterval(*min_element(_data.begin(),_data.end()),
                           *max_element(_data.begin(),_data.end()));
}

QwtRasterData * TwoDViewerData::copy() const
{
  return new TwoDViewerData(*this);
}

void TwoDViewerData::setName(const QString &name)
{
  _name = name;
}

QString TwoDViewerData::name()const
{
  return _name;
}
