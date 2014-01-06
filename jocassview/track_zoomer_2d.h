// Copyright (C) 2013 Lutz Foucar

/**
 * @file track_zoomer_2d.h contains zoomer for a 2d plot with tracking information
 *
 * @author Lutz Foucar
 */

#ifndef _TRACKZOOMER_2D_
#define _TRACKZOOMER_2D_

#include "qwt_scroll_zoomer.h"

class QwtRasterData;
class QPoint;

namespace jocassview
{
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
  virtual QwtText trackerText(const QPoint &pos) const;

  /** set the data to retrieve the values from
   *
   * @param data the object containing the data
   */
  void setData(QwtRasterData *data);

private:
  /** the data */
  QwtRasterData * _data;
};

}//end namespace jocassview
#endif
