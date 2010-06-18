#ifndef PLOTWIDGET_H
#define PLOTWIDGET_H


#include <QCoreApplication>
#include <string>
#include <iostream>

#include <qwt_plot.h>
#include <qwt_plot_grid.h>
#include <qwt_legend.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_zoomer.h>
#include <qwt_plot_rescaler.h>
#include <qwt_plot_spectrogram.h>
#include <qwt_plot_layout.h>
#include <qwt_scale_widget.h>
#include <qwt_scale_engine.h>
#include <qwt_scale_draw.h>
#include <qwt_color_map.h>
#include <qdialog.h>
#include <QDockWidget>
#include <QLabel>
#include <QLayout>
#include <QToolBar>
#include <QPushButton>
#include <QComboBox>
#include <QRadioButton>
#include <QCheckBox>
#include <QSpinBox>
#include <QAction>
#include <QFont>
#include <QQueue>
#include <QMouseEvent>
// derived qwt classes:
#include "qwt_logcolor_map.h"
#include "qwt_scroll_zoomer.h"
#include "../cass/cass_event.h"
#include "../cass/serializer.h"
#include "../cass/cass.h"
#include "../cass/histogram.h"
#include "../cass/postprocessing/postprocessor.h"
#include "soapCASSsoapProxy.h"

#include <math.h>


// prototypes:
class cassData;

     

class NaNCurve : public QwtPlotCurve
{
public:

    QwtDoubleRect boundingRect() const
    {
        const size_t sz = dataSize();

        if ( sz <= 0 )
            return QwtDoubleRect(1.0, 1.0, -2.0, -2.0); // invalid

        double minX, maxX, minY, maxY;
        minX = maxX = x(0);
        minY = maxY = y(0);

        for ( size_t i = 1; i < sz; i++ )
        {
            const double xv = x(i);
            if ( xv < minX )
                minX = xv;
            if ( xv > maxX )
                maxX = xv;

            const double yv = y(i);
            if ( yv < minY && !(isnan(yv) || isinf(yv)))
                minY = yv;
            if ( yv > maxY && !(isnan(yv) || isinf(yv)))
                maxY = yv;
        }
        return QwtDoubleRect(minX, minY, maxX - minX, maxY - minY);
    } 

    NaNCurve() : QwtPlotCurve() {}
    NaNCurve(QString& str) : QwtPlotCurve(str) {}
    void draw(QPainter *painter,
        const QwtScaleMap &xMap, const QwtScaleMap &yMap,
        int from, int to) const
    {   
        if (to < 0)
          to = dataSize() - 1;
        // paint curve, but leave out all NaNs and Infs.
        int ii=from;
        while(ii<=to)
        {
            int _from;
            while( (isnan(y(ii)) || isinf(y(ii))) && ii<to) ++ii;
            _from = ii;
            while( !(isnan(y(ii)) || isinf(y(ii))) && ii<to) ++ii;
            if (ii>=to) ii=to+1;
            QwtPlotCurve::draw(painter, xMap, yMap, _from, ii-1);
        }
    }
};



class EremoveCurve : public QEvent
{
public:
  EremoveCurve(QEvent::Type type):QEvent(type) {}
  NaNCurve* curve;
  QWidget* curveWidget;
  //Type type() { return static_cast<Type>(QEvent::User+111); }
};


class createScaleEngine
{
public:
  virtual QwtScaleEngine* create() = 0;
  virtual ~createScaleEngine(){}
};

class createLinearScaleEngine : public createScaleEngine
{
public:
  QwtScaleEngine* create() { return new QwtLinearScaleEngine; }
};

class createLog10ScaleEngine : public createScaleEngine
{
public:
  QwtScaleEngine* create() { return new QwtLog10ScaleEngine; }
};

class MyPlot: public QwtPlot
{
public:
  MyPlot(QWidget *parent=NULL)
    :QwtPlot(parent)
  {
    setMouseTracking(true);
  }

  void mouseMoveEvent ( QMouseEvent * /*event */)
  {
    //double yval = invTransform(QwtPlot::yRight, event->pos().y()) ;
    //std::cout << "scalewidget mousepressevent yval" <<yval << std::endl;
  }
};

class TrackZoomer1D: public ScrollZoomer
{
public:
  TrackZoomer1D(QwtPlotCanvas *canvas):
      ScrollZoomer(canvas), _hist(NULL)
  {
    setTrackerMode(AlwaysOn);
  }

  virtual QwtText trackerText(const QwtDoublePoint &pos) const
  {
    QColor bg(Qt::white);
    bg.setAlpha(200);

    QwtText text = QwtPlotZoomer::trackerText(pos);
    QString text_string(text.text());
    try {
      if (_hist) text_string = text_string + " : " + QString::number( (*_hist)(pos.x()) );
    }
    catch (std::out_of_range)
    {
      //
    }
    text.setText(text_string);        // todo: format numbers to display xx.xxexx format for small/big numbers...
    text.setBackgroundBrush( QBrush( bg ));
    return text;
  }

  void setHistogram(cass::Histogram1DFloat* hist) { _hist = hist; }

protected:
  cass::Histogram1DFloat* _hist;
};



class TrackZoomer2D: public ScrollZoomer /*QwtPlotZoomer*/
{
public:
  TrackZoomer2D(QwtPlotCanvas *canvas):
      //QwtPlotZoomer(canvas), _hist(NULL)
      ScrollZoomer(canvas), _hist(NULL)
  {
    setTrackerMode(AlwaysOn);
  }

  virtual QwtText trackerText(const QwtDoublePoint &pos) const
  {
    QColor bg(Qt::white);
    bg.setAlpha(200);

    QwtText text = QwtPlotZoomer::trackerText(pos);
    QString text_string(text.text());
    try
    {
      if (_hist) text_string = text_string + " : " + QString::number( (*_hist)(pos.x(), pos.y()) );
    }
    catch (std::out_of_range)
    {
      //
    }
    text.setText(text_string);
    text.setBackgroundBrush( QBrush( bg ));
    return text;
  }

  void setHistogram(cass::Histogram2DFloat* hist) { _hist = hist; }
protected:
  cass::Histogram2DFloat* _hist;
};


class spectrogramDataDummy: public QwtRasterData
{
public:
  spectrogramDataDummy():
      QwtRasterData(QwtDoubleRect(-1.5, -1.5, 3.0, 3.0))
  {}

  virtual QwtRasterData *copy() const
  {
    return new spectrogramDataDummy();
  }
  void setHistogram(cass::Histogram2DFloat* /*hist*/)
  {}

  virtual QwtDoubleInterval range() const
  {
    return QwtDoubleInterval(0.0, 10.0);
  }

  virtual double value(double x, double y) const
  {
    const double c = 0.842;

    const double v1 = x * x + (y-c) * (y+c);
    const double v2 = x * (y+c) + x * (y+c);

    return 1.0 / (v1 * v1 + v2 * v2);
  }
};


