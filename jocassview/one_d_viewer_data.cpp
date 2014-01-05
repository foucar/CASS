// Copyright (C) 2013 Lutz Foucar

/**
 * @file one_d_viewer_data.cpp contains the wrapper of the data for the 1d viewer
 *
 * @author Lutz Foucar
 */

#include <QtCore/QDebug>

#include "one_d_viewer_data.h"

using namespace jocassview;

OneDViewerData::OneDViewerData()
  : _xRange(QwtInterval(0,1)),
    _yRange(QwtInterval(0,1)),
    _name("")
{

}

size_t OneDViewerData::size() const
{
  return _data.size();
}

QPointF OneDViewerData::sample(size_t i) const
{
  const qreal x(_xRange.minValue() + i*(_xRange.maxValue()-_xRange.minValue())/(size()-1));
  const qreal y(_data[i]);
  return QPointF(x,y);
}

QRectF OneDViewerData::boundingRect() const
{
  return QRectF(_xRange.minValue(),_yRange.minValue(),
                _xRange.maxValue() - _xRange.minValue(),
                _yRange.maxValue() - _yRange.minValue());
}

void OneDViewerData::setData(const std::vector<float> &data, const QwtInterval &xRange)
{
  _data = data;
  _xRange = xRange;
  double ymin( 1e30);
  double ymax(-1e30);
  for (std::vector<float>::const_iterator it(_data.begin()); it != _data.end()-2; ++it)
  {
    if (std::isnan(*it) || std::isinf(*it))
      continue;
    if (*it < ymin)
      ymin = *it;
    if (ymax < *it)
      ymax = *it;
  }
  _yRange = QwtInterval(ymin,ymax);
}
