// Copyright (C) 2013 Lutz Foucar

/**
 * @file two_d_viewer.cpp contains viewer for 2d data
 *
 * @author Lutz Foucar
 */

#include <QtCore/QDebug>
#include <QtCore/QSettings>

#include <QtGui/QVBoxLayout>
#include <QtGui/QToolBar>
#include <QtGui/QSpinBox>
#include <QtGui/QLabel>

#include <qwt_plot.h>
#include <qwt_scale_widget.h>
#include <qwt_plot_layout.h>
#include <qwt_plot_spectrogram.h>
#include <qwt_color_map.h>

#include "two_d_viewer.h"

#include "two_d_viewer_data.h"
#include "histogram.h"
#include "minmax_control.h"
#include "track_zoomer_2d.h"
#include "logcolor_map.h"
#include "data.h"

using namespace jocassview;

TwoDViewer::TwoDViewer(QString title, QWidget *parent)
  : DataViewer(title,parent)
{
  // settings to read from the ini file
  QSettings settings;
  settings.beginGroup(windowTitle());

  // create the plot where the 2d data will be displayed in as central widget
  _plot = new QwtPlot(this);
  QwtScaleWidget *rightAxis(_plot->axisWidget(QwtPlot::yRight));
  rightAxis->setColorBarEnabled(true);
  _plot->enableAxis(QwtPlot::yRight);
  _plot->plotLayout()->setAlignCanvasToScales(true);
  _plot->setAutoReplot(false);
  // create spectrogram data
  TwoDViewerData *data(new TwoDViewerData);
  // create the spectrom that is displayed in the plot
  int colorid(settings.value("ColorTableID",-1).toInt());
  _spectrogram = new QwtPlotSpectrogram();
  _spectrogram->setData(data);
  _spectrogram->attach(_plot);
  _spectrogram->setColorMap(cmap(colorid));
  // create a zoomer for the 2d data
  _zoomer = new TrackZoomer2D(_plot->canvas());
//  _zoomer->setSelectionFlags( QwtPicker::RectSelection | QwtPicker::DragSelection );
  _zoomer->setMousePattern(QwtEventPattern::MouseSelect2,
                           Qt::RightButton, Qt::ControlModifier);
  _zoomer->setMousePattern(QwtEventPattern::MouseSelect3,
                           Qt::RightButton);
  _zoomer->setData(data);
  setCentralWidget(_plot);

  // create the toolbar
  QToolBar * toolbar(new QToolBar("Plot Control",this));
  addToolBar(Qt::BottomToolBarArea,toolbar);

  // Add title display to the toolbar
  _axisTitleControl = new QAction(QIcon(":images/axistitle.png"),
                                  tr("Toggle Axis Titles"),toolbar);
  _axisTitleControl->setCheckable(true);
  _axisTitleControl->setChecked(settings.value("DisplayTitles",true).toBool());
  connect(_axisTitleControl,SIGNAL(triggered()),this,SLOT(replot()));
  toolbar->addAction(_axisTitleControl);

  // add the min/max control to the toolbar
  _zControl = new MinMaxControl(QString(windowTitle() + "/z-scale"),toolbar);
  connect(_zControl,SIGNAL(controls_changed()),this,SLOT(replot()));
  toolbar->addWidget(_zControl);

  // Add separator
  toolbar->addSeparator();

  // Add the colorbar control
  _colorId = new QSpinBox();
  _colorId->setRange(-4,11);
  _colorId->setValue(colorid);
  _colorId->setWrapping(true);
  _colorId->setToolTip(tr("Select the used Colorbar"));
  connect(_colorId,SIGNAL(valueChanged(int)),this,SLOT(replot()));
  toolbar->addWidget(_colorId);

  // Set the size and position of the window
  resize(settings.value("WindowSize",size()).toSize());
  move(settings.value("WindowPosition",pos()).toPoint());
}

TwoDViewer::~TwoDViewer()
{

}

//void TwoDViewer::setData(cass::HistogramBackend *hist)
//{
//  if (!hist)
//    return;
//
//  cass::Histogram2DFloat *histogram(dynamic_cast<cass::Histogram2DFloat*>(hist));
//  TwoDViewerData *data(dynamic_cast<TwoDViewerData*>(_spectrogram->data()));
//  data->setData(histogram);
//
//}