class spectrogramData: public QwtRasterData
{
public:
  spectrogramData():
      QwtRasterData(QwtDoubleRect(0.0, 0.0,500 , 500.0)), _hist(NULL), _boundRect(0.0, 0.0, 100.0, 100.0), _interval(QwtDoubleInterval(0.0, 1.0))
  {
    VERBOSEOUT(std::cout << "spectrogramdata default constructor" << std::endl);
  }

  ~spectrogramData()
  {
    //delete _hist; // don't delete: histogram is owned by caller of setHistogram.
  }

  spectrogramData( cass::Histogram2DFloat* hist, QwtDoubleRect brect, QwtDoubleInterval interval) :
      QwtRasterData(brect), _hist(hist), _boundRect(brect), _interval(interval)
  {
    //setI
    VERBOSEOUT(std::cout << "spectrogramdata overloaded constructor" << std::endl);
  }

  void setHistogram(cass::Histogram2DFloat *hist,
                    bool manualScale, double min, double max)
  {
    //delete _hist;   // don't delete: spectrogram keeps a shallow copy of spectrogramdata and calls destructor in setData.
    _hist = hist;
    VERBOSEOUT(std::cout << "SpectorgramData::setHistogram()" << std::endl);
    if (_hist)
    {
      _interval.setMinValue(manualScale? min : _hist->min() );
      _interval.setMaxValue(manualScale? max : _hist->max() );
      _boundRect.setCoords(_hist->axis()[cass::HistogramBackend::xAxis].lowerLimit(),
                           _hist->axis()[cass::HistogramBackend::yAxis].upperLimit(),
                           _hist->axis()[cass::HistogramBackend::xAxis].upperLimit(),
                           _hist->axis()[cass::HistogramBackend::yAxis].lowerLimit());
      VERBOSEOUT(std::cout<<"SpectorgramData::setHistogram(): hist min : "<< _hist->min()<<" max: "<<_hist->max()
                 <<" hist left : "<<_boundRect.left()
                 <<" hist right : "<<_boundRect.right()
                 <<" hist top : "<<_boundRect.top()
                 <<" hist bottom : "<<_boundRect.bottom()
                 <<" hist width : "<<_boundRect.width()
                 <<" hist height : "<<_boundRect.height()
                 << std::endl);
    }
    setBoundingRect( _boundRect );
  }

  const cass::Histogram2DFloat *histogram() const
  {
    return _hist;
  }

  virtual QwtRasterData *copy() const
  {
    VERBOSEOUT(std::cout <<"spectrogramData::copy()"<<std::endl);
    return new spectrogramData(_hist, _boundRect, _interval);
  }

  virtual QwtDoubleInterval range() const
  {
    VERBOSEOUT(std::cout << "spectrogramData::range(): " << _interval.minValue() << " " <<_interval.maxValue()  << std::endl);
    return _interval;
//    return QwtDoubleInterval(0,1500);
  }

  virtual double value(double x, double y) const
  {
    try
    {
      return (*_hist)(x,y);
    }
    catch (std::out_of_range)
    {
      return(0.0);  // todo: this shouldn't happen if bounding box is set correctly
    }
  }

protected:
  cass::Histogram2DFloat* _hist;
  QwtDoubleRect _boundRect;
  QwtDoubleInterval _interval;
};



/** spectrogramWidget
   * widget that can display 2d histograms.
   * usage:
   *   left mouse drag zooms
   *   right mouse goes back in zoom history
   *   left mouse drag on colorbar sets colorstops
   *   right mouse on colorbar cycles throug axis transformations
   * @todo make different color scale available
   * @author Stephan Kassemeyer
   * @author Nicola Coppola
   */
class spectrogramWidget : public QWidget
{
  Q_OBJECT

public:

