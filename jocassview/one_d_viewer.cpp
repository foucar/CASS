// Copyright (C) 2013 Lutz Foucar

/**
 * @file one_d_viewer.cpp contains viewer for 1d data
 *
 * @author Lutz Foucar
 */

#include <QtCore/QSettings>
#include <QtCore/QVector>
#include <QtCore/QDebug>

#include <QtGui/QVBoxLayout>
#include <QtGui/QToolBar>
#include <QtGui/QAction>
#include <QtGui/QIcon>
#include <QtGui/QMenu>
#include <QtGui/QColorDialog>
#include <QtGui/QInputDialog>
#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>

#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_grid.h>
#include <qwt_legend.h>
#include <qwt_scale_engine.h>

#include "one_d_viewer.h"

#include "histogram.h"
#include "minmax_control.h"
#include "one_d_viewer_data.h"
#include "file_handler.h"
#include "curve_plot.h"

using namespace jocassview;

OneDViewer::OneDViewer(QString title, QWidget *parent)
  : DataViewer(title,parent)
{
  // add settings object to retrieve the settings for this view
  QSettings settings;
  settings.beginGroup(windowTitle());

  //create an vertical layout to put the plot and the toolbar in
  QVBoxLayout *layout(new QVBoxLayout);

  // Add the plot where the 1d data will be displayed in to layout
  _plot = new QwtPlot(this);
  // add a curve that should be displayed
  _curves.push_front(new PlotCurve);
  _curves[0]->setTitle("current data");
  OneDViewerData *data(new OneDViewerData);
  _curves[0]->setData(data);
  QPen pen;
  pen.setColor(settings.value("CurveColor",Qt::blue).value<QColor>());
  pen.setWidth(settings.value("CurveWidth",1).toInt());
  _curves[0]->setStyle(QwtPlotCurve::Steps);
  _curves[0]->setPen(pen);
  _curves[0]->attach(_plot);
  // add a grid to show on the plot
  _grid = new QwtPlotGrid;
  _grid->setMajPen(QPen(Qt::black, 0, Qt::DashLine));
  _grid->attach(_plot);
  // add a legend to the plot
  _legend = new QwtLegend;
  _legend->setItemMode(QwtLegend::ClickableItem);
  _plot->insertLegend(_legend,QwtPlot::RightLegend);
  QWidget *curveLegendWidget(_legend->find(_curves[0]));
  curveLegendWidget->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(curveLegendWidget,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(on_legend_right_clicked(QPoint)));
  connect(_plot,SIGNAL(legendClicked(QwtPlotItem*)),this,SLOT(on_legend_clicked(QwtPlotItem*)));
  // add the plot to the widget
  layout->addWidget(_plot);

  // create the toolbar
  QToolBar * toolbar(new QToolBar(this));

  // Add a button that allows to add a reference curve
  toolbar->addAction(QIcon(":images/graph_add.png"),
                     tr("Add a reference Graph to the Plot"),
                     this,SLOT(on_add_graph_triggered()));

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
  _xControl = new MinMaxControl(QString(windowTitle() + "/x-scale"),toolbar);
  connect(_xControl,SIGNAL(controls_changed()),this,SLOT(replot()));
  toolbar->addWidget(_xControl);

  // Add separator to toolbar
  toolbar->addSeparator();

  // Add y-axis control to the toolbar
  _yControl = new MinMaxControl(QString(windowTitle() + "/y-scale"),toolbar);
  connect(_yControl,SIGNAL(controls_changed()),this,SLOT(replot()));
  toolbar->addWidget(_yControl);

  // Add toolbar to layout
  layout->addWidget(toolbar);

  // set the widgets layout
  setLayout(layout);

  // Set the size and position of the window
  resize(settings.value("WindowSize",size()).toSize());
  move(settings.value("WindowPosition",pos()).toPoint());
}

OneDViewer::~OneDViewer()
{

}

