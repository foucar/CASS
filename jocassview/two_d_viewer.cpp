// Copyright (C) 2013 Lutz Foucar

/**
 * @file two_d_viewer.cpp contains viewer for 2d data
 *
 * @author Lutz Foucar
 */

#include <QtCore/QDebug>
#include <QtCore/QSettings>
#include <QtCore/QFileInfo>

#include <QtGui/QVBoxLayout>
#include <QtGui/QToolBar>
#include <QtGui/QSpinBox>
#include <QtGui/QLabel>
#include <QtGui/QFileDialog>
#include <QtGui/QInputDialog>

#include <qwt_plot.h>
#include <qwt_scale_widget.h>
#include <qwt_plot_layout.h>
#include <qwt_plot_spectrogram.h>
#include <qwt_color_map.h>
#include <qwt_scale_engine.h>

#include "two_d_viewer.h"

#include "two_d_viewer_data.h"
#include "histogram.h"
#include "minmax_control.h"
#include "track_zoomer_2d.h"
#include "logcolor_map.h"
#include "data.h"
#include "file_handler.h"
#include "geom_parser.h"

using namespace jocassview;
using namespace cass;

TwoDViewer::TwoDViewer(QString title, QWidget *parent)
  : DataViewer(title,parent)
{
  // settings to read from the ini file
  QSettings settings;
  settings.beginGroup(windowTitle());
  _geomFile = settings.value("GeomFile","").toString();

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
  _spectrogram = new QwtPlotSpectrogram();
  _spectrogram->setData(data);
  _spectrogram->attach(_plot);
  // create a zoomer for the 2d data
  _zoomer = new TrackZoomer2D(_plot->canvas());
//  _zoomer->setSelectionFlags( QwtPicker::RectSelection | QwtPicker::DragSelection );
  _zoomer->setMousePattern(QwtEventPattern::MouseSelect2,
                           Qt::RightButton, Qt::ControlModifier);
  _zoomer->setMousePattern(QwtEventPattern::MouseSelect3,
                           Qt::RightButton);
  _zoomer->setData(data);
  _zoomer->setWavelength_A(settings.value("Wavelength_A",5).toDouble());
  _zoomer->setCameraDistance_cm(settings.value("CameraDistance_cm",7).toDouble());
  _zoomer->setPixelSize_um(settings.value("PixelSize_um",110).toDouble());

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

  // Add a button that allows to add a reference curve
  toolbar->addAction(QIcon(":images/graph_add.png"),
                     tr("Load geom file"),
                     this,SLOT(on_load_geomfile_triggered()));

  // add the min/max control to the toolbar
  _zControl = new MinMaxControl(QString(windowTitle() + "/z-scale"),toolbar);
  connect(_zControl,SIGNAL(controls_changed()),this,SLOT(replot()));
  toolbar->addWidget(_zControl);

  // Add separator
  toolbar->addSeparator();

  // Add the colorbar control
  _colorId = new QSpinBox();
  _colorId->setRange(-4,11);
  _colorId->setValue(settings.value("ColorTableID",-1).toInt());
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

QList<Data*> TwoDViewer::data()
{
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
  if (data().isEmpty())
    return;
  FileHandler::saveData(filename,data().front()->result());
}

void TwoDViewer::dataChanged()
{
  /** check if the user wants to convert cheetah layout to lab frame */
  if (!_geomFile.isEmpty())
  {
    Histogram2DFloat * hist(dynamic_cast<Histogram2DFloat*>(data().front()->result()));
//    _origHist = dynamic_cast<Histogram2DFloat*>(hist->copyclone());

    GeometryInfo::lookupTable_t lut =
        GeometryInfo::generateLookupTable(_geomFile.toStdString(),
                                          hist->memory().size(),
                                          hist->axis()[HistogramBackend::xAxis].size(),
                                          false);

    Histogram2DFloat *labHist(
          new Histogram2DFloat(lut.nCols,lut.min.x,lut.max.x,
                               lut.nRows,lut.min.y,lut.max.y, "cols", "rows"));
    labHist->key() = hist->key();
    Histogram2DFloat::storage_t& destImage(labHist->memory());

    Histogram2DFloat::storage_t::const_iterator srcpixel(hist->memory().begin());
    Histogram2DFloat::storage_t::const_iterator srcImageEnd(hist->memory().end()-8);

    std::vector<size_t>::const_iterator idx(lut.lut.begin());

    for (; srcpixel != srcImageEnd; ++srcpixel, ++idx)
      destImage[*idx] = *srcpixel;

    data().front()->setResult(labHist);
  }

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

QStringList TwoDViewer::dataFileSuffixes() const
{
  QStringList list;
  list << "h5"<<"hst"<<"csv"<<"png";
  return list;
}

void TwoDViewer::replot()
{
  /** @note we need to new the color bar for both the axis widget and the spectrogram
   *        as they take over possesion of the colorbar and delete them when they
   *        think appropriate.
   */

  /** get the data from the spectrogram and get the min and max z-values to be displayed */
  TwoDViewerData *data(dynamic_cast<TwoDViewerData*>(_spectrogram->data()));
  const double min(!_zControl->autoscale() ? _zControl->min() : data->origZInterval(_zControl->log()).minValue());
  const double max(!_zControl->autoscale() ? _zControl->max() : data->origZInterval(_zControl->log()).maxValue());

  /** get the colormap to be used */
  int colorid = _colorId->value();

  /** set the colormap and min / max z-value */
  data->setInterval(Qt::ZAxis,QwtInterval(min,max));
  _spectrogram->setColorMap(cmap(colorid,_zControl->log()));
  _plot->axisWidget(QwtPlot::yRight)->setColorMap(_spectrogram->data()->interval(Qt::ZAxis),cmap(colorid,_zControl->log()));
  _plot->setAxisScale(QwtPlot::yRight,min,max);

  if (_zControl->log())
    _plot->setAxisScaleEngine(QwtPlot::yRight, new QwtLogScaleEngine);
  else
    _plot->setAxisScaleEngine(QwtPlot::yRight, new QwtLinearScaleEngine);

  /** display the axis titles if requested */
  if (_axisTitleControl->isChecked())
  {
    cass::HistogramBackend *hist(data->result());
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

void TwoDViewer::on_load_geomfile_triggered()
{
  QSettings settings;
  settings.beginGroup(windowTitle());

  /** reset the parameters */
  _geomFile.clear();
  double wavelength_A = 0;
  double cameraDistance_cm = 0;
  double pixelsize_um = 0;

  _zoomer->setWavelength_A(wavelength_A);
  _zoomer->setCameraDistance_cm(cameraDistance_cm);
  _zoomer->setPixelSize_um(pixelsize_um);


  /** open file dialog and retrieve the requested file */
  QString filter("Geom Files (*.geom)");
  QString filename = QFileDialog::getOpenFileName(this, tr("Load Geom File"),
                                                  QDir::currentPath(), filter);

  /** only set the file and resolution parameters if the file exits */
  if (!filename.isEmpty() && QFileInfo(filename).exists())
  {
    _geomFile = filename;

    bool ok(false);
    wavelength_A =
        QInputDialog::getDouble(this, tr("Set Wavelength [Angstroem]"),
                                tr("Wavelength [Angstroem]:"),
                                settings.value("Wavelength_A",5).toDouble(),
                                0, 20, 3, &ok);
    if (ok)
      _zoomer->setWavelength_A(wavelength_A);

    cameraDistance_cm =
        QInputDialog::getDouble(this, tr("Set Camera Distance [cm]"),
                                tr("Camera Distance [cm]:"),
                                settings.value("CameraDistance_cm",7).toDouble(),
                                0, 20, 3, &ok);
    if (ok)
      _zoomer->setCameraDistance_cm(cameraDistance_cm);

    pixelsize_um =
        QInputDialog::getDouble(this, tr("Set PixelSize [um]"),
                                tr("Pixel Size [um]:"),
                                settings.value("PixelSize_um",110).toDouble(),
                                0, 1000, 1, &ok);
    if (ok)
      _zoomer->setPixelSize_um(pixelsize_um);
  }
//  else
//  {
//    data().front()->setResult(_origHist);
//  }

  settings.setValue("GeomFile",_geomFile);
  settings.setValue("Wavelength_A",wavelength_A);
  settings.setValue("CameraDistance_cm",cameraDistance_cm);
  settings.setValue("PixelSize_um",pixelsize_um);

  dataChanged();
}

QwtLinearColorMap* TwoDViewer::cmap(const int colorid,bool log) const
{
  if (colorid == -4)
  {
    QwtLinearColorMap *map(log ? new LogColorMap(Qt::black, Qt::red) :
                                 new QwtLinearColorMap(Qt::black, Qt::red));
    map->addColorStop(0.999, QColor(Qt::white));
    map->addColorStop(0.001, QColor(Qt::white));
    return map;
  }
  if (colorid == -3)
  {
    QwtLinearColorMap *map(log ? new LogColorMap(Qt::black, Qt::black) :
                                 new QwtLinearColorMap(Qt::black, Qt::black));
    map->addColorStop(0.999, QColor(Qt::white));
    return map;
  }
  if (colorid == -2)
  {
    QwtLinearColorMap *map(log ? new LogColorMap(Qt::white, Qt::black) :
                                 new QwtLinearColorMap(Qt::white, Qt::black));
    return map;
  }
  if (colorid == -1)
  {
    QwtLinearColorMap *map(log ? new LogColorMap(Qt::black, Qt::white) :
                                 new QwtLinearColorMap(Qt::black, Qt::white));
    return map;
  }
  else if(colorid == 0)
  {
    QwtLinearColorMap *map(log ? new LogColorMap(Qt::darkCyan, Qt::red) :
                                 new QwtLinearColorMap(Qt::darkCyan, Qt::red));
    map->addColorStop(0.10, QColor(Qt::darkCyan));
    map->addColorStop(0.60, QColor(Qt::green));
    map->addColorStop(0.90, QColor(Qt::yellow));
    return map;
  }
  else if(colorid == 1)
  {
    QwtLinearColorMap *map(log ? new LogColorMap(Qt::black, QColor(255,0,0)) :
                                 new QwtLinearColorMap(Qt::black, QColor(255,0,0)));
    map->addColorStop(0.10, QColor(50,0,0));
    map->addColorStop(0.35, QColor(115,0,0));
    map->addColorStop(0.80, QColor(180,0,0));
    return map;
  }
  else if(colorid == 2)
  {
    QwtLinearColorMap *map(log ? new LogColorMap(Qt::black, QColor(0,255,0)) :
                                 new QwtLinearColorMap(Qt::black, QColor(0,255,0)));
    return map;
  }
  else if(colorid == 3)
  {
    QwtLinearColorMap *map(log ? new LogColorMap(Qt::black, QColor(0,0,255)) :
                                 new QwtLinearColorMap(Qt::black, QColor(0,0,255)));
    return map;
  }
  else if(colorid == 4)
  {
    QwtLinearColorMap *map(log ? new LogColorMap(Qt::black, QColor(255,0,255)) :
                                 new QwtLinearColorMap(Qt::black, QColor(255,0,255)));
    return map;
  }
  else if(colorid == 5)
  {
    QwtLinearColorMap *map(log ? new LogColorMap(Qt::black, QColor(0,255,255)) :
                                 new QwtLinearColorMap(Qt::black, QColor(0,255,255)));
    return map;
  }
  else if(colorid == 7)
  {
    QwtLinearColorMap *map(log ? new LogColorMap(Qt::black, QColor(255,255,0)) :
                                 new QwtLinearColorMap(Qt::black, QColor(255,255,0)));
    return map;
  }
  else if(colorid == 6)
  {
    QwtLinearColorMap *map(log ? new LogColorMap(Qt::black, Qt::red) :
                                 new QwtLinearColorMap(Qt::black, Qt::red));
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
    QwtLinearColorMap *map(log ? new LogColorMap(Qt::darkBlue, Qt::white) :
                                 new QwtLinearColorMap(Qt::darkBlue, Qt::white));
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
    QwtLinearColorMap *map(log ? new LogColorMap(QColor(65,105,241), QColor(255,51,204)) :
                                 new QwtLinearColorMap(QColor(65,105,241), QColor(255,51,204)));
    map->addColorStop(0.10, QColor(0,127,255));
    map->addColorStop(0.60, QColor(221,0,225));
    map->addColorStop(0.95, QColor(255,51,204));
    return map;
  }
  else if(colorid ==10)
  {
    QwtLinearColorMap *map(log ? new LogColorMap(QColor(72,6,7), Qt::white) :
                                 new QwtLinearColorMap(QColor(72,6,7), Qt::white));
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
    QwtLinearColorMap *map(log ? new LogColorMap(QColor(16,16,255), QColor(0,255,129)) :
                                 new QwtLinearColorMap(QColor(16,16,255), QColor(0,255,129)));
    map->addColorStop(0.10, QColor(16,16,255));
    map->addColorStop(0.50, Qt::cyan);
    map->addColorStop(0.90, QColor(0,255,155));
    return map;
  }
  else if(colorid ==12)
  {
    QwtLinearColorMap *map(log ? new LogColorMap(QColor(10,10,10), QColor(184,115,51)) :
                                 new QwtLinearColorMap(QColor(10,10,10), QColor(184,115,51)));
    map->addColorStop(0.10, QColor(10,10,10));
    map->addColorStop(0.20, QColor(149,34,0));
    map->addColorStop(0.90, QColor(184,115,51));
    return map;
  }
  else
  {
    return cmap(-1,log);
  }
}

QStringList TwoDViewer::cmaps()const
{
  QStringList list;
  return list;
}