  bool eventFilter(QObject *obj, QEvent *event)
  {
    if (obj == _rightAxis )
    {
      if (event->type() == QEvent::MouseButtonPress)
      {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->button() == Qt::RightButton)
        {
          if((++_cb_scaleEngineIt)==_cb_scaleEngines->end())
              _cb_scaleEngineIt = _cb_scaleEngines->begin();
          updateColorBarScale();
        }
      }
      if (event->type() == QEvent::MouseMove)
      {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        double yval= _plot->invTransform(QwtPlot::yRight, mouseEvent->pos().y());
        QwtDoubleInterval range = _spectrogram->data().range();
        double ystep = (yval - range.minValue() ) / (range.maxValue()-range.minValue());

        if (ystep>0 && ystep<1)
        {
          float topdiff = std::abs(ystep-_cs_top);
          float botdiff = std::abs(ystep-_cs_bot);
          if (topdiff<botdiff)
            _cs_top=ystep;
          else
            _cs_bot=ystep;

          updateColorBar(_color_scale->value());
        }
      }
    }
    // pass the event on to the parent class
    return QWidget::eventFilter(obj, event);
  }


  void mouseMoveEvent ( QMouseEvent * /* event */)
  {
    //double yval = _plot->invTransform(QwtPlot::yRight, event->pos().y()) ;
    //std::cout << "scalewidget mousepressevent yval" <<yval << std::endl;
  }


  spectrogramWidget()
  {
    _cs_top = 0.7;
    _cs_bot = 0.1;
    _cb_scaleEngines = new QList< createScaleEngine* >();
    _cb_scaleEngines->append( new createLinearScaleEngine );
    _cb_scaleEngines->append( new createLog10ScaleEngine );
    _cb_scaleEngineIt = _cb_scaleEngines->begin();

    _toolbar = new QToolBar;
    _colorbarPresets = new QComboBox;
    _rad_colormap_lin = new QRadioButton(tr("lin"));
    _rad_colormap_log = new QRadioButton(tr("log"));
    _rad_colormap_exp = new QRadioButton(tr("exp"));
    _rad_colormap_sqrt = new QRadioButton(tr("sqrt"));
    _rad_colormap_sq = new QRadioButton(tr("square"));

    _colorbarPresets->setEditable(true);
    _saveColorbar = new QPushButton(tr("save colorbar"));

    _bool_auto_scale = new QCheckBox(tr("man scale"));
    _bool_auto_scale->setChecked( FALSE );
    QLabel* _lbl_scale_min = new QLabel(tr("Min"),this);
    _sbx_scale_min = new QDoubleSpinBox(this);
    _sbx_scale_min->setRange(-2e12,2e12);
    _sbx_scale_min->setValue(0.);
    _sbx_scale_min->setDecimals(3);

    QLabel* _lbl_scale_max = new QLabel(tr("Max"),this);
    _sbx_scale_max = new QDoubleSpinBox(this);
    _sbx_scale_max->setRange(-2e12,2e12);
    _sbx_scale_max->setValue(1500.);
    _sbx_scale_max->setDecimals(3);

    if(_sbx_scale_max->value() < _sbx_scale_min->value())
    {
      _sbx_scale_min->setStyleSheet("QDoubleSpinBox {color: blue; background-color: #FF0000}");
      _sbx_scale_max->setStyleSheet("QDoubleSpinBox {color: blue; background-color: #FF0000}");
    }
    else
    {
      _sbx_scale_min->setStyleSheet("QDoubleSpinBox {color: black; background-color: #000000}");
      _sbx_scale_max->setStyleSheet("QDoubleSpinBox {color: black; background-color: #000000}");
    }

    QLabel* _lbl_color_scale = new QLabel(tr("Color"),this);
    _color_scale = new QSpinBox(this);
    _color_scale->setRange(-1,5);
    _color_scale->setValue(-1);

    // populate colorbar presets:
    QSettings settings;
    settings.beginGroup("ColorBar");
    _colorbarPresets->addItems( settings.childGroups() );
    QString current = settings.value("current","default").toString();
    _colorbarPresets->setCurrentIndex( _colorbarPresets->findText(current) );
    _colorbarPresets->setEditText(current);  // in case it didn't match

    connect(_rad_colormap_lin, SIGNAL(toggled(bool)), this, SLOT(changeColorIntLin(bool)));
    connect(_rad_colormap_log, SIGNAL(toggled(bool)), this, SLOT(changeColorIntLog(bool)));
    connect(_rad_colormap_exp, SIGNAL(toggled(bool)), this, SLOT(changeColorIntExp(bool)));
    connect(_rad_colormap_sqrt, SIGNAL(toggled(bool)), this, SLOT(changeColorIntSqrt(bool)));
    connect(_rad_colormap_sq, SIGNAL(toggled(bool)), this, SLOT(changeColorIntSquare(bool)));
    connect(_saveColorbar, SIGNAL(clicked()), this, SLOT(saveColorbar()));
    connect(_colorbarPresets, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(on_colorbarPreset_changed(const QString&)));

    connect(_color_scale, SIGNAL(valueChanged(int)), this, SLOT(updateColorBar(int)));

    connect(_sbx_scale_max, SIGNAL(valueChanged(double)), this, SLOT(Replot()));
    connect(_sbx_scale_min, SIGNAL(valueChanged(double)), this, SLOT(Replot()));

    _toolbar->addWidget( _colorbarPresets );
    _toolbar->addWidget( _saveColorbar );
    _toolbar->addWidget( _rad_colormap_lin );
    _toolbar->addWidget( _rad_colormap_log );
    _toolbar->addWidget( _rad_colormap_exp );
    _toolbar->addWidget( _rad_colormap_sqrt );
    _toolbar->addWidget( _rad_colormap_sq );
    _toolbar->addSeparator();
    _toolbar->addWidget(_bool_auto_scale);
    _toolbar->addWidget(_lbl_scale_min);
    _toolbar->addWidget(_sbx_scale_min);
    _toolbar->addWidget(_lbl_scale_max);
    _toolbar->addWidget(_sbx_scale_max);

    _toolbar->addWidget(_lbl_color_scale);
    _toolbar->addWidget(_color_scale);

    setMouseTracking(true);
    //_transformCol = QwtLogColorMap::trans_pow10;
    //_transformCol_inv = QwtLogColorMap::trans_log10;
    _transformCol = QwtLogColorMap::trans_lin;
    _transformCol_inv = QwtLogColorMap::trans_log10;
    _spectrogramData = new spectrogramData;
    _spectrogramDataDummy = new spectrogramDataDummy();
    _spectrogram = new QwtPlotSpectrogram();
    _plot = new MyPlot;

    _colorMap = new QwtLogColorMap(QColor(0,0,0), QColor(255,255,255));
    _colorMap->addColorStop(0.7, QColor(255,255,255));
    _colorMap->addColorStop(0.2, QColor(0,0,0));
    _colorMap->setTransformId(_transformCol);
    _spectrogram->setColorMap(*_colorMap);

    _spectrogram->setData(*_spectrogramData);
    _spectrogram->attach(_plot);

    _colorMapCol1 = new QwtLogColorMap(Qt::darkCyan, Qt::red);
    _colorMapCol1->addColorStop(0.1, Qt::cyan);
    _colorMapCol1->addColorStop(0.6, Qt::green);
    _colorMapCol1->addColorStop(0.95, Qt::yellow);
    _colorMapCol1->setTransformId(_transformCol);

    _colorMapColMany = new QwtLogColorMap(Qt::darkCyan, Qt::red);
    _colorMapColMany->addColorStop(0.1, Qt::cyan);
    _colorMapColMany->addColorStop(0.6, Qt::green);
    _colorMapColMany->addColorStop(0.95, Qt::yellow);
    _colorMapColMany->setTransformId(_transformCol);

    _colorMapColRed = new QwtLogColorMap(QColor(0,0,0), QColor(255,0,0));
    _colorMapColRed->addColorStop(_cs_top, QColor(180,0,0));
    _colorMapColRed->addColorStop(_cs_bot, QColor(50,0,0));
    _colorMapColRed->setTransformId(_transformCol);

    _colorMapColGreen = new QwtLogColorMap(QColor(0,0,0), QColor(0,255,0));
    _colorMapColGreen->addColorStop(_cs_top, QColor(0,180,0));
    _colorMapColGreen->addColorStop(_cs_bot, QColor(0,50,0));
    _colorMapColGreen->setTransformId(_transformCol);

    _colorMapColBlue = new QwtLogColorMap(QColor(0,0,0), QColor(0,0,255));
    _colorMapColBlue->addColorStop(_cs_top, QColor(0,0,180));
    _colorMapColBlue->addColorStop(_cs_bot, QColor(0,0,50));
    _colorMapColBlue->setTransformId(_transformCol);

    _colorMapColVio = new QwtLogColorMap(QColor(0,0,0), QColor(0,255,255));
    _colorMapColVio->addColorStop(_cs_top, QColor(0,180,180));
    _colorMapColVio->addColorStop(_cs_bot, QColor(0,50,50));
    _colorMapColVio->setTransformId(_transformCol);

    _colorMapColCyn = new QwtLogColorMap(QColor(0,0,0), QColor(255,0,255));
    _colorMapColCyn->addColorStop(_cs_top, QColor(180,0,180));
    _colorMapColCyn->addColorStop(_cs_bot, QColor(50,0,50));
    _colorMapColCyn->setTransformId(_transformCol);

    // A color bar on the right axis
    _rightAxis = _plot->axisWidget(QwtPlot::yRight);
    _rightAxis->setTitle("Intensity");
    _rightAxis->setColorBarEnabled(true);
    _rightAxis->installEventFilter(this);

    _colorMapInv = new QwtLogColorMap(Qt::darkCyan, Qt::red);
    _colorMapInv->addColorStop(0.1, Qt::cyan);
    _colorMapInv->addColorStop(0.6, Qt::green);
    _colorMapInv->addColorStop(0.95, Qt::yellow);
    _colorMapInv->setTransformId(_transformCol_inv);
    //_spectrogram->setColorMap(*_colorMapInv);

    _spectrogram->setColorMap(*_colorMap);
    _rightAxis->setColorMap(_spectrogram->data().range(),
                            *_colorMap);
                            
    //_rightAxis->setColorMap(_spectrogram->data().range(),
    //                        *_colorMapInv);
    //_plot->setAxisScaleEngine(QwtPlot::yRight, new QwtLog10ScaleEngine);
    _plot->setAxisScale(QwtPlot::yRight,
                        _spectrogram->data().range().minValue(),
                        _spectrogram->data().range().maxValue() );
    _plot->enableAxis(QwtPlot::yRight);


    QwtPlotRescaler dataPlotRescaler(_plot->canvas());
    dataPlotRescaler.setReferenceAxis(QwtPlot::xBottom);
    dataPlotRescaler.setAspectRatio(QwtPlot::yLeft, 1.0);
    dataPlotRescaler.setAspectRatio(QwtPlot::yRight, 0.0);
    dataPlotRescaler.setAspectRatio(QwtPlot::xTop, 0.0);

    _zoomer = new TrackZoomer2D(_plot->canvas());
    _zoomer->setSelectionFlags( QwtPicker::RectSelection | QwtPicker::DragSelection );
    _zoomer->setMousePattern(QwtEventPattern::MouseSelect2,
                             Qt::RightButton, Qt::ControlModifier);
    _zoomer->setMousePattern(QwtEventPattern::MouseSelect3,
                             Qt::RightButton);
    _layout.addWidget(_plot);
    _layout.addWidget(_toolbar);
    setLayout(&_layout);
    _spectrogram->setDisplayMode(QwtPlotSpectrogram::ContourMode, true);
    _plot->plotLayout()->setAlignCanvasToScales(true);
    loadColorbar( current );
    _plot->replot();
  }

  ~spectrogramWidget() {
     delete _colorMap;

     delete _colorMapCol1;
     delete _colorMapColMany;
     delete _colorMapColBlue;
     delete _colorMapColRed;
     delete _colorMapColGreen;

     delete _colorMapColVio;
     delete _colorMapColCyn;

     delete _colorMapInv;
     delete _spectrogram;
     delete _spectrogramDataDummy;
     delete _spectrogramData;
     while (!_cb_scaleEngines->isEmpty())
         delete _cb_scaleEngines->takeFirst();
     delete _cb_scaleEngines;
     delete _toolbar;
     delete _colorbarPresets;
     delete _rad_colormap_lin;
     delete _rad_colormap_exp;
     delete _rad_colormap_sqrt;
     delete _rad_colormap_sq;
     
     delete _saveColorbar;
     delete _color_scale;
  }


  void setData(cass::Histogram2DFloat* hist)
  {
    static std::string oldKey("");

    _spectrogram->setData(*_spectrogramDataDummy); //hack
    dynamic_cast<TrackZoomer2D*>(_zoomer)->setHistogram(hist);
    _spectrogramData->setHistogram(hist,
                                   _bool_auto_scale->checkState(),
                                   _sbx_scale_min->value(),
                                   _sbx_scale_max->value() );
    if(_sbx_scale_max->value() < _sbx_scale_min->value())
    {
      _sbx_scale_min->setStyleSheet("QDoubleSpinBox {color: blue; background-color: #FF0000}");
      _sbx_scale_max->setStyleSheet("QDoubleSpinBox {color: blue; background-color: #FF0000}");
    }
    else
    {
      _sbx_scale_min->setStyleSheet("QDoubleSpinBox {color: black; background-color: #FFFFFF}");
      _sbx_scale_max->setStyleSheet("QDoubleSpinBox {color: black; background-color: #FFFFFF}");
    }

    _spectrogram->setData(*_spectrogramData);   //hack
    _spectrogram->invalidateCache();
    _spectrogram->itemChanged();

    if(_color_scale->value()==-1)
      _rightAxis->setColorMap(_spectrogram->data().range(), *_colorMap);
    else if(_color_scale->value()==0)
      _rightAxis->setColorMap(_spectrogram->data().range(), *_colorMapColMany);
    else if(_color_scale->value()==1)
      _rightAxis->setColorMap(_spectrogram->data().range(), *_colorMapColRed);
    else if(_color_scale->value()==2)
      _rightAxis->setColorMap(_spectrogram->data().range(), *_colorMapColGreen);
    else if(_color_scale->value()==3)
      _rightAxis->setColorMap(_spectrogram->data().range(), *_colorMapColBlue);
    else if(_color_scale->value()==4)
      _rightAxis->setColorMap(_spectrogram->data().range(), *_colorMapColVio);
    else if(_color_scale->value()==5)
      _rightAxis->setColorMap(_spectrogram->data().range(), *_colorMapColCyn);

    /*    _rightAxis->setColorMap(_spectrogram->data().range(),
     *_colorMap);*/
    //_rightAxis->setColorMap(_spectrogram->data().range(),
    //                        *_colorMapInv);
    _plot->setAxisScale(QwtPlot::yRight,
                        _spectrogram->data().range().minValue()+0.001,
                        _spectrogram->data().range().maxValue() );
    if (hist->key() != oldKey)
    {
      QRectF brect;
      oldKey = hist->key();
      //            brect.setLeft( hist->axis()[cass::HistogramBackend::xAxis].lowerLimit() );
      //            brect.setTop( hist->axis()[cass::HistogramBackend::yAxis].upperLimit() );
      //            brect.setWidth( hist->axis()[cass::HistogramBackend::xAxis].upperLimit() - hist->axis()[cass::HistogramBackend::xAxis].lowerLimit());
      //            brect.setHeight( hist->axis()[cass::HistogramBackend::yAxis].upperLimit()- hist->axis()[cass::HistogramBackend::yAxis].lowerLimit() );
      brect.setCoords(
          hist->axis()[cass::HistogramBackend::xAxis].lowerLimit(),
          hist->axis()[cass::HistogramBackend::yAxis].upperLimit(),
          hist->axis()[cass::HistogramBackend::xAxis].upperLimit(),
          hist->axis()[cass::HistogramBackend::yAxis].lowerLimit());
      VERBOSEOUT(std::cout << "NEW"
          <<" left: "<<brect.left()
          <<" right: "<<brect.right()
          <<" top: "<<brect.top()
          <<" bottom: "<<brect.bottom()
          <<" width: "<<brect.width()
          <<" height: "<<brect.height()
          <<std::endl);
      _zoomer->setZoomBase( brect  );
      _zoomer->zoom(0);
    }
    _plot->replot();
  }


  QImage qimage()
  {
    return const_cast<cass::Histogram2DFloat*>(_spectrogramData->histogram())->qimage();
  }

