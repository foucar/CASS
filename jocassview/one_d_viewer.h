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

namespace cass
{
class Histogram1DFloat;
}//end namespace cass
class QAction;
class QwtPlot;
class QwtPlotGrid;
class QwtLegend;
class QwtPlotItem;

namespace jocassview
{
class MinMaxControl;
class OneDViewerData;
class PlotCurve;

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

  /** destructor */
  virtual ~OneDViewer();

  /** retrieve the displayed data
   *
   * @return pointer to the viewed data
   */
  virtual QList<Data*> data();

  /** retrieve the type of the data viewer
   *
   * @return the type as name
   */
  virtual QString type() const;

  /** save the data to file
   *
   * @param filename the filename to save the data to
   */
  virtual void saveData(const QString &filename);

  /** update the plot */
  virtual void dataChanged();

  /** suffixes for the data of this viewer
   *
   * @return suffixes for the data of this viewer
   */
  virtual QStringList dataFileSuffixes() const;

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
   * @param on when true the associated curve will be shown
   */
  void on_legend_checked(const QVariant &item, bool on);

  /** change the curves color
   *
   * if no parameter is given try to retrieve the curve from the sender
   * signal that is connected to this slot. We're assuming this is an QAction
   * whos parent is the curve.
   *
   * @param curve the curve to change is the senders parent widget
   */
  void change_curve_color(PlotCurve *curve=0);

  /** change the curves line width
   *
   * if no parameter is given try to retrieve the curve from the sender
   * signal that is connected to this slot. We're assuming this is an QAction
   * whos parent is the curve.
   *
   * @param curve the curve to change is the senders parent widget
   */
  void change_curve_width(PlotCurve *curve=0);

  /** remove the curve from the plot
   *
   * if no parameter is given try to retrieve the curve from the sender
   * signal that is connected to this slot. We're assuming this is an QAction
   * whos parent is the curve.
   *
   * @param curve the curve to remove from the plot
   */
  void remove_curve(PlotCurve *curve=0);

  /** react when addGraph action has been triggered
   *
   * use the openfile dialog to get the graph. Then use the FileHandler to
   * retrieve the data form the file.
   */
  void on_add_graph_triggered();

  /** react on when the gridControl has been triggered
   *
   * increase the the _gridLines variable and mask off the unneeded bits
   */
  void on_grid_triggered();

private:
  /** control for the x-axis */
  MinMaxControl *_xControl;

  /** control for the y-axis */
  MinMaxControl *_yControl;

  /** this plots curves */
  QList<PlotCurve*> _curves;

  /** this plots curves */
  QList<OneDViewerData*> _curvesData;

  /** an action to control the grid in the plot */
  QAction * _gridControl;

  /** a grid in the plot */
  QwtPlotGrid *_grid;

  /** flags to tell which grid lines should be drawn */
  quint8 _gridLines;

  /** an action to control the legend of curves */
  QAction * _legendControl;

  /** a legend for the different curves */
  QwtLegend *_legend;

  /** an action to control the legend of curves */
  QAction * _axisTitleControl;
};
}//end namespace jocassview

#endif
