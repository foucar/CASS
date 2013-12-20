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


namespace jocassview
{

/** a viewer that displays 2d data
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

};
}//end namespace jocassview

#endif