protected slots:
  void saveColorbar()
  {
    QSettings settings;
    settings.beginGroup("ColorBar");
    if (_colorbarPresets->currentText() == QString("") ) _colorbarPresets->setEditText(tr("default"));
    settings.beginGroup( _colorbarPresets->currentText() );
    settings.setValue("pos1", _cs_top );
    settings.setValue("pos2", _cs_bot );
    settings.setValue("transformCol", _transformCol);
  }

  void Replot()
  {
    /*    _spectrogramData->setHistogram(hist,
                                   _bool_auto_scale->checkState(),
                                   _sbx_scale_min->value(),
                                   _sbx_scale_max->value() );*/


    if(_bool_auto_scale->checkState())
      _plot->setAxisScale(QwtPlot::yRight,
                        _sbx_scale_min->value(),
                        _sbx_scale_max->value() );
    _plot->replot();
  }


  void changeColorIntLin(bool checked) {if (checked) _transformCol = QwtLogColorMap::trans_lin; changeColorInt();}
  void changeColorIntLog(bool checked) {if (checked) _transformCol = QwtLogColorMap::trans_log10; changeColorInt();}
  void changeColorIntExp(bool checked) {if (checked) _transformCol = QwtLogColorMap::trans_pow10; changeColorInt();}
  void changeColorIntSqrt(bool checked) {if (checked) _transformCol = QwtLogColorMap::trans_sqrt; changeColorInt();}
  void changeColorIntSquare(bool checked) {if (checked) _transformCol = QwtLogColorMap::trans_square; changeColorInt();}

  void changeColorInt()
  {
    saveColorbar();
    updateColorBar(_color_scale->value());
  }

  void on_colorbarPreset_changed(const QString& name)
  {
    QSettings settings;
    settings.beginGroup("ColorBar");
    settings.setValue("current", name);
    loadColorbar(name);
  }

  void loadColorbar(QString name)
  {
    QSettings settings;

    settings.beginGroup("ColorBar");
    settings.beginGroup( name );
    _cs_top = settings.value("pos1", 0.7).toDouble();
    _cs_bot = settings.value("pos2", 0.1).toDouble();
    _transformCol = static_cast<QwtLogColorMap::transformId>( settings.value("transformCol", QwtLogColorMap::trans_lin).toInt() );
    switch(_transformCol)
    {
    case QwtLogColorMap::trans_lin: _rad_colormap_lin->setChecked(true);
      break;
    case QwtLogColorMap::trans_log10: _rad_colormap_log->setChecked(true);
      break;
    case QwtLogColorMap::trans_pow10: _rad_colormap_exp->setChecked(true);
      break;
    case QwtLogColorMap::trans_square: _rad_colormap_sq->setChecked(true);
      break;
    case QwtLogColorMap::trans_sqrt: _rad_colormap_sqrt->setChecked(true);
      break;
    }
    updateColorBar(_color_scale->value());
  }

  void updateColorBarScale()
  {
    _plot->setAxisScaleEngine(QwtPlot::yRight, (*_cb_scaleEngineIt)->create() );
    _plot->replot();
  }


  void updateColorBar(int _color_scale)
  {
    // old colormap is deleted by _spectrogram->setColorMap !!
    /*
    delete _colorMap;
    _colorMap = new QwtLogColorMap(QColor(0,0,0), QColor(255,255,255));
    _colorMap->addColorStop(_cs_top, QColor(255,255,255));
    _colorMap->addColorStop(_cs_bot, QColor(0,0,0));
    _colorMap->setTransformId(_transformCol);
    _rightAxis->setColorMap(_spectrogram->data().range(), *_colorMap);
    */
    if(_color_scale==-1)
    {
      delete _colorMap;
      _colorMap = new QwtLogColorMap(QColor(0,0,0), QColor(255,255,255));
      _colorMap->addColorStop(_cs_top, QColor(255,255,255));
      _colorMap->addColorStop(_cs_bot, QColor(0,0,0));
      _colorMap->setTransformId(_transformCol);
      _rightAxis->setColorMap(_spectrogram->data().range(), *_colorMap);
      _spectrogram->setColorMap(*_colorMap);
    }
    else if(_color_scale==0)
    {
      delete _colorMapColMany;
      _colorMapColMany = new QwtLogColorMap(Qt::darkCyan, Qt::red);
      _colorMapColMany->addColorStop(0.1, Qt::cyan);
      _colorMapColMany->addColorStop(0.6, Qt::green);
      _colorMapColMany->addColorStop(0.95, Qt::yellow);
      _colorMapColMany->setTransformId(_transformCol);
      _rightAxis->setColorMap(_spectrogram->data().range(), *_colorMapColMany);
      _spectrogram->setColorMap(*_colorMapColMany);
    }
    else if(_color_scale==1)
    {
      delete _colorMapColRed;
      _colorMapColRed = new QwtLogColorMap(QColor(0,0,0), QColor(255,0,0));
      _colorMapColRed->addColorStop(_cs_top, QColor(180,0,0));
      _colorMapColRed->addColorStop(_cs_bot, QColor(50,0,0));
      _colorMapColRed->addColorStop((_cs_top+_cs_bot)/2., QColor(115,0,0));
      _colorMapColRed->setTransformId(_transformCol);
      _rightAxis->setColorMap(_spectrogram->data().range(), *_colorMapColRed);
      _spectrogram->setColorMap(*_colorMapColRed);
    }
    else if(_color_scale==2)
    {
      delete _colorMapColGreen;
      _colorMapColGreen = new QwtLogColorMap(QColor(0,0,0), QColor(0,255,0));
      _colorMapColGreen->addColorStop(_cs_top, QColor(0,180,0));
      _colorMapColGreen->addColorStop(_cs_bot, QColor(0,50,0));
      _colorMapColGreen->addColorStop((_cs_top+_cs_bot)/2., QColor(0,115,0));
      _colorMapColGreen->setTransformId(_transformCol);
      _rightAxis->setColorMap(_spectrogram->data().range(), *_colorMapColGreen);
      _spectrogram->setColorMap(*_colorMapColGreen);
    }
    else if(_color_scale==3)
    {
      delete _colorMapColBlue;
      _colorMapColBlue = new QwtLogColorMap(QColor(0,0,0), QColor(0,0,255));
      _colorMapColBlue->addColorStop(_cs_top, QColor(0,0,180));
      _colorMapColBlue->addColorStop(_cs_bot, QColor(0,0,50));
      _colorMapColBlue->addColorStop((_cs_top+_cs_bot)/2., QColor(0,0,115));
      _colorMapColBlue->setTransformId(_transformCol);
      _rightAxis->setColorMap(_spectrogram->data().range(), *_colorMapColBlue);
      _spectrogram->setColorMap(*_colorMapColBlue);
    }
    else if(_color_scale==4)
    {
      delete _colorMapColVio;
      _colorMapColVio = new QwtLogColorMap(QColor(0,0,0), QColor(255,0,255));
      _colorMapColVio->addColorStop(_cs_top, QColor(180,0,180));
      _colorMapColVio->addColorStop(_cs_bot, QColor(50,0,50));
      _colorMapColVio->addColorStop((_cs_top+_cs_bot)/2., QColor(115,0,115));
      _colorMapColVio->setTransformId(_transformCol);
      _rightAxis->setColorMap(_spectrogram->data().range(), *_colorMapColVio);
      _spectrogram->setColorMap(*_colorMapColVio);
    }
    else if(_color_scale==5)
    {
      delete _colorMapColCyn;
      _colorMapColCyn = new QwtLogColorMap(QColor(0,0,0), QColor(0,255,255));
      _colorMapColCyn->addColorStop(_cs_top, QColor(0,180,180));
      _colorMapColCyn->addColorStop(_cs_bot, QColor(0,50,50));
      _colorMapColCyn->addColorStop((_cs_top+_cs_bot)/2., QColor(0,115,115));
      _colorMapColCyn->setTransformId(_transformCol);
      _rightAxis->setColorMap(_spectrogram->data().range(), *_colorMapColCyn);
      _spectrogram->setColorMap(*_colorMapColCyn);
    }


    //_spectrogram->setColorMap(*_colorMapInv);

    _plot->replot();
  }

