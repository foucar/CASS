// Copyright (C) 2013 Lutz Foucar

/**
 * @file data_viewer.h contains the base class for all data viewers
 *
 * @author Lutz Foucar
 */

#ifndef _DATAVIEWER_
#define _DATAVIEWER_

#include <QtGlobal>

#if QT_VERSION >= 0x050000
#include <QtWidgets/QMainWindow>
#else
#include <QtGui/QMainWindow>
#endif

class QCloseEvent;

namespace cass
{
class HistogramBackend;
}//end namespace cass

class QwtPlot;

namespace jocassview
{
class Data;

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

  /** retrieve the data displayed by this viewer
   *
   * @return the list of data displayed by this viewer
   */
  virtual QList<Data*> data() = 0;

  /** retrieve the type of the data viewer
   *
   * @return the type as name
   */
  virtual QString type() const = 0;

  /** print the plot */
  virtual void print()const;

  /** save the data to file
   *
   * @param filename the name of the file to save the data to
   */
  virtual void saveData(const QString & filename)=0;

  /** use this to tell that the data has changed */
  virtual void dataChanged();

  /** the list of file types that the data can be stored as by this viewer
   *
   * @return list or suffixes that the data by this viewer can be saveData
   */
  virtual QStringList dataFileSuffixes()const = 0;

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
