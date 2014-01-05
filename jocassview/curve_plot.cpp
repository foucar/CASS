// Copyright (C) 2014 Lutz Foucar

/**
 * @file curve_plot.cpp contains an alternative curve plot
 *
 * @author Lutz Foucar
 */

#include <QtCore/QDebug>

#include "curve_plot.h"

using namespace jocassview;

PlotCurve::PlotCurve()
{

}

PlotCurve::PlotCurve(const QString &title)
  : QwtPlotCurve(title)
{

}

void PlotCurve::drawSeries(QPainter *painter,
                           const QwtScaleMap &xMap, const QwtScaleMap &yMap,
                           const QRectF &canvasRect,int from, int to) const
{
  qDebug()<<"drawSeries"<<canvasRect<<from<<to;

  QwtPlotCurve::drawSeries(painter,xMap,yMap,canvasRect,from,to);
}
