// Copyright (C) 2013 Lutz Foucar

/**
 * @file two_d_viewer.cpp contains viewer for 2d data
 *
 * @author Lutz Foucar
 */

#include <QtCore/QDebug>

#include <QtGui/QVBoxLayout>
#include <QtGui/QToolBar>
#include <QtGui/QSpinBox>
#include <QtGui/QLabel>

#include <qwt_plot.h>
#include <qwt_scale_widget.h>
#include <qwt_plot_layout.h>
#include <qwt_plot_spectrogram.h>

#include "two_d_viewer.h"

#include "two_d_viewer_data.h"
#include "histogram.h"
#include "minmax_control.h"
#include "track_zoomer_2d.h"

using namespace jocassview;

TwoDViewer::TwoDViewer(QString title, QWidget *parent)
  : DataViewer(title,parent)
{
  // generate the list of color maps
  _maps[-1] = QwtLinearColorMap(QColor(0,0,0), QColor(255,255,255));
  _maps[-1].addColorStop(0.10, QColor(0,0,0));
  _maps[-1].addColorStop(0.80, QColor(255,255,255));

  _maps[ 0] = QwtLinearColorMap(Qt::darkCyan, Qt::red);
  _maps[ 0].addColorStop(0.10, QColor(Qt::darkCyan));
  _maps[ 0].addColorStop(0.60, QColor(Qt::green));
  _maps[ 0].addColorStop(0.90, QColor(Qt::yellow));

  _maps[ 1] = QwtLinearColorMap(QColor(0,0,0), QColor(255,0,0));
  _maps[ 1].addColorStop(0.10, QColor(50,0,0));
  _maps[ 1].addColorStop(0.35, QColor(115,0,0));
  _maps[ 1].addColorStop(0.80, QColor(180,0,0));

  _maps[ 2] = QwtLinearColorMap(QColor(0,0,0), QColor(0,255,0));
  _maps[ 2].addColorStop(0.10, QColor(0,50,0));
  _maps[ 2].addColorStop(0.35, QColor(0,115,0));
  _maps[ 2].addColorStop(0.80, QColor(0,180,0));

  _maps[ 3] = QwtLinearColorMap(QColor(0,0,0), QColor(0,0,255));
  _maps[ 3].addColorStop(0.10, QColor(0,0,50));
  _maps[ 3].addColorStop(0.35, QColor(0,0,115));
  _maps[ 3].addColorStop(0.80, QColor(0,0,180));

  _maps[ 4] = QwtLinearColorMap(QColor(0,0,0), QColor(255,0,255));
  _maps[ 4].addColorStop(0.10, QColor(50,0,50));
  _maps[ 4].addColorStop(0.35, QColor(115,0,115));
  _maps[ 4].addColorStop(0.80, QColor(180,0,180));

  _maps[ 5] = QwtLinearColorMap(QColor(0,0,0), QColor(0,255,255));
  _maps[ 5].addColorStop(0.10, QColor(0,50,50));
  _maps[ 5].addColorStop(0.35, QColor(0,115,115));
  _maps[ 5].addColorStop(0.80, QColor(0,180,180));

  _maps[ 6] = QwtLinearColorMap(Qt::black, Qt::red);
  _maps[ 6].addColorStop(0.10, Qt::blue);
  _maps[ 6].addColorStop(0.30, Qt::darkCyan);
  _maps[ 6].addColorStop(0.40, Qt::cyan);
  _maps[ 6].addColorStop(0.60, Qt::darkGreen);
  _maps[ 6].addColorStop(0.70, Qt::green);
  _maps[ 6].addColorStop(0.95, Qt::yellow);

  _maps[ 7] = QwtLinearColorMap(Qt::darkBlue, Qt::white);
  _maps[ 7].addColorStop(0.15, Qt::blue);
  _maps[ 7].addColorStop(0.30, QColor(255,90,255));
  _maps[ 7].addColorStop(0.40, Qt::yellow);
  _maps[ 7].addColorStop(0.60, Qt::darkYellow);
  _maps[ 7].addColorStop(0.70, Qt::red);
  _maps[ 7].addColorStop(0.80, Qt::darkRed);
  _maps[ 7].addColorStop(0.95, QColor(149,24,0));

  _maps[ 8] = QwtLinearColorMap(QColor(65,105,241), QColor(255,51,204));
  _maps[ 8].addColorStop(0.10, QColor(0,127,255));
  _maps[ 8].addColorStop(0.60, QColor(221,0,225));
  _maps[ 8].addColorStop(0.95, QColor(255,51,204));

  _maps[ 9] = QwtLinearColorMap(QColor(72,6,7), Qt::white);
  _maps[ 9].addColorStop(0.10, QColor(72,6,7));
  _maps[ 9].addColorStop(0.20, Qt::darkRed);
  _maps[ 9].addColorStop(0.35, Qt::red);
  _maps[ 9].addColorStop(0.65, QColor(255,195,59));
  _maps[ 9].addColorStop(0.85, Qt::yellow);
  _maps[ 9].addColorStop(0.98, Qt::white);

  _maps[10] = QwtLinearColorMap(QColor(16,16,255), QColor(0,255,129));
  _maps[10].addColorStop(0.10, QColor(16,16,255));
  _maps[10].addColorStop(0.50, Qt::cyan);
  _maps[10].addColorStop(0.90, QColor(0,255,155));

  _maps[11] = QwtLinearColorMap(QColor(10,10,10), QColor(184,115,51));
  _maps[11].addColorStop(0.10, QColor(10,10,10));
  _maps[11].addColorStop(0.20, QColor(149,34,0));
  _maps[11].addColorStop(0.90, QColor(184,115,51));


  //create an vertical layout to put the plot and the toolbar in
  QVBoxLayout *layout(new QVBoxLayout);

  // create the plot where the 2d data will be displayed in
  _plot = new QwtPlot(this);
  QwtScaleWidget *rightAxis(_plot->axisWidget(QwtPlot::yRight));
  rightAxis->setTitle("Intensity");
  rightAxis->setColorBarEnabled(true);
  rightAxis->setColorMap(QwtDoubleInterval(0,1),_maps[-1]);
  _plot->setAxisScale(QwtPlot::yRight,0,1);
  _plot->axisWidget(QwtPlot::xBottom)->setTitle("x-axis");
  _plot->axisWidget(QwtPlot::yLeft)->setTitle("y-axis");
  _plot->enableAxis(QwtPlot::yRight);
  _plot->plotLayout()->setAlignCanvasToScales(true);
  _plot->setAutoReplot(false);
  // create spectrogram data
  _data = new TwoDViewerData;
  // create the spectrom that is displayed in the plot
  _spectrogram = new QwtPlotSpectrogram();
  _spectrogram->setData(*_data);
  _spectrogram->attach(_plot);
  _spectrogram->setColorMap(_maps[-1]);
  // create a zoomer for the 2d data
  _zoomer = new TrackZoomer2D(_plot->canvas());
  _zoomer->setSelectionFlags( QwtPicker::RectSelection | QwtPicker::DragSelection );
  _zoomer->setMousePattern(QwtEventPattern::MouseSelect2,
                           Qt::RightButton, Qt::ControlModifier);
  _zoomer->setMousePattern(QwtEventPattern::MouseSelect3,
                           Qt::RightButton);
  layout->addWidget(_plot);

  // create the toolbar
  QToolBar * toolbar(new QToolBar(this));
  // add the min/max control to the toolbar
  _zControl = new MinMaxControl(tr("z-scale"));
  connect(_zControl,SIGNAL(controls_changed()),this,SLOT(replot()));
  toolbar->addWidget(_zControl);
  // Add separator
  toolbar->addSeparator();
  // Add the colorbar control
//  QLabel * cLabel(new QLabel(tr("Colorbar ID")));
//  toolbar->addWidget(cLabel);
  _colorId = new QSpinBox();
  _colorId->setRange(-1,11);
  _colorId->setValue(-1);
  _colorId->setWrapping(true);
  _colorId->setToolTip(tr("Select the used Colorbar"));
  connect(_colorId,SIGNAL(valueChanged(int)),this,SLOT(replot()));
  toolbar->addWidget(_colorId);

  layout->addWidget(toolbar);
  setLayout(layout);
}

