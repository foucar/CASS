// Copyright (C) 2013 Lutz Foucar

/**
 * @file one_d_viewer.h contains the viewer for 1d data
 *
 * @author Lutz Foucar
 */

#ifndef _ONEDVIEWER_
#define _ONEDVIEWER_

#include <QtCore/QList>

#include <QtGui/QWidget>

namespace cass
{
class Histogram1DFloat;
}//end namespace cass

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
class OneDViewer : public QWidget
{
  Q_OBJECT

public:
  /** constructor
   *
   * @param parent The parent of this
   */
  OneDViewer(QWidget *parent=0);

signals:
  /** emit when add graph has been clicked */
  void add_graph_triggered();

public slots:
  /** set the data to display
   *
   * @param histogram The histogram that contains the data to display
   */
  void setData(cass::Histogram1DFloat *histogram);

  /** add a curve with data to the plot
   *
   * @param histogram the additional curve to be plotted
   */
  void addData(cass::Histogram1DFloat *histogram);

private slots:
  /** redraw the plot */
  void replot();

  /** react when an legend item has been selected
   *
   * open a context menue at the position of the legenditem.
   *
   * @param pos the position where the context menu should be opened
   */
  void on_legend_right_clicked(QPoint pos);

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
