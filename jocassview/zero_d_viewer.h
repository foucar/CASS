// Copyright (C) 2013 Lutz Foucar

/**
 * @file zero_d_viewer.h contains the viewer for 0d data
 *
 * @author Lutz Foucar
 */

#ifndef _ZERODVIEWER_
#define _ZERODVIEWER_

#include "data_viewer.h"

namespace cass
{
class Histogram0DFloat;
}//end namespace cass
class QLabel;

namespace jocassview
{

/** a viewer that displays 0d data
 *
 * @author Stephan Kassemeyer
 * @author Lutz Foucar
 */
class ZeroDViewer : public DataViewer
{
  Q_OBJECT

public:
  /** constructor
   *
   * @param title the title of this viewer
   * @param parent The parent of this
   */
  ZeroDViewer(QString title, QWidget *parent=0);

  /** destructor */
  virtual ~ZeroDViewer();

  /** set the data to display
   *
   * @param histogram The histogram that contains the data to display
   */
  void setData(cass::HistogramBackend *histogram);

  /** retrieve the displayed data
   *
   * @return The histogram that contains the displayed data
   */
  cass::HistogramBackend* data();

  /** set the data to display
   *
   * @param float The value to be displayed
   */
  void setData(float value);

  /** retrieve the type of the data viewer
   *
   * @return the type as name
   */
  QString type() const;

private:
  /** the label that displays the value */
  QLabel *_value;

  /** pointer to the histogram conatining the 0d value */
  cass::Histogram0DFloat *_hist;

};
}//end namespace jocassview

#endif
