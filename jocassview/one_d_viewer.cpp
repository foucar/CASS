// Copyright (C) 2013 Lutz Foucar

/**
 * @file one_d_viewer.cpp contains viewer for 1d data
 *
 * @author Lutz Foucar
 */

#include <QtCore/QSettings>
#include <QtCore/QVector>
#include <QtCore/QDebug>
#include <QtCore/QTime>

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
#include <qwt_scale_widget.h>
#include <qwt_legend.h>
#include <qwt_scale_engine.h>
#include <qwt_legend_label.h>

#include "one_d_viewer.h"

#include "histogram.h"
#include "minmax_control.h"
#include "one_d_viewer_data.h"
#include "file_handler.h"
#include "curve_plot.h"
#include "data.h"
#include "data_source_manager.h"
#include "jocassviewer.h"

using namespace jocassview;
using namespace cass;

OneDViewer::OneDViewer(QString title, QWidget *parent)
  : DataViewer(title,parent)
{
  // add settings object to retrieve the settings for this view
  QSettings settings;
  settings.beginGroup(windowTitle());

  // Add the plot where the 1d data will be displayed as central widget
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
  _grid->setMajorPen(QPen(Qt::black, 0, Qt::DashLine));
  _grid->setMinorPen(QPen(Qt::black, 0, Qt::DotLine));
  _grid->attach(_plot);
  _gridLines = settings.value("GridEnabled",0).toUInt();
  // add a legend to the plot
  _legend = new QwtLegend;
  _legend->setDefaultItemMode(QwtLegendData::Checkable);
  _plot->insertLegend(_legend,QwtPlot::RightLegend);
  QwtLegendLabel *curveLegendLabel(qobject_cast<QwtLegendLabel *>(_legend->legendWidget(_plot->itemToInfo(_curves[0]))));
  curveLegendLabel->setContextMenuPolicy(Qt::CustomContextMenu);
  curveLegendLabel->setChecked(true);
  connect(curveLegendLabel,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(on_legend_right_clicked(QPoint)));
  connect(_legend,SIGNAL(checked(const QVariant&,bool,int)),this,SLOT(on_legend_checked(const QVariant &,bool)));
  // add the plot to the widget
  setCentralWidget(_plot);

  // create the toolbar
  QToolBar * toolbar(new QToolBar("Plot Control",this));
  addToolBar(Qt::BottomToolBarArea,toolbar);

  // Add a button that allows to add a reference curve
  toolbar->addAction(QIcon(":images/graph_add.png"),
                     tr("Add a reference Graph to the Plot"),
                     this,SLOT(on_add_graph_triggered()));

  // Add grid control to toolbar
  _gridControl = new QAction(QIcon(":images/grid.png"),
                             tr("toggle Grid"),toolbar);
  connect(_gridControl,SIGNAL(triggered()),this,SLOT(on_grid_triggered()));
  toolbar->addAction(_gridControl);

  // Add title display to the toolbar
  _axisTitleControl = new QAction(QIcon(":images/axistitle.png"),
                                  tr("Toggle Axis Titles"),toolbar);
  _axisTitleControl->setCheckable(true);
  _axisTitleControl->setChecked(settings.value("DisplayTitles",true).toBool());
  connect(_axisTitleControl,SIGNAL(triggered()),this,SLOT(replot()));
  toolbar->addAction(_axisTitleControl);

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
  QWidgetAction *xControlAction(new QWidgetAction(toolbar));
  xControlAction->setDefaultWidget(_xControl);
  toolbar->addAction(xControlAction);

  // Add separator to toolbar
  toolbar->addSeparator();

  // Add y-axis control to the toolbar
  _yControl = new MinMaxControl(QString(windowTitle() + "/y-scale"),toolbar);
  connect(_yControl,SIGNAL(controls_changed()),this,SLOT(replot()));
  QWidgetAction *yControlAction(new QWidgetAction(toolbar));
  yControlAction->setDefaultWidget(_yControl);
  toolbar->addAction(yControlAction);

  qsrand(QTime::currentTime().msec());
  // Set the size and position of the window
  resize(settings.value("WindowSize",size()).toSize());
  move(settings.value("WindowPosition",pos()).toPoint());

  settings.endGroup();
}

