#ifndef PLOTWIDGET_H
#define PLOTWIDGET_H


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
#include <qdialog.h>
#include <QDockWidget>
#include <QLabel>
#include <QLayout>
#include <QToolBar>
#include <QPushButton>
#include <QComboBox>
#include <QRadioButton>
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


// prototypes:
class cassData;

class createScaleEngine
{
public:
    virtual QwtScaleEngine* create() = 0;
    virtual ~createScaleEngine(){}
};
class createLinearScaleEngine : public createScaleEngine
{
public:
    QwtScaleEngine* create() { return new QwtLinearScaleEngine; };
};

class createLog10ScaleEngine : public createScaleEngine
{
public:
    QwtScaleEngine* create() { return new QwtLog10ScaleEngine; };
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

    void setHistogram(cass::Histogram1DFloat* hist) { _hist = hist; };
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
        try {
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

    void setHistogram(cass::Histogram2DFloat* hist) { _hist = hist; };
protected:
    cass::Histogram2DFloat* _hist;
};


class spectrogramDataDummy: public QwtRasterData
{
public:
    spectrogramDataDummy():
        QwtRasterData(QwtDoubleRect(-1.5, -1.5, 3.0, 3.0))
    {
    }

    virtual QwtRasterData *copy() const
    {
        return new spectrogramDataDummy();
    }
     void setHistogram(cass::Histogram2DFloat* /*hist*/)
    {
    }

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
        std::cout << "spectrogramdata default constructor" << std::endl;
    }

    ~spectrogramData()
    {
        //delete _hist; // don't delete: histogram is owned by caller of setHistogram.
    }

    spectrogramData( cass::Histogram2DFloat* hist, QwtDoubleRect brect, QwtDoubleInterval interval) :
        QwtRasterData(brect), _hist(hist), _boundRect(brect), _interval(interval)
    {
        //setI
        std::cout << "spectrogramdata overloaded constructor" << std::endl;
    }

    void setHistogram(cass::Histogram2DFloat* hist)
    {
        //delete _hist;   // don't delete: spectrogram keeps a shallow copy of spectrogramdata and calls destructor in setData.
        _hist = hist;
        std::cout << "spectrogramdata setHistogram" << std::endl;
        if (_hist) {
            _interval.setMinValue( _hist->min() );
            _interval.setMaxValue( _hist->max() );
            _boundRect.setLeft( _hist->axis()[cass::HistogramBackend::xAxis].lowerLimit() );
            _boundRect.setRight( _hist->axis()[cass::HistogramBackend::yAxis].lowerLimit() );
            _boundRect.setWidth( _hist->axis()[cass::HistogramBackend::xAxis].upperLimit()  );
            _boundRect.setHeight( _hist->axis()[cass::HistogramBackend::yAxis].upperLimit() );
            std::cout << " hist min : " << _hist->min() << " max: " << _hist->max() << std::endl;
        }
        setBoundingRect( _boundRect );
    }

    virtual QwtRasterData *copy() const
    {
        std::cout <<"spectrogramdata::copy()"<<std::endl;
        return new spectrogramData(_hist, _boundRect, _interval);
    }

    virtual QwtDoubleInterval range() const
    {
        std::cout << "spectrogramdata range: " << _interval.minValue() << " " <<_interval.maxValue()  << std::endl;
        return _interval;
    }