QList<Data*> TwoDViewer::data()
{
//  return dynamic_cast<TwoDViewerData*>(_spectrogram->data())->data();
  QList<Data*> list;
  list.append(dynamic_cast<TwoDViewerData*>(_spectrogram->data()));
  return list;
}

QString TwoDViewer::type() const
{
  return QString("2DViewer");
}

void TwoDViewer::saveData(const QString &filename)
{

}

void TwoDViewer::dataChanged()
{
  /** check if the data is different (the bounding box changed) in which case we
   *  reinitialize the zoomer
   *  @note the below will be done when zooming into the initial bounding rect
@code
  _plot->setAxisScale(QwtPlot::yLeft,_data->boundingRect().top(),_data->boundingRect().bottom());
  _plot->setAxisScale(QwtPlot::xBottom,_data->boundingRect().left(),_data->boundingRect().right());
@endcode
   */
  if (_zoomer->zoomBase() != _spectrogram->boundingRect())
  {
    _zoomer->setZoomBase(_spectrogram->boundingRect());
    _zoomer->zoom(_spectrogram->boundingRect());
    _zoomer->setZoomBase(true);
  }
  replot();
}

void TwoDViewer::replot()
{
  /** get the data from the spectrogram and get the min and max z-values to be displayed */
  TwoDViewerData *data(dynamic_cast<TwoDViewerData*>(_spectrogram->data()));
  const double min(!_zControl->autoscale() ? _zControl->min() : data->origZInterval().minValue());
  const double max(!_zControl->autoscale() ? _zControl->max() : data->origZInterval().maxValue());

  /** get the colormap to be used */
  int colorid = _colorId->value();

  /** set the colormap and min / max z-value */
  _spectrogram->data()->setInterval(Qt::ZAxis,QwtInterval(min,max));
  _spectrogram->setColorMap(cmap(colorid));
  _plot->axisWidget(QwtPlot::yRight)->setColorMap(_spectrogram->data()->interval(Qt::ZAxis),cmap(colorid));
  _plot->setAxisScale(QwtPlot::yRight,min,max);

  /** display the axis titles if requested */
  if (_axisTitleControl->isChecked())
  {
    cass::HistogramBackend *hist(this->data().front()->result());
    if (hist)
    {
      QString xtitle(QString::fromStdString(hist->axis()[cass::HistogramBackend::xAxis].title()));
      _plot->axisWidget(QwtPlot::xBottom)->setTitle(xtitle);
      QString ytitle(QString::fromStdString(hist->axis()[cass::HistogramBackend::yAxis].title()));
      _plot->axisWidget(QwtPlot::yLeft)->setTitle(ytitle);
    }
  }
  else
  {
    _plot->axisWidget(QwtPlot::yLeft)->setTitle("");
    _plot->axisWidget(QwtPlot::xBottom)->setTitle("");
  }

  /** replot the plot */
  _plot->replot();

  /** save the current settings */
  QSettings settings;
  settings.beginGroup(windowTitle());
  settings.setValue("ColorTableID",colorid);
  settings.setValue("DisplayTitles",_axisTitleControl->isChecked());
}