protected:

  QwtLogColorMap::transformId _transformCol, _transformCol_inv;
  QwtLogColorMap* _colorMapInv;
  QwtLogColorMap* _colorMap;

  QwtLogColorMap* _colorMapCol1;
  QwtLogColorMap* _colorMapColMany;
  QwtLogColorMap* _colorMapColBlue;
  QwtLogColorMap* _colorMapColRed;
  QwtLogColorMap* _colorMapColGreen;
  QwtLogColorMap* _colorMapColVio;
  QwtLogColorMap* _colorMapColCyn;

  spectrogramData* _spectrogramData;
  spectrogramDataDummy* _spectrogramDataDummy;
  QwtPlotSpectrogram* _spectrogram;
  MyPlot* _plot;
  QVBoxLayout _layout;
  QwtScaleWidget * _rightAxis;
  QwtPlotZoomer* _zoomer;

  double _cs_top, _cs_bot;
  QList< createScaleEngine* >* _cb_scaleEngines;
  QList< createScaleEngine* >::iterator _cb_scaleEngineIt;

  QToolBar* _toolbar;
  QComboBox* _colorbarPresets;
  QPushButton* _saveColorbar;

  QRadioButton* _rad_colormap_log;
  QRadioButton* _rad_colormap_lin;
  QRadioButton* _rad_colormap_exp;
  QRadioButton* _rad_colormap_sqrt;
  QRadioButton* _rad_colormap_sq;

  QCheckBox* _bool_auto_scale;
  QDoubleSpinBox* _sbx_scale_min;
  QDoubleSpinBox* _sbx_scale_max;
  QSpinBox* _color_scale;
};



