// Copyright (C) 2013 Lutz Foucar

/**
 * @file track_zoomer_2d.h contains zoomer for a 2d plot with tracking information
 *
 * @author Lutz Foucar
 */

#ifndef _TRACKZOOMER_2D_
#define _TRACKZOOMER_2D_

#include "qwt_scroll_zoomer.h"

class QPoint;
class QStatusBar;

namespace jocassview
{
class TwoDViewerData;

/** class that allows to zoom in a 2d view with tracking information
 *
 * tracks the data values and position
 *
 * @author Stephan Kassemeyer
 * @author Lutz Foucar
 */
class TrackZoomer2D : public ScrollZoomer
{
  Q_OBJECT

public:
  /** constructor
   *
   * @param canvas the canvas which hold the plot
   */
  TrackZoomer2D(QWidget *canvas);

  /** change the tracker text
   *
   * @return the text to be displayed
   * @param pos The position of the mouse
   */
  virtual QwtText trackerTextF(const QPointF &pos) const;

  /** set the data to retrieve the values from
   *
   * @param data the object containing the data
   */
  void setData(TwoDViewerData *data);

  /** set the wavelength that is  needed of the optional resolution calculation
   *
   * @param wavelength_A the wavelength in Angstroem
   */
  void setWavelength_A(double wavelength_A);

  /** set the camera distance that is  needed of the optional resolution calculation
   *
   * @param cameradistance_cm the camera distance in centimeter
   */
  void setCameraDistance_cm(double cameradistance_cm);

  /** set the size of a pixel in micro meters
   *
   * @param pixelsize_um the pixel size in micro meters
   */
  void setPixelSize_um(double pixelsize_um);

  /** set the statusbar pointer
   *
   * @param statusbar pointer to the statusbar to put the text to
   */
  void setStatusBar(QStatusBar *statusbar);

private:
  /** the data */
  TwoDViewerData * _data;

  /** the wavelength in Angstroem (for resolution determination) */
  double _wavelength_A;

  /** the cameraDistance in cm (for resolution determination) */
  double _cameraDistance_cm;

  /** the pixel size in micro meters (for resolution determination) */
  double _pixelsize_um;

  /** pointer to the statusbar to put the text to */
  QStatusBar *_statusbar;
};

}//end namespace jocassview
#endif
