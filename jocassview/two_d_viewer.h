// Copyright (C) 2013 Lutz Foucar

/**
 * @file tow_d_viewer.h contains the viewer for 2d data
 *
 * @author Lutz Foucar
 */

#ifndef _TWODVIEWER_
#define _TWODVIEWER_

#include <QtGui/QWidget>

namespace cass
{
class Histogram2DFloat;
}//end namespace cass

namespace jocassview
{

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

};
}//end namespace jocassview

#endif
