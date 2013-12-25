// Copyright (C) 2013 Lutz Foucar

/**
 * @file one_d_viewer_data.cpp contains the wrapper of the data for the 1d viewer
 *
 * @author Lutz Foucar
 */

#include "one_d_viewer_data.h"

using namespace jocassview;

OneDViewerData::OneDViewerData()
  : QwtData(),
    _xRange(QwtDoubleInterval(0,1)),
    _yRange(QwtDoubleInterval(0,1)),
    _name("")
{

}

OneDViewerData::OneDViewerData(const OneDViewerData &other)
  : QwtData(),
    _data(other._data),
    _xRange(other._xRange),
    _yRange(other._yRange),
    _name(other._name)
{

}

QwtData * OneDViewerData::copy() const
{
  return new OneDViewerData(*this);
}

size_t OneDViewerData::size() const
{
  return _data.size();
}

double OneDViewerData::x(size_t i) const
{
  return _xRange.minValue() + i*(_xRange.maxValue()-_xRange.minValue()/(size()-1));
}

double OneDViewerData::y(size_t i) const
{
  return (std::isnan(_data[i]) || std::isinf(_data[i])) ? 0 : _data[i];
}

QwtDoubleRect OneDViewerData::boundingRect() const
{
  return QwtDoubleRect(_xRange.minValue(),_yRange.minValue(),
                       _xRange.maxValue() - _xRange.minValue(),
                       _yRange.minValue() - _yRange.minValue());
}
