// Copyright (C) 2013 Lutz Foucar

/**
 * @file one_d_viewer.cpp contains viewer for 1d data
 *
 * @author Lutz Foucar
 */

#include <QtGui/QVBoxLayout>
#include <QtGui/QToolBar>

#include <qwt_plot.h>

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

  // create the toolbar
  QToolBar * toolbar(new QToolBar(this));

  // Add x-axis control to the toolbar
  _xControl = new MinMaxControl();
//  connect(_xControl,SIGNAL(controls_changed()),this,SLOT(replot()));
  toolbar->addWidget(_xControl);

  // Add separator to toolbar
  toolbar->addSeparator();

  // Add y-axis control to the toolbar
  _xControl = new MinMaxControl();
  toolbar->addWidget(_yControl);

  // Add toolbar to layout
  layout->addWidget(toolbar);

  // set the widgets layout
  setLayout(layout);
}

void OneDViewer::setData(cass::Histogram1DFloat *histogram)
{

}
