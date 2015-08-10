// Copyright (C) 2013 Lutz Foucar

/**
 * @file one_d_viewer_data.cpp contains the wrapper of the data for the 1d viewer
 *
 * @author Lutz Foucar
 */

#include <QtCore/QDebug>

#include "one_d_viewer_data.h"

#include "result.hpp"

using namespace jocassview;
using namespace cass;

OneDViewerData::OneDViewerData()
  : _logMinPos(QPointF(1,1)),
    _xLog(false),
    _yLog(false)
{
  d_boundingRect = QRectF(1.0, 1.0, -2.0, -2.0); //invalid
}

OneDViewerData::~OneDViewerData()
{

}

size_t OneDViewerData::size() const
{
  return result() ? result()->shape().first : 0;
}

QPointF OneDViewerData::sample(size_t i) const
{
  if (!result())
    return QPointF(0,0);

  const qreal xMin(d_boundingRect.left());
  const qreal xWidth(d_boundingRect.width());
  const qreal x(xMin + i*xWidth/(size()-1));
  const qreal y((*result())[i]);
  //const qreal y(res[i]);
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

void OneDViewerData::setResult(result_t::shared_pointer result)
{
  if(!result)
    return;
  _result = result;

  /** set the initial bounding rect in x */
  const result_t::axe_t &xaxis(_result->axis(result_t::xAxis));
  d_boundingRect.setLeft(xaxis.low);
  d_boundingRect.setRight(xaxis.up);

  /** go through all data points of the curve and find min/max values for lin
   *  and log scale purposes
   */
  d_boundingRect.setTop(1e30);
  d_boundingRect.setBottom(-1e30);
  _logMinPos.setX(1e30);
  _logMinPos.setY(1e30);
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

Data::result_t::shared_pointer OneDViewerData::result()
{
  return _result;
}

const Data::result_t::shared_pointer OneDViewerData::result()const
{
  return _result;
}

void OneDViewerData::setXRangeForLog(bool log)
{
  _xLog = log;
}

void OneDViewerData::setYRangeForLog(bool log)
{
  _yLog = log;
}
