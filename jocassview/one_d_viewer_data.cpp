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
  const qreal xMin(d_boundingRect.left());
  const qreal xWidth(d_boundingRect.width());
  const qreal x(xMin + i*xWidth/(size()-1));
  const qreal y(_data[i]);
  return QPointF(x,y);
}

QRectF OneDViewerData::boundingRect() const
{
  QRectF rect(d_boundingRect);
  if (_xLog)
    rect.setLeft(_logMinPos.x());
  if (_yLog)
    rect.setTop(_logMinPos.y());
  return rect;
}

void OneDViewerData::setData(const std::vector<float> &data, const QwtInterval &xRange)
{
  _data = data;
  d_boundingRect.setLeft(xRange.minValue());
  d_boundingRect.setRight(xRange.maxValue());
  d_boundingRect.setTop(1e30);
  d_boundingRect.setBottom(-1e30);
  _logMinPos.setX(1e30);
  _logMinPos.setY(1e30);

  /** go through all data points of the curve and find min/max values for lin
   *  and log scale purposes
   */
  for (size_t i(0); i < size(); ++i)
  {
    const qreal x(sample(i).x());
    const qreal y(sample(i).y());

    /** skip the check if the either coordinate of the point is not a number */
    if (!std::isfinite(x) || !std::isfinite(y))
      continue;

    /** find the max y value */
    if (d_boundingRect.bottom() < y)
      d_boundingRect.setBottom(y);

    /** find the min y value */
    if (y < d_boundingRect.top())
      d_boundingRect.setTop(y);

    /** find the min y value that is positive */
    if (y < _logMinPos.y() && 0 < y)
      _logMinPos.setY(y);

    /** find the min x value that is positive */
    if (x < _logMinPos.x() && 0 < x)
      _logMinPos.setX(x);
  }
}

void OneDViewerData::setXRangeForLog(bool log)
{
  _xLog = log;
}

void OneDViewerData::setYRangeForLog(bool log)
{
  _yLog = log;
}
