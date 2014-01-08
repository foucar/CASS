// Copyright (C) 2013 Lutz Foucar

/**
 * @file zero_d_viewer.h contains the viewer for 0d data
 *
 * @author Lutz Foucar
 */

#ifndef _ZERODVIEWER_
#define _ZERODVIEWER_

#include "data_viewer.h"

class QLabel;

namespace jocassview
{
class ZeroDViewerData;

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

  /** retrieve the displayed data
   *
   * @return The histogram that contains the displayed data
   */
  QList<Data*> data();

  /** retrieve the type of the data viewer
   *
   * @return the type as name
   */
  QString type() const;

  /** overload printing
   *
   * just create an error message saying that a 0d value can't be plottet
   */
  virtual void print()const;

private:
  /** the label that displays the value */
  QLabel *_value;

  /** pointer to the 0d data wrapper */
  ZeroDViewerData *_data;

};
}//end namespace jocassview

#endif
