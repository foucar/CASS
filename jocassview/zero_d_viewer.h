// Copyright (C) 2013 Lutz Foucar

/**
 * @file zero_d_viewer.h contains the viewer for 0d data
 *
 * @author Lutz Foucar
 */

#ifndef _ZERODVIEWER_
#define _ZERODVIEWER_

#include <QtGui/QWidget>

namespace cass
{
class Histogram0DFloat;
}//end namespace cass


namespace jocassview
{

/** a viewer that displays 0d data
 *
 * @author Stephan Kassemeyer
 * @author Lutz Foucar
 */
class ZeroDViewer : public QWidget
{
  Q_OBJECT

public:
  /** constructor
   *
   * @param parent The parent of this
   */
  ZeroDViewer(QWidget *parent=0);

public slots:
  /** set the data to display
   *
   * @param histogram The histogram that contains the data to display
   */
  void setData(cass::Histogram0DFloat *histogram);

};
}//end namespace jocassview

#endif