/** 1d plot
 *
 * @author Stephan Kassemeyer
 * @author Nicola Coppola
 * @todo 1d hist, log of x axis
 * @todo scale overlay data, change color and thickness of overlay line
 * @todo zoom into x axis but let y axis scale automatically to highest value
 * @todo document this class
 */
class plotWidget : public QWidget
{
  Q_OBJECT
public:
  void addOverlay(cass::Histogram1DFloat* hist, QString name)
  {
    NaNCurve* overlayCurve = new NaNCurve(name);
    overlayCurve->setPen( QPen(QColor::fromHsv(qrand() % 256, 255, 190)) );
    //overlayCurve->setPen( QPen(Qt::red));
    _overlayCurves.append( overlayCurve );


    QVector<double> qdata(hist->size());
    QVector<double> qx(hist->size());
    const cass::AxisProperty &axis = hist->axis()[0];
    for (size_t ii=0;ii<hist->size();ii++)
    {
      qx[ii]=static_cast<double>(axis.position(ii));
      qdata[ii]=static_cast<double>(hist->bin(ii));
    }
    overlayCurve->attach(&_plot);
    overlayCurve->setData(static_cast<QwtArray<double> >(qx), static_cast<QwtArray<double> >(qdata));
    QWidget* curveWidget = _legend->find( overlayCurve );
    _overlayCurveWidgets.append(curveWidget);
    curveWidget->installEventFilter(this);
  }

  virtual void redraw() {std::cout << "base redraw" << std::endl;}

  void setHistogramKey(std::string str)
  {
    _histogramKey = str;
    redraw();
  }

  void setData(cass::Histogram1DFloat* hist )
  {
    static std::string oldKey("");
    //QVector<cass::HistogramFloatBase::value_t> &qdata = QVector::fromStdVector ( data.memory() );
    //QVector<cass::HistogramFloatBase::value_t> qdata(hist.size());
    QVector<double> qdata(hist->size());
    //QVector<cass::HistogramFloatBase::value_t> qx(hist.size());
    QVector<double> qx(hist->size());
    const cass::AxisProperty &axis = hist->axis()[0];
    for (size_t ii=0;ii<hist->size();ii++) {
      qx[ii]=static_cast<double>(axis.position(ii));
      if(!_linyaxis)
        qdata[ii]=static_cast<double>(hist->bin(ii))+1.e-6;
      else
        qdata[ii]=static_cast<double>(hist->bin(ii));
    }
    _curve.attach(&_plot);
    _curve.setData(static_cast<QwtArray<double> >(qx), static_cast<QwtArray<double> >(qdata));

    dynamic_cast<TrackZoomer1D*>(_zoomer)->setHistogram(hist);

    _baseRect.setTop( hist->max() );
    _baseRect.setBottom( hist->min() );
    if(!_bool_auto_scale1d->checkState())
    {
      if (_linyaxis) _plot.setAxisScale(QwtPlot::yLeft,hist->min(),hist->max(),0.);
      else
        _plot.setAxisScale(QwtPlot::yLeft,hist->max()/1.e5,hist->max(),0.);
    }
    else
    {
      _plot.setAxisScale(QwtPlot::yLeft,_sbx_scale1d_min->value(),_sbx_scale1d_max->value(),0.);
    }

    if(_sbx_scale1d_max->value() < _sbx_scale1d_min->value())
    {
      _sbx_scale1d_min->setStyleSheet("QDoubleSpinBox {color: blue; background-color: #FF0000}");
      _sbx_scale1d_max->setStyleSheet("QDoubleSpinBox {color: blue; background-color: #FF0000}");
    }
    else
    {
      _sbx_scale1d_min->setStyleSheet("QDoubleSpinBox {color: black; background-color: #FFFFFF}");
      _sbx_scale1d_max->setStyleSheet("QDoubleSpinBox {color: black; background-color: #FFFFFF}");
    }

    if (hist->key() != oldKey)
    {
      oldKey = hist->key();
      _baseRect.setLeft( axis.position(0) );
      _baseRect.setRight( axis.position(hist->size()) );
      _zoomer->setZoomBase(_baseRect);
      _zoomer->setZoomBase();
      _zoomer->zoom(0);
    }
    _plot.replot();
  };

  void customEvent( QEvent* event)
  {
    if (event->type() == QEvent::User+111)
    {
      EremoveCurve* removeCurveEvent = dynamic_cast<EremoveCurve*>(event);
      _overlayCurves.removeAll(removeCurveEvent->curve);
      _overlayCurveWidgets.removeAll(removeCurveEvent->curveWidget);
      removeCurveEvent->curve->detach();
      _plot.replot();
    }
  }

