// Copyright (C) 2013 Lutz Foucar

/**
 * @file tow_d_viewer.h contains the viewer for 2d data
 *
 * @author Lutz Foucar
 */

#ifndef _TWODVIEWER_
#define _TWODVIEWER_

#include <QtCore/QMap>

#include <QtGui/QWidget>

#include <qwt_color_map.h>

namespace cass
{
class Histogram2DFloat;
}//end namespace cass

class QwtPlot;
class QwtLinearColorMap;
class QwtPlotSpectrogram;

namespace jocassview
{
class TwoDViewerData;
class MinMaxControl;
class TrackZoomer2D;

/** a viewer that displays 2d data
 *
 * @author Stephan Kassemeyer
 * @author Lutz Foucar
 */
class TwoDViewer : public QWidget
{
  Q_OBJECT

public:
  /** constructor
   *
   * @param parent The parent of this
   */
  TwoDViewer(QWidget *parent=0);

public slots:
  /** set the data to display
   *
   * @param histogram The histogram that contains the data to display
   */
  void setData(cass::Histogram2DFloat *histogram);

private slots:
  /** replot the data */
  void replot();

private:
  /** The plot area */
  QwtPlot * _plot;

  /** the spectrogram that is used to display the 2d data */
  QwtPlotSpectrogram * _spectrogram;

  /** the wrapper for the 2d data */
  TwoDViewerData *_data;

  /** the color maps */
  QMap<int,QwtLinearColorMap> _maps;

  /** the z-scale control */
  MinMaxControl *_zControl;

  /** a zoomer for the 2d view */
  TrackZoomer2D *_zoomer;
};
}//end namespace jocassview

#endif