QwtLinearColorMap* TwoDViewer::cmap(const int colorid) const
{
  if (colorid == -4)
  {
    QwtLinearColorMap *map(new QwtLinearColorMap(QColor(Qt::black), QColor(Qt::red)));
    map->addColorStop(0.999, QColor(Qt::white));
    map->addColorStop(0.001, QColor(Qt::white));
    return map;
  }
  if (colorid == -3)
  {
    QwtLinearColorMap *map(new QwtLinearColorMap(QColor(Qt::black), QColor(Qt::black)));
    map->addColorStop(0.999, QColor(Qt::white));
    return map;
  }
  if (colorid == -2)
  {
    QwtLinearColorMap *map(new QwtLinearColorMap(QColor(Qt::white),QColor(Qt::black)));
    return map;
  }
  if (colorid == -1)
  {
    QwtLinearColorMap *map(new QwtLinearColorMap(QColor(Qt::black), QColor(Qt::white)));
    return map;
  }
  else if(colorid == 0)
  {
    QwtLinearColorMap *map(new QwtLinearColorMap(Qt::darkCyan, Qt::red));
    map->addColorStop(0.10, QColor(Qt::darkCyan));
    map->addColorStop(0.60, QColor(Qt::green));
    map->addColorStop(0.90, QColor(Qt::yellow));
    return map;
  }
  else if(colorid == 1)
  {
    QwtLinearColorMap *map( new QwtLinearColorMap(QColor(Qt::black), QColor(255,0,0)));
    map->addColorStop(0.10, QColor(50,0,0));
    map->addColorStop(0.35, QColor(115,0,0));
    map->addColorStop(0.80, QColor(180,0,0));
    return map;
  }
  else if(colorid == 2)
  {
    QwtLinearColorMap *map( new QwtLinearColorMap(QColor(Qt::black), QColor(0,255,0)));
    return map;
  }
  else if(colorid == 3)
  {
    QwtLinearColorMap *map( new QwtLinearColorMap(QColor(Qt::black), QColor(0,0,255)));
    return map;
  }
  else if(colorid == 4)
  {
    QwtLinearColorMap *map( new QwtLinearColorMap(QColor(Qt::black), QColor(255,0,255)));
    return map;
  }
  else if(colorid == 5)
  {
    QwtLinearColorMap *map( new QwtLinearColorMap(QColor(Qt::black), QColor(0,255,255)));
    return map;
  }
  else if(colorid == 6)
  {
    QwtLinearColorMap *map( new QwtLinearColorMap(QColor(Qt::black), QColor(255,255,0)));
    return map;
  }
  else if(colorid == 7)
  {
//    QwtLinearColorMap *map( new LogColorMap(Qt::black, Qt::red));
    QwtLinearColorMap *map( new QwtLinearColorMap(Qt::black, Qt::red));
    map->addColorStop(0.10, Qt::blue);
    map->addColorStop(0.30, Qt::darkCyan);
    map->addColorStop(0.40, Qt::cyan);
    map->addColorStop(0.60, Qt::darkGreen);
    map->addColorStop(0.70, Qt::green);
    map->addColorStop(0.95, Qt::yellow);
    return map;
  }
  else if(colorid == 8)
  {

    QwtLinearColorMap *map( new QwtLinearColorMap(Qt::darkBlue, Qt::white));
    map->addColorStop(0.15, Qt::blue);
    map->addColorStop(0.30, QColor(255,90,255));
    map->addColorStop(0.40, Qt::yellow);
    map->addColorStop(0.60, Qt::darkYellow);
    map->addColorStop(0.70, Qt::red);
    map->addColorStop(0.80, Qt::darkRed);
    map->addColorStop(0.95, QColor(149,24,0));
    return map;
  }
  else if(colorid == 9)
  {

    QwtLinearColorMap *map( new QwtLinearColorMap(QColor(65,105,241), QColor(255,51,204)));
    map->addColorStop(0.10, QColor(0,127,255));
    map->addColorStop(0.60, QColor(221,0,225));
    map->addColorStop(0.95, QColor(255,51,204));
    return map;
  }
  else if(colorid ==10)
  {

    QwtLinearColorMap *map( new QwtLinearColorMap(QColor(72,6,7), Qt::white));
    map->addColorStop(0.10, QColor(72,6,7));
    map->addColorStop(0.20, Qt::darkRed);
    map->addColorStop(0.35, Qt::red);
    map->addColorStop(0.65, QColor(255,195,59));
    map->addColorStop(0.85, Qt::yellow);
    map->addColorStop(0.98, Qt::white);
    return map;
  }
  else if(colorid ==11)
  {

    QwtLinearColorMap *map( new QwtLinearColorMap(QColor(16,16,255), QColor(0,255,129)));
    map->addColorStop(0.10, QColor(16,16,255));
    map->addColorStop(0.50, Qt::cyan);
    map->addColorStop(0.90, QColor(0,255,155));
    return map;
  }
  else if(colorid ==12)
  {

    QwtLinearColorMap *map( new QwtLinearColorMap(QColor(10,10,10), QColor(184,115,51)));
    map->addColorStop(0.10, QColor(10,10,10));
    map->addColorStop(0.20, QColor(149,34,0));
    map->addColorStop(0.90, QColor(184,115,51));
    return map;
  }
  else
  {
    QwtLinearColorMap *map(new QwtLinearColorMap(QColor(Qt::black), QColor(Qt::white)));
    return map;
  }
}

QStringList TwoDViewer::cmaps()const
{
  QStringList list;
  return list;
}