void TwoDViewer::setData(cass::HistogramBackend *hist)
{
  cass::Histogram2DFloat *histogram(dynamic_cast<cass::Histogram2DFloat*>(hist));
  const cass::AxisProperty &xaxis(histogram->axis()[cass::Histogram2DFloat::xAxis]);
  const cass::AxisProperty &yaxis(histogram->axis()[cass::Histogram2DFloat::yAxis]);
  _data->setData(histogram->memory(),std::make_pair(xaxis.nbrBins(),yaxis.nbrBins()),
                 QwtDoubleInterval(xaxis.lowerLimit(),xaxis.upperLimit()),
                 QwtDoubleInterval(yaxis.lowerLimit(),yaxis.upperLimit()));
  _spectrogram->setData(*_data);
  _spectrogram->invalidateCache();
  _spectrogram->itemChanged();

  _plot->axisWidget(QwtPlot::xBottom)->setTitle(QString::fromStdString(xaxis.title()));
  _plot->axisWidget(QwtPlot::yLeft)->setTitle(QString::fromStdString(yaxis.title()));

  /** @note the below will be done when zooming into the initial bounding rect
   *  _plot->setAxisScale(QwtPlot::yLeft,_data->boundingRect().top(),_data->boundingRect().bottom());
   *  _plot->setAxisScale(QwtPlot::xBottom,_data->boundingRect().left(),_data->boundingRect().right());
   */

  _zoomer->setData(_data);

  // check if the data is new (the bounding box changed) in which case we
  // reinitialize the zoomer
  if (_zoomer->zoomBase() != _spectrogram->boundingRect())
  {
    _zoomer->setZoomBase(_spectrogram->boundingRect());
    _zoomer->zoom(_spectrogram->boundingRect());
    _zoomer->setZoomBase(true);
  }
  replot();
}

cass::HistogramBackend* TwoDViewer::data()
{
  return 0;
}

void TwoDViewer::replot()
{
  const double min(!_zControl->autoscale() ? _zControl->min() : _data->zRange().minValue());
  const double max(!_zControl->autoscale() ? _zControl->max() : _data->zRange().maxValue());
  int colorid = _colorId->value();

  _data->setZRange(QwtDoubleInterval(min,max));
  _spectrogram->setData(*_data);
  _spectrogram->setColorMap(_maps[colorid]);
  _plot->axisWidget(QwtPlot::yRight)->setColorMap(_spectrogram->data().range(),_maps[colorid]);
  _plot->setAxisScale(QwtPlot::yRight,min,max);
  _plot->replot();
}
