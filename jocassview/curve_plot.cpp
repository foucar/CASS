// Copyright (C) 2014 Lutz Foucar

/**
 * @file curve_plot.cpp contains an alternative curve plot
 *
 * @author Lutz Foucar
 */

#include <cmath>

#include <QtCore/QDebug>

#include <qwt_scale_map.h>

#include "curve_plot.h"

using namespace jocassview;

PlotCurve::PlotCurve()
{

}

PlotCurve::PlotCurve(const QString &title)
  : QwtPlotCurve(title)
{

}

/** extract point coordinates and check if they are valid
 *
 * @return true when both coordinates are finite numbers
 * @param sample the point to check
 * @param xMap
 * @param yMap
 *
 * @author Lutz Foucar
 */
bool validate(const QPointF& sample,
              const QwtScaleMap &xMap, const QwtScaleMap &yMap)
{
    double xi(xMap.transform(sample.x()));
    double yi(yMap.transform(sample.y()));
    return (std::isfinite(xi) && std::isfinite(yi));

}

void PlotCurve::drawSeries(QPainter *painter,
                           const QwtScaleMap &xMap, const QwtScaleMap &yMap,
                           const QRectF &canvasRect,int from, int to) const
{
  if (to < 0)
    to = dataSize() - 1;
  int i(from);
  while(i<=to)
  {
    /** find the first point that is valid */
    while(!validate(data()->sample(i),xMap,yMap) && i <= to)
      ++i;
    const int firstValid(i);

    /** find the last point that is valid */
    while(validate(data()->sample(i),xMap,yMap) && i <= to)
      ++i;
    const int lastValid(i-1);

    /** plot that valid subrange */
    QwtPlotCurve::drawSeries(painter,xMap,yMap,canvasRect,firstValid,lastValid);
  }

}
