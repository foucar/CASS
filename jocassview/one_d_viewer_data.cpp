// Copyright (C) 2013 Lutz Foucar

/**
 * @file one_d_viewer_data.cpp contains the wrapper of the data for the 1d viewer
 *
 * @author Lutz Foucar
 */

#include <QtCore/QDebug>

#include "one_d_viewer_data.h"

#include "histogram.h"

using namespace jocassview;
using namespace cass;

OneDViewerData::OneDViewerData()
  : _hist(0)
{

}

OneDViewerData::~OneDViewerData()
{
  delete _hist;
}

size_t OneDViewerData::size() const
{
  return result()->axis()[Histogram1DFloat::xAxis].nbrBins();
}

QPointF OneDViewerData::sample(size_t i) const
{
  const qreal xMin(d_boundingRect.left());
  const qreal xWidth(d_boundingRect.width());
  const qreal x(xMin + i*xWidth/(size()-1));
  const qreal y(_hist->memory()[i]);
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

void OneDViewerData::setResult(HistogramBackend *hist)
{
  if(!hist || dynamic_cast<Histogram1DFloat*>(hist) == _hist)
    return;
  delete _hist;
  _hist = dynamic_cast<Histogram1DFloat*>(hist);

  /** set the initial bounding rect in x */
  const AxisProperty &xaxis(result()->axis()[Histogram1DFloat::xAxis]);
  d_boundingRect.setLeft(xaxis.lowerLimit());
  d_boundingRect.setRight(xaxis.upperLimit());

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

HistogramBackend* OneDViewerData::result()
{
  return _hist;
}

const HistogramBackend* OneDViewerData::result()const
{
  return _hist;
}

void OneDViewerData::setXRangeForLog(bool log)
{
  _xLog = log;
}

void OneDViewerData::setYRangeForLog(bool log)
{
  _yLog = log;
}