void OneDViewer::setData(cass::HistogramBackend *hist)
{
  if (!hist)
    return;

  cass::Histogram1DFloat *histogram(dynamic_cast<cass::Histogram1DFloat*>(hist));
  const cass::AxisProperty &xaxis(histogram->axis()[cass::Histogram1DFloat::xAxis]);

  OneDViewerData *data (dynamic_cast<OneDViewerData*>(_curves[0]->data()));
  data->setData(histogram->memory(), QwtInterval(xaxis.lowerLimit(),xaxis.upperLimit()));
  _curves[0]->itemChanged();
  replot();
}

cass::HistogramBackend* OneDViewer::data()
{
  return 0;
}

QString OneDViewer::type() const
{
  return QString("1DViewer");
}

void OneDViewer::addData(cass::Histogram1DFloat *histogram)
{
  if (!histogram)
    return;

  _curves.push_back(new PlotCurve);
  PlotCurve * curve(_curves.back());

  QPen pen;
  pen.setWidth(1);
  pen.setColor(QColor::fromHsv(qrand() % 256, 255, 190));
  curve->setPen(pen);
  curve->setTitle(QString::fromStdString(histogram->key()));
  curve->setStyle(QwtPlotCurve::Steps);
  curve->attach(_plot);
  QWidget *curveWidget(_legend->find(curve));
  curveWidget->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(curveWidget,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(on_legend_right_clicked(QPoint)));

  OneDViewerData *data(new OneDViewerData);
  const cass::AxisProperty &xaxis(histogram->axis()[cass::Histogram1DFloat::xAxis]);
  data->setData(histogram->memory(), QwtInterval(xaxis.lowerLimit(),xaxis.upperLimit()));
  curve->setData(data);

  replot();
}

void OneDViewer::replot()
{
  /** check if grid should be enabled */
  _grid->enableX(_gridControl->isChecked());
  _grid->enableY(_gridControl->isChecked());

  /** check if legend should be drawn
   *  (hide all legend items and update the layout if not)
   */
  QList<QWidget*> list(_legend->legendItems());
  for (int i=0; i<list.size();++i)
  {
    if(_legendControl->isChecked())
      list.at(i)->show();
    else
      list.at(i)->hide();
  }

  QwtLog10ScaleEngine *log(new QwtLog10ScaleEngine);
  log->setAttribute(QwtScaleEngine::Symmetric);

  /** check if the scales should be log or linear */
  if(_xControl->log())
    _plot->setAxisScaleEngine(QwtPlot::xBottom, new QwtLog10ScaleEngine);
  else
    _plot->setAxisScaleEngine(QwtPlot::xBottom, new QwtLinearScaleEngine);

  if(_yControl->log())
//    _plot->setAxisScaleEngine(QwtPlot::yLeft, new QwtLog10ScaleEngine);
    _plot->setAxisScaleEngine(QwtPlot::yLeft, log);
  else
    _plot->setAxisScaleEngine(QwtPlot::yLeft, new QwtLinearScaleEngine);

  /** check if autoscale, and set the axis limits */
  if (_xControl->autoscale())
    _plot->setAxisAutoScale(QwtPlot::xBottom);
  else
    _plot->setAxisScale(QwtPlot::xBottom,_xControl->min(),_xControl->max());

  if (_yControl->autoscale())
    _plot->setAxisAutoScale(QwtPlot::yLeft);
  else
    _plot->setAxisScale(QwtPlot::yLeft,_yControl->min(),_yControl->max());

  /** update the layout and replot the plot */
  _plot->updateLayout();
  _plot->replot();

  /** save the states of the controls */
  QSettings settings;
  settings.beginGroup(windowTitle());
  settings.setValue("CurveColor",_curves[0]->pen().color());
  settings.setValue("CurveWidth",_curves[0]->pen().width());
  settings.setValue("GridEnabled",_gridControl->isChecked());
  settings.setValue("LegendShown",_legendControl->isChecked());
}

