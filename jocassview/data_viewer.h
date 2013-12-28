// Copyright (C) 2013 Lutz Foucar

/**
 * @file data_viewer.h contains the base class for all data viewers
 *
 * @author Lutz Foucar
 */

#ifndef _DATAVIEWER_
#define _DATAVIEWER_

#include <QtCore/QDebug>
#include <QtCore/QSettings>

#include <QtGui/QWidget>
#include <QtGui/QCloseEvent>

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
    : QWidget(parent,Qt::Window)
  {
    setWindowTitle(title);
    setAttribute(Qt::WA_DeleteOnClose);
  }

  /** destructor
   *
   * virtual to make this a base class
   */
  virtual ~DataViewer() {}

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
  void closeEvent(QCloseEvent *event)
  {
    emit viewerClosed(this);
    event->accept();
  }

  /** receive move events to store the current position to the settings
   *
   * @param event the move event
   */
  void moveEvent(QMoveEvent *event)
  {
    QSettings settings;
    settings.beginGroup(windowTitle());
    settings.setValue("WindowPosition",event->pos());
  }

  /** receive resize events to store the current size to the settings
   *
   * @param event the resize event
   */
  void resizeEvent(QResizeEvent *event)
  {
    QSettings settings;
    settings.beginGroup(windowTitle());
    settings.setValue("WindowSize",event->size());
  }


};
}//end namespace jocassview
#endif
