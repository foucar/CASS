// Copyright (C) 2013 Lutz Foucar

/**
 * @file data_viewer.h contains the base class for all data viewers
 *
 * @author Lutz Foucar
 */

#ifndef _DATAVIEWER_
#define _DATAVIEWER_

#include <QtGui/QWidget>

namespace cass
{
class HistogramBackend;
}//end namespace cass

namespace jocassview
{
/** base class for viewers
 *
 * @author Lutz Foucar
 */
class DataViewer : public QWidget
{
  Q_OBJECT

public:
  /** constructor
   *
   * sets the window title
   *
   * @param title the window title of this viewer
   * @param parent the parent widget of this viewer
   */
  DataViewer(QString title, QWidget *parent=0)
    : QWidget(parent)
  {
    setWindowTitle(title);
  }

  /** destructor
   *
   * virtual to make this a base class
   */
  virtual ~DataViewer();

  /** set the data to be displayed by this viewer
   *
   * The viewer that this data is passed to should take control over it and
   * delete it if necessary.
   *
   * @param data the data to be displayed
   */
  virtual void setData(cass::HistogramBackend *data) = 0;

  /** retrieve the data displayed by this viewer
   *
   * @param data the data to be displayed
   */
  virtual cass::HistogramBackend * data() = 0;
};
}//end namespace jocassview
#endif