  bool eventFilter(QObject *obj, QEvent *event)
  {
    if ( _overlayCurveWidgets.contains(dynamic_cast<QWidget*>(obj)) )
    {
      if (event->type() == QEvent::MouseButtonPress)
      {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->button() == Qt::RightButton)
        {
          NaNCurve* curve = dynamic_cast<NaNCurve*>( _legend->find(dynamic_cast<QWidget*>(obj)) );
          if (curve) {
            EremoveCurve* removeCurveEvent = new EremoveCurve( static_cast<QEvent::Type>(QEvent::User+111) );
            removeCurveEvent->curve = curve;
            removeCurveEvent->curveWidget = dynamic_cast<QWidget*>(obj);
            QCoreApplication::postEvent(this, removeCurveEvent);
          }
        }
      }
    }
    // pass the event on to the parent class
    return QWidget::eventFilter(obj, event);
  }

public slots:

  void setHistogramKey(const QString& str)
  {
    setHistogramKey(str.toStdString());
  }

  void Replot()
  {
    if(_bool_auto_scale1d->checkState())
      _plot.setAxisScale(QwtPlot::yLeft,_sbx_scale1d_min->value(),_sbx_scale1d_max->value(),0.);
    _plot.replot();
  }

  void legendChecked(QwtPlotItem* item, bool checked)
  {
    if (!checked) item->show();
    else item->hide();
    _plot.replot();
  }

  void ZoomReset()
  {
    _zoomer->zoom(0);
    _plot.setAxisAutoScale(QwtPlot::xBottom);
    _plot.setAxisAutoScale(QwtPlot::yLeft);
    //    if(_linyaxis) _plot.setAxisAutoScale(QwtPlot::yLeft);
    //else _plot.setAxisScale(QwtPlot::yLeft,hist->max()/1.e5,hist->max(),0.);
    _plot.replot();
    _zoomer->setZoomBase();
  }

  void ZoomY(double factor)
  {
    QStack<QRectF> stack = _zoomer->zoomStack();
#ifdef VERBOSE
    int ii=0;
    for (QStack<QRectF>::iterator it=stack.begin(); it!=stack.end(); ++it)
      std::cout << "zoomstack " << ii++ << ":  left " << it->left() << "right " << it->right() << "top " << it->top() << "bot " << it->bottom() << std::endl;
    std::cout << std::endl;
#endif
    //_zoomer->zoom(1);
    int idx = _zoomer->zoomRectIndex();
    VERBOSEOUT(std::cout << "idx: " << idx << std::endl);
    QRectF newZoomRect = stack[idx];

    // top: lower axis limit
    // bottom: upper axis limit
    double oldheight = newZoomRect.height();
    double newheight = newZoomRect.height() * factor;
    newZoomRect.setHeight( newheight );
    newZoomRect.moveBottom( newZoomRect.bottom() - (newheight-oldheight)/2 );
    newZoomRect = newZoomRect.normalized();

    stack.push( newZoomRect );
    //stack[0] = newZoomRect.unite(stack[0]);
    _zoomer->setZoomStack(stack, -1);
    _plot.replot();
  }

  void ZoomOut()
  {
    ZoomY(2.0);
  }
  void ZoomIn()
  {
    ZoomY(0.5);
  }

  void GridToggle(bool checked)
  {
    _grid->enableX(checked);
    _grid->enableY(checked);
    _plot.replot();
  }

  void YAxisToggle(int checked)
  {
    _linyaxis=checked;
    if((++_cb_scaleEngineIt)==_cb_scaleEngines->end())
      _cb_scaleEngineIt = _cb_scaleEngines->begin();
    if(_linyaxis) _plot.setAxisScaleEngine(QwtPlot::yLeft, (*_cb_scaleEngineIt)->create());
    if(!_linyaxis)  _plot.setAxisScaleEngine(QwtPlot::yLeft, (*_cb_scaleEngineIt)->create());
    _plot.replot();
  }

