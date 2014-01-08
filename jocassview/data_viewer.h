// Copyright (C) 2013 Lutz Foucar

/**
 * @file data_viewer.h contains the base class for all data viewers
 *
 * @author Lutz Foucar
 */

#ifndef _DATAVIEWER_
#define _DATAVIEWER_


#include <QtGui/QMainWindow>
#include <QtGui/QCloseEvent>

namespace cass
{
class HistogramBackend;
}//end namespace cass

class QwtPlot;

namespace jocassview
{
/** base class for viewers
 *
 * @author Lutz Foucar
 */
class DataViewer : public QMainWindow
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
  DataViewer(QString title, QWidget *parent);

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

  /** retrieve the type of the data viewer
   *
   * @return the type as name
   */
  virtual QString type() const = 0;

  /** print the plot */
  virtual void print()const;

signals:
  /** signal emitted when viewer is about to be destroyed
   *
   * @param viewer the viewer that is closed (this)
   */
  void viewerClosed(DataViewer *viewer);

protected:
  /** react when a close event is send to this viewer
   *
   * @param event the close event
   */
  void closeEvent(QCloseEvent *event);

  /** receive move events to store the current position to the settings
   *
   * @param event the move event
   */
  void moveEvent(QMoveEvent *event);

  /** receive resize events to store the current size to the settings
   *
   * @param event the resize event
   */
  void resizeEvent(QResizeEvent *event);

protected:
  /** the plot inside which the data will be displayed */
  QwtPlot *_plot;
};
}//end namespace jocassview
#endif
