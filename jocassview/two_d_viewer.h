// Copyright (C) 2013 Lutz Foucar

/**
 * @file two_d_viewer.h contains the viewer for 2d data
 *
 * @author Lutz Foucar
 */

#ifndef _TWODVIEWER_
#define _TWODVIEWER_

#include <QtCore/QMap>

#include "data_viewer.h"

class QwtPlot;
class QwtLinearColorMap;
class QwtPlotSpectrogram;
class QSpinBox;
class QStringList;

namespace cass
{
class Histogram2DFloat;
}

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
  virtual QList<Data*> data();

  /** retrieve the type of the data viewer
   *
   * @return the type as name
   */
  virtual QString type() const;

  /** save the data to file
   *
   * @param filename the file name to save the data to
   */
  virtual void saveData(const QString &filename);

  /** update the plot
   *
   * check if the zoomer is still valid and replot the plot
   */
  virtual void dataChanged();

  /** suffixes for the data of this viewer
   *
   * @return suffixes for the data of this viewer
   */
  virtual QStringList dataFileSuffixes() const;

private slots:
  /** replot the data */
  void replot();

  /** load the geom file */
  void on_load_geomfile_triggered();

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
   * @param log if true a log scale color map will be returned
   */
  QwtLinearColorMap* cmap(const int mapId, bool log) const;

  /** return the list of possible colormaps
   *
   * @return list with possible colormap ids
   */
  QStringList cmaps()const;

private:
  /** the spectrogram that is used to display the 2d data */
  QwtPlotSpectrogram * _spectrogram;

  /** the z-scale control */
  MinMaxControl *_zControl;

  /** the colorbar control */
  QSpinBox *_colorId;

  /** an action to control the legend of curves */
  QAction * _axisTitleControl;

  /** a zoomer for the 2d view */
  TrackZoomer2D *_zoomer;

  /** the geom file to convert data to lab frame */
  QString _geomFile;

  /** the original histogram */
  cass::Histogram2DFloat *_origHist;

  /** flag to tell whether the data is the original data */
  bool _isOriginalData;
};
}//end namespace jocassview

#endif
