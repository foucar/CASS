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
#include <QtGui/QMenu>
#include <QtGui/QColorDialog>
#include <QtGui/QInputDialog>

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
  QPen pen;
  pen.setColor(settings.value("CurveColor",Qt::blue).value<QColor>());
  pen.setWidth(settings.value("CurveWidth",1).toInt());
  _curves[0]->setPen(pen);
  _curves[0]->attach(_plot);
  // add a grid to show on the plot
  _grid = new QwtPlotGrid;
  _grid->setMajPen(QPen(Qt::black, 0, Qt::DashLine));
  _grid->attach(_plot);
  // add a legend to the plot
  _legend = new QwtLegend;
//  _legend->setItemMode(QwtLegend::CheckableItem);
  _plot->insertLegend(_legend,QwtPlot::RightLegend);
  QWidget *curveLegendWidget(_legend->find(_curves[0]));
  curveLegendWidget->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(curveLegendWidget,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(on_legend_right_clicked(QPoint)));
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

void OneDViewer::addData(cass::Histogram1DFloat *histogram)
{
  _curves.push_back(new QwtPlotCurve);
  QwtPlotCurve * curve(_curves.back());

  QwtArray<double> xx(histogram->size());
  QwtArray<double> yy(histogram->size());
  const cass::AxisProperty &xaxis(histogram->axis()[cass::Histogram1DFloat::xAxis]);
  for (size_t i=0; i<xaxis.nbrBins();++i)
  {
    xx[i] = xaxis.position(i);
    const float yVal(histogram->memory()[i]);
    yy[i] = (std::isnan(yVal) || std::isinf(yVal)) ? 0 : yVal;
  }
  curve->setData(xx,yy);

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

  QList<QWidget*> list(_legend->legendItems());
  for (int i=0; i<list.size();++i)
  {
    if(_legendControl->isChecked())
      list.at(i)->show();
    else
      list.at(i)->hide();
  }

  _plot->updateLayout();
  _plot->replot();
}

void OneDViewer::on_legend_right_clicked(QPoint pos)
{
  qDebug()<<"ContextMenuRequested";

  if (!sender()->isWidgetType())
    return;

  QWidget *curveWidget(dynamic_cast<QWidget*>(sender()));
  QwtPlotCurve *curve(dynamic_cast<QwtPlotCurve*>(_legend->find(curveWidget)));
  QPoint globalPos(curveWidget->mapToGlobal(pos));

  QMenu myMenu;
  myMenu.addAction(tr("Color"));
  myMenu.addAction(tr("Width"));
  myMenu.addSeparator();
  if(curve->isVisible())
    myMenu.addAction(tr("Hide"));
  else
    myMenu.addAction(tr("Show"));
  if(curve->title() != QwtText("current data"))
    myMenu.addAction(tr("Delete"));

  QPen pen(curve->pen());
  QAction* selectedItem = myMenu.exec(globalPos);
  if (selectedItem && !selectedItem->isSeparator())
  {
    if (selectedItem->text() == tr("Hide"))
      curve->hide();
    else if (selectedItem->text() == tr("Show"))
      curve->show();
    else if (selectedItem->text() == tr("Delete"))
    {
      _curves.removeAll(curve);
      curve->detach();
      _legend->update();
    }
    else if(selectedItem->text() == tr("Color"))
    {
      QColor col(QColorDialog::getColor(pen.color(),this,tr("Select Color")));
      if (col.isValid())
        pen.setColor(col);
    }
    else if (selectedItem->text() == tr("Width"))
    {
      bool ok(false);
      int width(QInputDialog::getInt(this,tr("Set Line Width"),tr("Line width"),pen.width(),0,20,1,&ok));
      if (ok)
        pen.setWidth(width);
    }

    curve->setPen(pen);
    replot();
  }
}
