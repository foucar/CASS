// Copyright (C) 2013 Lutz Foucar

/**
 * @file tow_d_viewer.h contains the viewer for 2d data
 *
 * @author Lutz Foucar
 */

#ifndef _TWODVIEWER_
#define _TWODVIEWER_

#include <QtCore/QMap>

#include "data_viewer.h"

#include <qwt_color_map.h>

class QwtPlot;
class QwtLinearColorMap;
class QwtPlotSpectrogram;
class QSpinBox;

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
class TwoDViewer : public DataViewer
{
  Q_OBJECT

public:
  /** constructor
   *
   * @param title the title of this viewer
   * @param parent The parent of this
   */
  TwoDViewer(QString title, QWidget *parent=0);

  /** destructor */
  virtual ~TwoDViewer();

  /** set the data to display
   *
   * @param histogram The histogram that contains the data to display
   */
  void setData(cass::HistogramBackend *histogram);

  /** set the data to display
   *
   * @param histogram The histogram that contains the data to display
   */
  cass::HistogramBackend *data();

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

  /** the colorbar control */
  QSpinBox *_colorId;

  /** a zoomer for the 2d view */
  TrackZoomer2D *_zoomer;
};
}//end namespace jocassview

#endif
