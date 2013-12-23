// Copyright (C) 2013 Lutz Foucar

/**
 * @file one_d_viewer.cpp contains viewer for 1d data
 *
 * @author Lutz Foucar
 */

#include <QtGui/QVBoxLayout>
#include <QtGui/QToolBar>

#include <qwt_plot.h>
#include <qwt_plot_curve.h>

#include "one_d_viewer.h"

#include "histogram.h"
#include "minmax_control.h"

using namespace jocassview;

OneDViewer::OneDViewer(QWidget *parent)
  : QWidget(parent)
{
  //create an vertical layout to put the plot and the toolbar in
  QVBoxLayout *layout(new QVBoxLayout);

  // Add the plot where the 1d data will be displayed in to layout
  _plot = new QwtPlot(this);
  _curves.push_front(new QwtPlotCurve);
  _curves[0]->attach(_plot);
  layout->addWidget(_plot);

  // create the toolbar
  QToolBar * toolbar(new QToolBar(this));

  // Add x-axis control to the toolbar
  _xControl = new MinMaxControl();
  connect(_xControl,SIGNAL(controls_changed()),this,SLOT(replot()));
  toolbar->addWidget(_xControl);

  // Add separator to toolbar
  toolbar->addSeparator();

  // Add y-axis control to the toolbar
  _yControl = new MinMaxControl();
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

  replot();
}

void OneDViewer::replot()
{
  _plot->replot();
}