protected:

  void initToolbar(QLayout& layout)
  {
    _toolbar = new QToolBar(tr("plot toolbar"), this);
    _act_zoomin  = new QAction( QIcon(":images/zoomin.png"), tr("&Zoom in"), this);
    _act_zoomout = new QAction( QIcon(":images/zoomout.png"), tr("Zoom in"), this);
    _act_zoomreset = new QAction( QIcon(":images/zoomreset.png"), tr("Zoom reset"), this);
    _act_gridtoggle = new QAction( QIcon(":images/grid.png"), tr("toggle Grid"), this);
    _act_gridtoggle->setCheckable(true);
    _act_gridtoggle->setChecked(true);
    connect(_act_zoomreset, SIGNAL(triggered()), this, SLOT(ZoomReset()));
    connect(_act_zoomin, SIGNAL(triggered()), this, SLOT(ZoomIn()));
    connect(_act_zoomout, SIGNAL(triggered()), this, SLOT(ZoomOut()));
    connect(_act_gridtoggle, SIGNAL(toggled(bool)), this, SLOT(GridToggle(bool)));

    _toolbar->addAction(_act_zoomin);
    _toolbar->addAction(_act_zoomout);
    _toolbar->addAction(_act_zoomreset);
    _toolbar->addAction(_act_gridtoggle);

    //QLabel* _linlog_yscale = new QLabel(tr("  lin/log y-scale "),this);
    //_toolbar->addWidget(_linlog_yscale);
    //_bool_1d_lin_yscale = new QAction( QIcon(":images/chart-Axis_Features.jpg"), tr("toggle lin/log y-scale"), this);
    //_bool_1d_lin_yscale = new QAction( QIcon(":images/Modelica_Math_log10I.png"), tr("toggle lin/log y-scale"), this);
    //_bool_1d_lin_yscale = new QCheckBox(tr("lin/log y-scale"));
    //_bool_1d_lin_yscale->setCheckable( true );
    //_bool_1d_lin_yscale->setChecked( true );
    //_toolbar->addAction(_bool_1d_lin_yscale);
    //connect(_bool_1d_lin_yscale, SIGNAL(toggled(bool)), this, SLOT(YAxisToggle(bool)));

    _bool_1d_lin_yscale = new QCheckBox(tr("lin/log y-scale"),this);
    _bool_1d_lin_yscale->setChecked( TRUE );
    _toolbar->addWidget(_bool_1d_lin_yscale);

    connect(_bool_1d_lin_yscale, SIGNAL(stateChanged(int)), this, SLOT(YAxisToggle(int)));
    _cb_scaleEngines = new QList< createScaleEngine* >();
    _cb_scaleEngines->append( new createLinearScaleEngine );
    _cb_scaleEngines->append( new createLog10ScaleEngine );
    _cb_scaleEngineIt = _cb_scaleEngines->begin();

    _bool_auto_scale1d = new QCheckBox(tr("man scale"));
    _bool_auto_scale1d->setChecked( FALSE );
    QLabel* _lbl_scale1d_min = new QLabel(tr("Min"),this);
    _sbx_scale1d_min = new QDoubleSpinBox(this);
    _sbx_scale1d_min->setRange(-2.e6,2.e6);
    _sbx_scale1d_min->setValue(1);
    _sbx_scale1d_min->setDecimals(3);

    QLabel* _lbl_scale1d_max = new QLabel(tr("Max"),this);
    _sbx_scale1d_max = new QDoubleSpinBox(this);
    _sbx_scale1d_max->setRange(-2.e9,2.e9);
    _sbx_scale1d_max->setValue(1.e6);
    _sbx_scale1d_max->setDecimals(3);
    _toolbar->addWidget(_bool_auto_scale1d);
    _toolbar->addWidget(_lbl_scale1d_min);
    _toolbar->addWidget(_sbx_scale1d_min);
    _toolbar->addWidget(_lbl_scale1d_max);
    _toolbar->addWidget(_sbx_scale1d_max);
    connect(_sbx_scale1d_max, SIGNAL(valueChanged(double)), this, SLOT(Replot()));
    connect(_sbx_scale1d_min, SIGNAL(valueChanged(double)), this, SLOT(Replot()));

    if(_sbx_scale1d_max->value() < _sbx_scale1d_min->value())
    {
      _sbx_scale1d_min->setStyleSheet("QDoubleSpinBox {color: blue; background-color: #FF0000}");
      _sbx_scale1d_max->setStyleSheet("QDoubleSpinBox {color: blue; background-color: #FF0000}");
    }
    else
    {
      _sbx_scale1d_min->setStyleSheet("QDoubleSpinBox {color: black; background-color: #000000}");
      _sbx_scale1d_max->setStyleSheet("QDoubleSpinBox {color: black; background-color: #000000}");
    }

    layout.addWidget(_toolbar);
  }

  void initPlot(QLayout&  layout)
  {
    layout.addWidget(&_plot);
    _plot.replot();
    _curve.setPen( QPen(Qt::blue) );
    _zoomer = new TrackZoomer1D(_plot.canvas());
    _zoomer->setSelectionFlags( QwtPicker::RectSelection | QwtPicker::DragSelection );
    /*_zoomer = new TrackZoomer1D(QwtPlot::xBottom, QwtPlot::yLeft,
                                     QwtPicker::DragSelection, QwtPicker::AlwaysOff,
                                     _plot.canvas());*/
    _zoomer->setMousePattern(QwtEventPattern::MouseSelect3,
                             Qt::RightButton);
    _zoomer->setMousePattern(QwtEventPattern::MouseSelect2,
                             Qt::RightButton, Qt::ControlModifier);
    _zoomer->setRubberBandPen(QPen(Qt::green));
    _zoomer->setMousePattern(QwtEventPattern::MouseSelect2, Qt::RightButton,
                             Qt::ControlModifier);
    _zoomer->setMousePattern(QwtEventPattern::MouseSelect3, Qt::RightButton);
    // Grid for the plot
    _grid = new QwtPlotGrid;
    _grid->setMajPen(QPen(Qt::black, 0, Qt::DashLine));
    _grid->attach(&_plot);
    // Legend for the plot
    _legend = new QwtLegend;
    _legend->setItemMode(QwtLegend::CheckableItem);
    connect(&_plot, SIGNAL(legendChecked(QwtPlotItem *, bool)),
            this, SLOT(legendChecked(QwtPlotItem *, bool)));
    //_plot.insertLegend(_legend, QwtPlot::ExternalLegend);
    _plot.insertLegend(_legend, QwtPlot::RightLegend);

  }

  QwtPlot _plot;
  QwtPlotZoomer* _zoomer;
  QwtPlotGrid* _grid;
  QwtLegend* _legend;
  QwtDoubleRect _baseRect;
  NaNCurve _curve;
  QList<NaNCurve*> _overlayCurves;
  QList<QWidget*> _overlayCurveWidgets;

  QToolBar* _toolbar;
  QAction* _act_zoomin;
  QAction* _act_zoomout;
  QAction* _act_zoomreset;
  QAction* _act_gridtoggle;

  //QAction* _bool_1d_lin_yscale;
  QCheckBox* _bool_1d_lin_yscale;
  bool _linyaxis;
  QList< createScaleEngine* >* _cb_scaleEngines;
  QList< createScaleEngine* >::iterator _cb_scaleEngineIt;

  QCheckBox* _bool_auto_scale1d;
  QDoubleSpinBox* _sbx_scale1d_min;
  QDoubleSpinBox* _sbx_scale1d_max;


  CASSsoapProxy* _cass;

  std::string _histogramKey;
};





/** plotWidget1D
   * widget that can display 1d histograms.
   * usage:
   *   left mouse drag zooms
   *   right mouse goes back in zoom history
   * @author Stephan Kassemeyer
   */
class plotWidget1D : public plotWidget
{
public:
  //
  plotWidget1D()
  {
    setupUI();
  }

  void setupUI()
  {
    initPlot(_layout);
    initToolbar(_layout);
    setLayout(&_layout);
  }

protected slots:
  //void getHistogram();
  //void getList();

protected:
  QVBoxLayout _layout;

  std::string getHistogram_mime(size_t type);
private:
  //
};






/** plotWidget0D
 * widget that can display 0d histograms.
 * Displays current value and a plot of last accumulationLength values.
 * usage:
 *   left mouse drag zooms
 *   right mouse goes back in zoom history
 * @author Stephan Kassemeyer
 */
class plotWidget0D: public plotWidget
{
public:
  plotWidget0D(int accumulationLength)
    : _accumulationLength(accumulationLength),
    _histAccumulator(accumulationLength, 0, accumulationLength-1)   // hist: nbrXBins, xLow, xUp
  {
    VERBOSEOUT(std::cout << "0d constr" << std::endl);
    setupUI();
  }

  void setupUI()
  {
    initPlot(_layout);
    _lblValue = new QLabel;
    _lblValue->setAlignment(Qt::AlignHCenter);
    QFont font(_lblValue->font());
    font.setPointSize(20);
    _lblValue->setFont(font);
    _layout.addWidget(_lblValue);
    initToolbar(_layout);
    setLayout(&_layout);
  }

  void setData(cass::Histogram0DFloat* hist )
  {
    setValue(hist->getValue());
    redraw();
  }

  void redraw()
  {
    _histAccumulator.clear();
    int ii=0;
    for (QQueue<float>::iterator valueit=_values[_histogramKey].begin(); valueit!=_values[_histogramKey].end(); valueit++)
    {
      _histAccumulator.fill( ii, (*valueit));
      ii++;
    }
    plotWidget::setData(&_histAccumulator);
  }

  void setValue(float val)
  {
    _lblValue->setText( QString::number(val) );
    _values[_histogramKey].enqueue(val);
    if (_values[_histogramKey].size()>_accumulationLength) _values[_histogramKey].dequeue();
  }

protected:

  QLabel* _lblValue;

  std::map<std::string, QQueue<float> > _values;

  int _accumulationLength;

  cass::Histogram1DFloat _histAccumulator;

  QVBoxLayout _layout;
};


#endif


// Local Variables:
// coding: utf-8
// mode: C++
// c-file-style: "gnu"
// c-file-offsets: ((c . 0) (innamespace . 0))
// fill-column: 100
// End:
