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
class QStringList;

namespace jocassview
{
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

  /** retrieve the type of the data viewer
   *
   * @return the type as name
   */
  QString type() const;

private slots:
  /** replot the data */
  void replot();

private:
  /** retrieve a color id
   *
   * the colormap needs be created on the heap and a pointer is then passed to
   * the functions who will take over ownership and destroy the allocated space.
   *
   * In case the mapId is unknown a standart black & white map is returned
   *
   * @return pointer to the requested colormap
   * @param mapId the key of the requested color id
   */
  QwtLinearColorMap* cmap(const int mapId) const;

  /** return the list of possible colormaps
   *
   * @return list with possible colormap ids
   */
  QStringList cmaps()const;

private:
  /** The plot area */
  QwtPlot * _plot;

  /** the spectrogram that is used to display the 2d data */
  QwtPlotSpectrogram * _spectrogram;

  /** the z-scale control */
  MinMaxControl *_zControl;

  /** the colorbar control */
  QSpinBox *_colorId;

  /** a zoomer for the 2d view */
  TrackZoomer2D *_zoomer;
};
}//end namespace jocassview

#endif