OneDViewer::~OneDViewer()
{

}

QList<Data *> OneDViewer::data()
{
  QList<Data*>list;
  QwtPlotItemList plotlist(_plot->itemList(QwtPlotItem::Rtti_PlotCurve));
  for (QwtPlotItemIterator it = plotlist.begin(); it != plotlist.end() ; ++it)
  {
    PlotCurve *curve(dynamic_cast<PlotCurve*>(*it));
    list.append(dynamic_cast<OneDViewerData*>(curve->data()));
  }
  return list;
}

QString OneDViewer::type() const
{
  return QString("1DViewer");
}

void OneDViewer::saveData(const QString &filename)
{
  QList<Data*> dataList(data());
  QList<Data*>::iterator dataIt(dataList.begin());
  while (dataIt != dataList.end())
  {
    QString fname(filename);
    if(!FileHandler::isContainerFile(filename))
      fname.insert(fname.lastIndexOf("."),"_" + QString::fromStdString((*dataIt)->result()->key()));
    FileHandler::saveData(filename,(*dataIt)->result());
    ++dataIt;
  }
}

void OneDViewer::dataChanged()
{
  replot();
}

QStringList OneDViewer::dataFileSuffixes() const
{
  QStringList list;
  list << "h5"<<"hst"<<"csv";
  return list;
}

