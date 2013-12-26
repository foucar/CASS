// Copyright (C) 2013 Lutz Foucar

/**
 * @file one_d_viewer.cpp contains viewer for 1d data
 *
 * @author Lutz Foucar
 */

#include <QtCore/QSettings>
#include <QtCore/QDebug>

#include <QtGui/QVBoxLayout>
#include <QtGui/QToolBar>
#include <QtGui/QAction>
#include <QtGui/QIcon>

#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_grid.h>
#include <qwt_legend.h>

#include "one_d_viewer.h"

#include "histogram.h"
#include "minmax_control.h"
#include "one_d_viewer_data.h"

using namespace jocassview;

OneDViewer::OneDViewer(QWidget *parent)
  : QWidget(parent)
{
  QSettings settings;

  //create an vertical layout to put the plot and the toolbar in
  QVBoxLayout *layout(new QVBoxLayout);

  // Add the plot where the 1d data will be displayed in to layout
  _plot = new QwtPlot(this);
  // add data and a curve that should be displayed
  _curvesData.push_front(new OneDViewerData);
  _curves.push_front(new QwtPlotCurve);
  _curves[0]->setTitle("current data");
  _curves[0]->attach(_plot);
  // add a grid to show on the plot
  _grid = new QwtPlotGrid;
  _grid->setMajPen(QPen(Qt::black, 0, Qt::DashLine));
  _grid->attach(_plot);
  // add a legend to the plot
  _legend = new QwtLegend;
  _legend->setItemMode(QwtLegend::CheckableItem);
  _plot->insertLegend(_legend,QwtPlot::RightLegend);
//  connect(_plot,SIGNAL(legendChecked(QwtPlotItem*,bool)),this,SLOT(on_legend_checked(QwtPlotItem)));
  // add the plot to the widget
  layout->addWidget(_plot);

  // create the toolbar
  QToolBar * toolbar(new QToolBar(this));

  // Add grid control to toolbar
  _gridControl = new QAction(QIcon(":images/grid.png"),
                             tr("toggle Grid"),toolbar);
  _gridControl->setCheckable(true);
  _gridControl->setChecked(settings.value("GridEnabled",true).toBool());
  connect(_gridControl,SIGNAL(triggered()),this,SLOT(replot()));
  toolbar->addAction(_gridControl);

  // Add legend control to toolbar
  _legendControl = new QAction(QIcon(":images/legend.png"),tr("toggle Legend"),toolbar);
  _legendControl->setCheckable(true);
  _legendControl->setChecked(settings.value("LegendShown",true).toBool());
  connect(_legendControl,SIGNAL(triggered()),this,SLOT(replot()));
  toolbar->addAction(_legendControl);

  // Add separator to toolbar
  toolbar->addSeparator();

  // Add x-axis control to the toolbar
  _xControl = new MinMaxControl(tr("x-scale"));
  connect(_xControl,SIGNAL(controls_changed()),this,SLOT(replot()));
  toolbar->addWidget(_xControl);

  // Add separator to toolbar
  toolbar->addSeparator();

  // Add y-axis control to the toolbar
  _yControl = new MinMaxControl(tr("y-scale"));
  connect(_xControl,SIGNAL(controls_changed()),this,SLOT(replot()));
  toolbar->addWidget(_yControl);

  // Add toolbar to layout
  layout->addWidget(toolbar);

  // set the widgets layout
  setLayout(layout);

  replot();
}

void OneDViewer::setData(cass::Histogram1DFloat *histogram)
{
  QwtArray<double> xx(histogram->size());
  QwtArray<double> yy(histogram->size());
  const cass::AxisProperty &xaxis(histogram->axis()[cass::Histogram1DFloat::xAxis]);
  for (size_t i=0; i<xaxis.nbrBins();++i)
  {
    xx[i] = xaxis.position(i);
    const float yVal(histogram->memory()[i]);
    yy[i] = (std::isnan(yVal) || std::isinf(yVal)) ? 0 : yVal;
  }
  _curves[0]->setData(xx,yy);

//  qDebug()<<_curves[0]->boundingRect();
//
//  _curvesData[0]->setData(histogram->memory(),
//                          QwtDoubleInterval(xaxis.lowerLimit(),xaxis.upperLimit()));
//  _curves[0]->setData(*_curvesData[0]);

  replot();
}

void OneDViewer::replot()
{
  _grid->enableX(_gridControl->isChecked());
  _grid->enableY(_gridControl->isChecked());

  if(_legendControl->isChecked())
    _legend->show();
  else
    _legend->hide();

  _plot->replot();
}