    virtual double value(double x, double y) const
    {
        try {
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
   * @author Stephan Kassemeyer
   */
class spectrogramWidget : public QWidget
{
    Q_OBJECT
public:
    bool eventFilter(QObject *obj, QEvent *event)
    {
        if (obj == _rightAxis ) {
            if (event->type() == QEvent::MouseButtonPress) {
                QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
                if (mouseEvent->button() == Qt::RightButton) {
                    if((++_cb_scaleEngineIt)==_cb_scaleEngines->end()) _cb_scaleEngineIt=_cb_scaleEngines->begin();
                    updateColorBarScale();
                }
            }
            if (event->type() == QEvent::MouseMove) {
                QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
                double yval= _plot->invTransform(QwtPlot::yRight, mouseEvent->pos().y());
                QwtDoubleInterval range = _spectrogram->data().range();
                double ystep = (yval - range.minValue() ) / (range.maxValue()-range.minValue());

                if (ystep>0 && ystep<1) {
                    float topdiff = fabs(ystep-_cs_top);
                    float botdiff = fabs(ystep-_cs_bot);
                    if (topdiff<botdiff)
                        _cs_top=ystep;
                    else
                        _cs_bot=ystep;

                    updateColorBar();
                }
            }
        }
        // pass the event on to the parent class
        return QWidget::eventFilter(obj, event);
    }

    void mouseMoveEvent ( QMouseEvent * /* event */) {
        //double yval = _plot->invTransform(QwtPlot::yRight, event->pos().y()) ;

        //std::cout << "scalewidget mousepressevent yval" <<yval << std::endl;
    }

    spectrogramWidget() {
        _cs_top = 0.7;
        _cs_bot = 0.2;
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

        _toolbar->addWidget( _colorbarPresets );
        _toolbar->addWidget( _saveColorbar );
        _toolbar->addWidget( _rad_colormap_lin );
        _toolbar->addWidget( _rad_colormap_log );
        _toolbar->addWidget( _rad_colormap_exp );
        _toolbar->addWidget( _rad_colormap_sqrt );
        _toolbar->addWidget( _rad_colormap_sq );


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


    _rightAxis->setColorMap(_spectrogram->data().range(),
        *_colorMap);

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

    void setData(cass::Histogram2DFloat* hist) {
        static int oldId=cass::PostProcessors::InvalidPP;

        _spectrogram->setData(*_spectrogramDataDummy); //hack
        dynamic_cast<TrackZoomer2D*>(_zoomer)->setHistogram(hist);
        _spectrogramData->setHistogram(hist);
        _spectrogram->setData(*_spectrogramData);   //hack
        _spectrogram->invalidateCache();
        _spectrogram->itemChanged();
        _rightAxis->setColorMap(_spectrogram->data().range(),
            *_colorMap);
        _plot->setAxisScale(QwtPlot::yRight,
            _spectrogram->data().range().minValue()+0.001,
            _spectrogram->data().range().maxValue() );

        if (hist->getId() != oldId) {
            QRectF brect;
            oldId = hist->getId();
            brect.setLeft( hist->axis()[cass::HistogramBackend::xAxis].lowerLimit() );
            brect.setRight( hist->axis()[cass::HistogramBackend::yAxis].lowerLimit() );
            brect.setWidth( hist->axis()[cass::HistogramBackend::xAxis].upperLimit()  );
            brect.setHeight( hist->axis()[cass::HistogramBackend::yAxis].upperLimit() );
            _zoomer->setZoomBase( brect  );
            _zoomer->zoom(0);
        }

        _plot->replot();

    };
protected slots:
    void saveColorbar() {
        QSettings settings;
        settings.beginGroup("ColorBar");
        if (_colorbarPresets->currentText() == QString("") ) _colorbarPresets->setEditText(tr("default"));
        settings.beginGroup( _colorbarPresets->currentText() );
        settings.setValue("pos1", _cs_top );
        settings.setValue("pos2", _cs_bot );
        settings.setValue("transformCol", _transformCol);
    }

    void changeColorIntLin(bool checked) {if (checked) _transformCol = QwtLogColorMap::trans_lin; changeColorInt();};
    void changeColorIntLog(bool checked) {if (checked) _transformCol = QwtLogColorMap::trans_log10; changeColorInt();};
    void changeColorIntExp(bool checked) {if (checked) _transformCol = QwtLogColorMap::trans_pow10; changeColorInt();};
    void changeColorIntSqrt(bool checked) {if (checked) _transformCol = QwtLogColorMap::trans_sqrt; changeColorInt();};
    void changeColorIntSquare(bool checked) {if (checked) _transformCol = QwtLogColorMap::trans_square; changeColorInt();};

    void changeColorInt() {
        saveColorbar();
        updateColorBar();
    }

    void on_colorbarPreset_changed(const QString& name) {
        QSettings settings;
        settings.beginGroup("ColorBar");
        settings.setValue("current", name);
        loadColorbar(name);
    }

    void loadColorbar(QString name) {
        QSettings settings;
        settings.beginGroup("ColorBar");
        settings.beginGroup( name );
        _cs_top = settings.value("pos1", 0.7).toDouble();
        _cs_bot = settings.value("pos2", 0.2).toDouble();
        _transformCol = static_cast<QwtLogColorMap::transformId>( settings.value("transformCol", QwtLogColorMap::trans_lin).toInt() );
        switch(_transformCol) {
            case QwtLogColorMap::trans_lin: _rad_colormap_lin->setChecked(true);
            break;
            case QwtLogColorMap::trans_pow10: _rad_colormap_exp->setChecked(true);
            break;
            case QwtLogColorMap::trans_log10: _rad_colormap_log->setChecked(true);
            break;
            case QwtLogColorMap::trans_square: _rad_colormap_sq->setChecked(true);
            break;
            case QwtLogColorMap::trans_sqrt: _rad_colormap_sqrt->setChecked(true);
            break;
        }
        updateColorBar();
    }

    void updateColorBarScale() {
        _plot->setAxisScaleEngine(QwtPlot::yRight, (*_cb_scaleEngineIt)->create() );
        _plot->replot();
    }


    void updateColorBar() {
        // old colormap is deleted by _spectrogram->setColorMap !!
        _colorMap = new QwtLogColorMap(QColor(0,0,0), QColor(255,255,255));
        _colorMap->addColorStop(_cs_top, QColor(255,255,255));
        _colorMap->addColorStop(_cs_bot, QColor(0,0,0));
        _colorMap->setTransformId(_transformCol);
        _spectrogram->setColorMap(*_colorMap);
        _rightAxis->setColorMap(_spectrogram->data().range(),
            *_colorMap);
        _plot->replot();
    }

protected:

    QwtLogColorMap::transformId _transformCol, _transformCol_inv;
    QwtLogColorMap* _colorMapInv;
    QwtLogColorMap* _colorMap;
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


};


class plotWidget : public QWidget
{
   Q_OBJECT
public:
      void addOverlay(cass::Histogram1DFloat* hist ) {
         QwtPlotCurve* overlayCurve = new QwtPlotCurve;
         overlayCurve->setPen( QPen(QColor::fromHsv(qrand() % 256, 255, 190)) );
         //overlayCurve->setPen( QPen(Qt::red));
         _overlayCurves.append( overlayCurve );
         QVector<double> qdata(hist->size());
         QVector<double> qx(hist->size());
         const cass::AxisProperty &axis = hist->axis()[0];
         for (size_t ii=0;ii<hist->size();ii++) {
            qx[ii]=static_cast<double>(axis.position(ii));
            qdata[ii]=static_cast<double>(hist->bin(ii));
         }
         overlayCurve->attach(&_plot);
         overlayCurve->setData(static_cast<QwtArray<double> >(qx), static_cast<QwtArray<double> >(qdata));
      }

      void setData(cass::Histogram1DFloat* hist ) {
         static int oldId = cass::PostProcessors::InvalidPP;
         //QVector<cass::HistogramFloatBase::value_t> &qdata = QVector::fromStdVector ( data.memory() );
         //QVector<cass::HistogramFloatBase::value_t> qdata(hist.size());
         QVector<double> qdata(hist->size());
         //QVector<cass::HistogramFloatBase::value_t> qx(hist.size());
         QVector<double> qx(hist->size());
         const cass::AxisProperty &axis = hist->axis()[0];
         for (size_t ii=0;ii<hist->size();ii++) {
            qx[ii]=static_cast<double>(axis.position(ii));
            qdata[ii]=static_cast<double>(hist->bin(ii));
         }
         _curve.attach(&_plot);
         _curve.setData(static_cast<QwtArray<double> >(qx), static_cast<QwtArray<double> >(qdata));

         dynamic_cast<TrackZoomer1D*>(_zoomer)->setHistogram(hist);

         if (hist->getId() != oldId) {
             oldId = hist->getId();
             _baseRect.setLeft( axis.position(0) );
             _baseRect.setRight( axis.position(hist->size()) );
             _baseRect.setTop( hist->max() );
             _baseRect.setBottom( hist->min() );
             _zoomer->setZoomBase(_baseRect);
             _zoomer->setZoomBase();
             _zoomer->zoom(0);
         }
         _plot.replot();
      };

public slots:

      void legendChecked(QwtPlotItem* item, bool checked) {
          if (!checked) item->show();
          else item->hide();
          _plot.replot();
      }


      void ZoomReset() {
          _zoomer->zoom(0);
          _plot.setAxisAutoScale(QwtPlot::xBottom);
          _plot.setAxisAutoScale(QwtPlot::yLeft);
          _plot.replot();
          _zoomer->setZoomBase();
      }

      void ZoomY(double factor) {
          QStack<QRectF> stack = _zoomer->zoomStack();
          int ii=0;
          for (QStack<QRectF>::iterator it=stack.begin(); it!=stack.end(); ++it)
              std::cout << "zoomstack " << ii++ << ":  left " << it->left() << "right " << it->right() << "top " << it->top() << "bot " << it->bottom() << std::endl;
          std::cout << std::endl;
          //_zoomer->zoom(1);
          int idx = _zoomer->zoomRectIndex();
          std::cout << "idx: " << idx << std::endl;
          QRectF newZoomRect = stack[idx];

          double newheight = newZoomRect.height() * factor;
          newZoomRect.setHeight( newheight );
          newZoomRect.moveBottom( newheight/2 );
          newZoomRect = newZoomRect.normalized();

          stack.push( newZoomRect );
          //stack[0] = newZoomRect.unite(stack[0]);
          _zoomer->setZoomStack(stack, -1);
          _plot.replot();
      }

      void ZoomOut() {
          ZoomY(2.0);
      }
      void ZoomIn() {
          ZoomY(0.5);
      }

      void GridToggle(bool checked) {
          _grid->enableX(checked);
          _grid->enableY(checked);
          _plot.replot();
      }

protected:

      void initToolbar(QLayout& layout) {
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
         layout.addWidget(_toolbar);
      };

    void initPlot(QLayout&  layout) {
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


      };

      QwtPlot _plot;
      QwtPlotZoomer* _zoomer;
      QwtPlotGrid* _grid;
      QwtLegend* _legend;
      QwtDoubleRect _baseRect;
      QwtPlotCurve _curve;
      QList<QwtPlotCurve*> _overlayCurves;

      QToolBar* _toolbar;
      QAction* _act_zoomin;
      QAction* _act_zoomout;
      QAction* _act_zoomreset;
      QAction* _act_gridtoggle;

      CASSsoapProxy* _cass;
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
      plotWidget1D() {
          setupUI();
      };

      void setupUI() {
         initPlot(_layout);
         initToolbar(_layout);
         setLayout(&_layout);
      };

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
    plotWidget0D(int accumulationLength) : _accumulationLength(accumulationLength),
                                           _histAccumulator(accumulationLength, 0, accumulationLength-1)   // hist: nbrXBins, xLow, xUp
    {std::cout << "0d constr" << std::endl;
        setupUI();
    };

    void setupUI() {
        initPlot(_layout);
        _lblValue = new QLabel;
        _lblValue->setAlignment(Qt::AlignHCenter);
        QFont font(_lblValue->font());
        font.setPointSize(20);
        _lblValue->setFont(font);
        _layout.addWidget(_lblValue);
        initToolbar(_layout);
        setLayout(&_layout);
    };

    void setData(cass::Histogram0DFloat* hist ) {
        setValue(hist->getValue());
        _histAccumulator.clear();
        int ii=0;
        for (QQueue<float>::iterator valueit=_values.begin(); valueit!=_values.end(); valueit++) {
            _histAccumulator.fill( ii, (*valueit));
            ii++;
        }
        plotWidget::setData(&_histAccumulator);
    };
    void setValue(float val) {
        _lblValue->setText( QString::number(val) );
        _values.enqueue(val);
        if (_values.size()>_accumulationLength) _values.dequeue();
    };
protected:
    QLabel* _lblValue;
    QQueue<float> _values;
    int _accumulationLength;
    cass::Histogram1DFloat _histAccumulator;

    QVBoxLayout _layout;
};


#endif