void OneDViewer::replot()
{
  /** check if grid should be enabled */
  _grid->enableX(static_cast<bool>(_gridLines & 0x1));
  _grid->enableXMin(static_cast<bool>(_gridLines & 0x4));
  _grid->enableY(static_cast<bool>(_gridLines & 0x2));
  _grid->enableYMin(static_cast<bool>(_gridLines & 0x8));

  /** hide /show the legend (this is a hack, since legends can't be directly hidden)
   *  retrieve all curve plots from the plot and get theier corresponding
   *  legend widget. This needs to be hidden and then the legend to be updated
   */
  QwtPlotItemList list(_plot->itemList(QwtPlotItem::Rtti_PlotCurve));
  for (QwtPlotItemIterator it = list.begin(); it != list.end() ; ++it)
    _legend->legendWidget(_plot->itemToInfo(*it))->setVisible(_legendControl->isChecked());

  OneDViewerData *data(dynamic_cast<OneDViewerData*>(_curves[0]->data()));
  /** set the scales to be log or linear */
  data->setXRangeForLog(_xControl->log());
  if(_xControl->log())
    _plot->setAxisScaleEngine(QwtPlot::xBottom, new QwtLogScaleEngine);
  else
    _plot->setAxisScaleEngine(QwtPlot::xBottom, new QwtLinearScaleEngine);

  data->setYRangeForLog(_yControl->log());
  if(_yControl->log())
    _plot->setAxisScaleEngine(QwtPlot::yLeft, new QwtLogScaleEngine);
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

  /** display the axis titles */
  if (_axisTitleControl->isChecked())
  {
    if (!this->data().isEmpty())
    {
      cass::HistogramBackend *hist(this->data().front()->result());
      if (hist)
      {
        QString xtitle(QString::fromStdString(hist->axis()[cass::HistogramBackend::xAxis].title()));
        _plot->axisWidget(QwtPlot::xBottom)->setTitle(xtitle);
      }
    }
  }
  else
    _plot->axisWidget(QwtPlot::xBottom)->setTitle("");

  /** update the layout and replot the plot */
  _plot->updateLayout();
  _plot->replot();

  /** save the states of the controls */
  QSettings settings;
  settings.beginGroup(windowTitle());
  settings.setValue("CurveColor",_curves[0]->pen().color());
  settings.setValue("CurveWidth",_curves[0]->pen().width());
  settings.setValue("GridEnabled",_gridLines);
  settings.setValue("LegendShown",_legendControl->isChecked());
  settings.setValue("DisplayTitles",_axisTitleControl->isChecked());
  settings.endGroup();
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
  PlotCurve * curve(dynamic_cast<PlotCurve*>((_plot->infoToItem(_legend->itemInfo(curveWidget)))));
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

void OneDViewer::on_legend_checked(const QVariant &itemInfo, bool on)
{
  QwtPlotItem *curve(_plot->infoToItem(itemInfo));
  curve->setVisible(on);
  replot();
}

void OneDViewer::change_curve_color(PlotCurve *curve)
{
  if (!curve)
  {
    if (!sender()->parent()->isWidgetType())
      return;
    QWidget *curveWidget(dynamic_cast<QWidget*>(sender()->parent()));
    curve = dynamic_cast<PlotCurve*>(_plot->infoToItem(_legend->itemInfo(curveWidget)));
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
    curve = dynamic_cast<PlotCurve*>(_plot->infoToItem(_legend->itemInfo(curveWidget)));
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
    curve = dynamic_cast<PlotCurve*>(_plot->infoToItem(_legend->itemInfo(curveWidget)));
  }

  _curves.removeAll(curve);
  curve->detach();
  _legend->update();

  replot();
}

void OneDViewer::on_add_graph_triggered()
{
  /** ask the user from which source the added graph should come */
  QStringList sourceNames(DataSourceManager::sourceNames());
  sourceNames << "***New Source***";
  int currentIdx(sourceNames.indexOf(DataSourceManager::currentSourceName()));
  bool ok(false);
  QString sourceName(QInputDialog::getItem(0, QObject::tr("Select Source"),
                                     QObject::tr("Source:"), sourceNames,
                                     currentIdx, false, &ok));
  if (!ok || sourceName.isEmpty())
    return;
  /** if the source should be a new one, ask the user for the filename of the
   *  new source and add it to the list of sources
   */
  if (sourceName == "***New Source***")
  {
    QString filter("Data Files (*.csv *.hst *.h5 *.hdf5)");
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Open Reference Data File"),
                                                    QDir::currentPath(), filter);
    if(fileName.isEmpty())
      return;
    DataSourceManager::addSource(fileName,new FileHandler(fileName),false);
    sourceName = fileName;
  }

  /** retrieve the list of items that are available from the source, if more
   *  than 1 ask the user which should be added
   */
  DataSource * source(DataSourceManager::source(sourceName));
  QStringList items(source->resultNames());
  QString item(QInputDialog::getItem(0, QObject::tr("Select Key"),
                                     QObject::tr("Key:"), items, 0, false, &ok));
  if (!ok || item.isEmpty())
    return;

  /** retrieve the result from the source and check if it is a 1d result */
  HistogramBackend *result(source->result(item));
  if (!(result && result->dimension() ==1))
  {
    QMessageBox::critical(this,tr("Error"),tr("The requested data doesn't exit or is not 1d data"));
    return;
  }

  /** create a new data container and add the result to it */
  OneDViewerData *data(new OneDViewerData);
  data->setSourceName(sourceName);
  data->setResult(result);

  /** add a new curve to the plot and intialize it with the data and  a random
   *  color then set up the legenditem and its context menu for the curve
   */
  _curves.push_back(new PlotCurve);
  PlotCurve * curve(_curves.back());
  QPen pen;
  pen.setWidth(1);
  pen.setColor(QColor::fromHsv(qrand() % 256, 255, 190));
  curve->setPen(pen);
  curve->setTitle(item);
  curve->setStyle(QwtPlotCurve::Steps);
  curve->attach(_plot);
  curve->setData(data);
  QwtLegendLabel *curveWidget(qobject_cast<QwtLegendLabel*>(_legend->legendWidget(_plot->itemToInfo(curve))));
  curveWidget->setContextMenuPolicy(Qt::CustomContextMenu);
  curveWidget->setChecked(true);
  connect(curveWidget,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(on_legend_right_clicked(QPoint)));

  /** and replot the plot */
  replot();
}

void OneDViewer::on_grid_triggered()
{
  _gridLines = (_gridLines+1) & 0xF;
  while (_gridLines == 4 ||
         _gridLines == 6 ||
         _gridLines == 8 ||
         _gridLines == 9 ||
         _gridLines == 12 ||
         _gridLines == 13 ||
         _gridLines == 14)
    _gridLines = (_gridLines+1) & 0xF;
  replot();
}
