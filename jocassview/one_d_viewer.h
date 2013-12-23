// Copyright (C) 2013 Lutz Foucar

/**
 * @file one_d_viewer.h contains the viewer for 1d data
 *
 * @author Lutz Foucar
 */

#ifndef _ONEDVIEWER_
#define _ONEDVIEWER_

#include <QtGui/QWidget>

namespace cass
{
class Histogram1DFloat;
}//end namespace cass

class QwtPlot;

namespace jocassview
{
class MinMaxControl;

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

public slots:
  /** set the data to display
   *
   * @param histogram The histogram that contains the data to display
   */
  void setData(cass::Histogram1DFloat *histogram);

private:
  /** The plot area */
  QwtPlot * _plot;

  /** control for the x-axis */
  MinMaxControl *_xControl;

  /** control for the y-axis */
  MinMaxControl *_yControl;
};
}//end namespace jocassview

#endif
