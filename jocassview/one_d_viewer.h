// Copyright (C) 2013 Lutz Foucar

/**
 * @file one_d_viewer.h contains the viewer for 1d data
 *
 * @author Lutz Foucar
 */

#ifndef _ONEDVIEWER_
#define _ONEDVIEWER_

#include <QtCore/QList>

#include "data_viewer.h"

class QAction;
class QwtPlot;
class QwtPlotCurve;
class QwtPlotGrid;
class QwtLegend;
class QwtPlotItem;

namespace jocassview
{
class MinMaxControl;
class OneDViewerData;

/** a viewer that displays 1d data
 *
 * @author Stephan Kassemeyer
 * @author Lutz Foucar
 */
class OneDViewer : public DataViewer
{
  Q_OBJECT

public:
  /** constructor
   *
   * @param title The title of this view
   * @param parent The parent of this
   */
  OneDViewer(QString title, QWidget *parent=0);

  /** set the data to display
   *
   * @param data The histogram that contains the data to display
   */
  void setData(cass::HistogramBackend *data);

  /** retrieve the displayed data
   *
   * @return pointer to the viewed data
   */
  cass::HistogramBackend *data();

private slots:
  /** add a curve with data to the plot
   *
   * @param histogram the additional curve to be plotted
   */
  void addData(cass::Histogram1DFloat *histogram);

  /** redraw the plot */
  void replot();

  /** react when an legend item has been selected
   *
   * open a context menue at the position of the legenditem.
   *
   * @param pos the position where the context menu should be opened
   */
  void on_legend_right_clicked(QPoint pos);

  /** react when an legend item has been clicked
   *
   * toggle hide / show the curve
   *
   * @param item the plot item assiciated with the legend entry
   */
  void on_legend_clicked(QwtPlotItem *item);

  /** change the curves color
   *
   * if no parameter is given try to retrieve the curve from the sender
   * signal that is connected to this slot. We're assuming this is an QAction
   * whos parent is the curve.
   *
   * @param curve the curve to change is the senders parent widget
   */
  void change_curve_color(QwtPlotCurve *curve=0);

  /** change the curves line width
   *
   * if no parameter is given try to retrieve the curve from the sender
   * signal that is connected to this slot. We're assuming this is an QAction
   * whos parent is the curve.
   *
   * @param curve the curve to change is the senders parent widget
   */
  void change_curve_width(QwtPlotCurve *curve=0);

  /** remove the curve from the plot
   *
   * if no parameter is given try to retrieve the curve from the sender
   * signal that is connected to this slot. We're assuming this is an QAction
   * whos parent is the curve.
   *
   * @param curve the curve to remove from the plot
   */
  void remove_curve(QwtPlotCurve *curve=0);

  /** react when addGraph action has been triggered
   *
   */
  void on_add_graph_triggered();

private:
  /** The plot area */
  QwtPlot * _plot;

  /** control for the x-axis */
  MinMaxControl *_xControl;

  /** control for the y-axis */
  MinMaxControl *_yControl;

  /** this plots curves */
  QList<QwtPlotCurve*> _curves;

  /** this plots curves */
  QList<OneDViewerData*> _curvesData;

  /** an action to control the grid in the plot */
  QAction * _gridControl;

  /** a grid in the plot */
  QwtPlotGrid *_grid;

  /** an action to control the legend of curves */
  QAction * _legendControl;

  /** a legend for the different curves */
  QwtLegend *_legend;
};
}//end namespace jocassview

#endif