void OneDViewer::on_legend_right_clicked(QPoint pos)
{
  /** check if the sender of the signal is of widget type (is the legend item) */
  if (!sender()->isWidgetType())
    return;

  /** retrieve the legenditem widget and the corresponding curve, determine the
   *  position where the right click happen to be able to open the menu at this
   *  position
   */
  QWidget *curveWidget(dynamic_cast<QWidget*>(sender()));
  PlotCurve *curve(dynamic_cast<PlotCurve*>(_legend->find(curveWidget)));
  QPoint globalPos(curveWidget->mapToGlobal(pos));

  /** create the context menu and execute it (block in this function until a
   *  choice has been made).
   *
   *  In case this is the main curve (name is the name of the main curve), don't
   *  add the option to delete the curve. Otherwise create and connect their
   *  triggered signals to the appropriate private slots that will react on
   *  the choice.
   */
  QMenu menu;
  QAction *act;
  act = menu.addAction(tr("Color"),this,SLOT(change_curve_color()));
  act->setParent(curveWidget);
  act = menu.addAction(tr("Line Width"),this,SLOT(change_curve_width()));
  act->setParent(curveWidget);
  if(curve->title() != QwtText("current data"))
  {
    act = menu.addSeparator();
    act = menu.addAction(tr("Delete"),this,SLOT(remove_curve()));
    act->setParent(curveWidget);
  }
  menu.exec(globalPos);
}

void OneDViewer::on_legend_clicked(QwtPlotItem *item)
{
  if(item->isVisible())
    item->hide();
  else
    item->show();
  replot();
}

void OneDViewer::change_curve_color(PlotCurve *curve)
{
  if (!curve)
  {
    if (!sender()->parent()->isWidgetType())
      return;
    QWidget *curveWidget(dynamic_cast<QWidget*>(sender()->parent()));
    curve = dynamic_cast<PlotCurve*>(_legend->find(curveWidget));
  }

  QPen pen(curve->pen());

  QColor col(QColorDialog::getColor(pen.color(),this,tr("Select Color")));
  if (col.isValid())
    pen.setColor(col);

  curve->setPen(pen);
  replot();
}

void OneDViewer::change_curve_width(PlotCurve *curve)
{
  if (!curve)
  {
    if (!sender()->parent()->isWidgetType())
      return;
    QWidget *curveWidget(dynamic_cast<QWidget*>(sender()->parent()));
    curve = dynamic_cast<PlotCurve*>(_legend->find(curveWidget));
  }

  QPen pen(curve->pen());

  bool ok(false);
  int width(QInputDialog::getInt(this,tr("Set Line Width"),tr("Line width"),pen.width(),0,20,1,&ok));
  if (ok)
    pen.setWidth(width);

  curve->setPen(pen);
  replot();
}

void OneDViewer::remove_curve(PlotCurve *curve)
{
  if (!curve)
  {
    if (!sender()->parent()->isWidgetType())
      return;
    QWidget *curveWidget(dynamic_cast<QWidget*>(sender()->parent()));
    curve = dynamic_cast<PlotCurve*>(_legend->find(curveWidget));
  }

  _curves.removeAll(curve);
  curve->detach();
  _legend->update();

  replot();
}

void OneDViewer::on_add_graph_triggered()
{
  QString filter("Data Files (*.csv *.hst *.h5 *.hdf5)");
  QString fileName = QFileDialog::getOpenFileName(this, tr("Open Reference Data File"), QDir::currentPath(), filter);
  if(fileName.isEmpty())
    return;
  cass::HistogramBackend *data(FileHandler::getData(fileName));
  if (data->dimension() == 1)
    addData(dynamic_cast<cass::Histogram1DFloat*>(data));
  else
    QMessageBox::critical(this,tr("Error"),tr("The requested data is not 1d data"));
}
