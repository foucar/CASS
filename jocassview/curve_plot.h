// Copyright (C) 2014 Lutz Foucar

/**
 * @file curve_plot.h contains an alternative curve plot
 *
 * @author Lutz Foucar
 */

#ifndef _CURVEPLOT_
#define _CURVEPLOT_

#include <qwt_plot_curve.h>

class Qstring;

namespace jocassview
{
/** an alternative curve plot
 *
 * is an qwt plot curve but evaluates whether the point to be drawn is invalid
 * if so it just skips these parts of the curve.
 *
 * @author Stephan Kassemeyer
 * @author Lutz Foucar
 */
class PlotCurve : public QwtPlotCurve
{
public:
  /** default constructor */
  PlotCurve();

  /** constructor
   *
   * @param title The title of the curve
   */
  PlotCurve(const QString &title);

  /** draw the curve
   *
   * uses the base class draw, but only for areas hat have valid values
   *
   * @param painter
   * @param xMap
   * @param yMap
   * @param canvasRect
   * @param from
   * @param to
   */
  void drawSeries(QPainter *painter,
                  const QwtScaleMap &xMap, const QwtScaleMap &yMap,
                  const QRectF &canvasRect, int from, int to) const;
};

}//end namespace jocassview
#endif
