#ifndef PLOTWIDGET_H 
#define PLOTWIDGET_H


#include <string>
#include <iostream>

#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_zoomer.h>
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
#include <QAction>
#include <QFont>
#include <QQueue>
#include "qwt_logcolor_map.h"
#include "../cass/cass_event.h"
#include "../cass/serializer.h"
#include "../cass/cass.h"
#include "../cass/histogram.h"
#include "../cass/postprocessing/postprocessor.h"
#include "soapCASSsoapProxy.h"


// prototypes:
class cassData;

class MyZoomer: public QwtPlotZoomer
{
public:
    MyZoomer(QwtPlotCanvas *canvas):
        QwtPlotZoomer(canvas), _hist(NULL)
    {
        setTrackerMode(AlwaysOn);
    }

    virtual QwtText trackerText(const QwtDoublePoint &pos) const
    {
        QColor bg(Qt::white);
#if QT_VERSION >= 0x040300
        bg.setAlpha(200);
#endif

        QwtText text = QwtPlotZoomer::trackerText(pos);
        QString text_string(text.text());
        if (pos.x()>1 && pos.x()<1023 && pos.y()>1 && pos.y()<1023) // todo: catch out of range exception.
        if (_hist) text_string = text_string + " : " + QString::number( (*_hist)(pos.x(), pos.y()) );
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
    void setHistogram(cass::Histogram2DFloat* hist)
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
        delete _hist;
    }

    spectrogramData( cass::Histogram2DFloat* hist, QwtDoubleRect brect, QwtDoubleInterval interval) :
        QwtRasterData(QwtDoubleRect(0,0,1024,1024)), _hist(hist), _boundRect(brect), _interval(interval)
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
            _boundRect.setRight( _hist->axis()[cass::HistogramBackend::xAxis].upperLimit() );
            _boundRect.setTop( _hist->axis()[cass::HistogramBackend::yAxis].upperLimit()  );
            _boundRect.setBottom( _hist->axis()[cass::HistogramBackend::yAxis].lowerLimit() );
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
        if (_hist) if (x>1) if (x<1023) if (y>1) if (y<1023) return (*_hist)(x,y);  //todo: catch out of range exception.
        return 0.0;
    }
protected:
    cass::Histogram2DFloat* _hist;
    QwtDoubleRect _boundRect;
    QwtDoubleInterval _interval;
};

class spectrogramWidget : public QWidget
{
    Q_OBJECT
public:
    spectrogramWidget() {
        //transformCol = QwtLogColorMap::trans_pow10;
        //transformCol_inv = QwtLogColorMap::trans_log10;
        transformCol = QwtLogColorMap::trans_log10;
        transformCol_inv = QwtLogColorMap::trans_log10;
        _spectrogramData = new spectrogramData;
        _spectrogramDataDummy = new spectrogramDataDummy();
        _spectrogram = new QwtPlotSpectrogram();
        _plot = new QwtPlot;

        _colorMap = new QwtLogColorMap(Qt::darkCyan, Qt::red);
        _colorMap->addColorStop(0.1, Qt::cyan);
        _colorMap->addColorStop(0.6, Qt::green);
        _colorMap->addColorStop(0.95, Qt::yellow);
        _colorMap->setTransformId(transformCol);
        _spectrogram->setColorMap(*_colorMap);

        _spectrogram->setData(*_spectrogramData);
        _spectrogram->attach(_plot);

    // A color bar on the right axis
    _rightAxis = _plot->axisWidget(QwtPlot::yRight);
    _rightAxis->setTitle("Intensity");
    _rightAxis->setColorBarEnabled(true);

        _colorMapInv = new QwtLogColorMap(Qt::darkCyan, Qt::red);
        _colorMapInv->addColorStop(0.1, Qt::cyan);
        _colorMapInv->addColorStop(0.6, Qt::green);
        _colorMapInv->addColorStop(0.95, Qt::yellow);
        _colorMapInv->setTransformId(transformCol_inv);


    _rightAxis->setColorMap(_spectrogram->data().range(),
        *_colorMapInv);

    _plot->setAxisScaleEngine(QwtPlot::yRight, new QwtLog10ScaleEngine);
    _plot->setAxisScale(QwtPlot::yRight,
        _spectrogram->data().range().minValue()+1,
        _spectrogram->data().range().maxValue() );
    _plot->enableAxis(QwtPlot::yRight);
    

        _zoomer = new MyZoomer(_plot->canvas());
        _zoomer->setSelectionFlags( QwtPicker::RectSelection | QwtPicker::DragSelection );
        _zoomer->setMousePattern(QwtEventPattern::MouseSelect2,
           Qt::RightButton, Qt::ControlModifier);
        _zoomer->setMousePattern(QwtEventPattern::MouseSelect3,
           Qt::RightButton);
        _layout.addWidget(_plot);
        setLayout(&_layout);
        _spectrogram->setDisplayMode(QwtPlotSpectrogram::ContourMode, true);
        _plot->plotLayout()->setAlignCanvasToScales(true);
        _plot->replot();
        
    }
    
    void setData(cass::Histogram2DFloat* hist) {

        _spectrogram->setData(*_spectrogramDataDummy); //hack
        dynamic_cast<MyZoomer*>(_zoomer)->setHistogram(hist);
        _spectrogramData->setHistogram(hist);
        _spectrogram->setData(*_spectrogramData);   //hack
        _spectrogram->invalidateCache();
        _spectrogram->itemChanged();
        _rightAxis->setColorMap(_spectrogram->data().range(),
            *_colorMapInv);
        _plot->setAxisScale(QwtPlot::yRight,
            _spectrogram->data().range().minValue()+1,
            _spectrogram->data().range().maxValue() );

        _plot->replot();
        
    };
protected:

    QwtLogColorMap::transformId transformCol, transformCol_inv;
    QwtLogColorMap* _colorMapInv;
    QwtLogColorMap* _colorMap;
    spectrogramData* _spectrogramData;
    spectrogramDataDummy* _spectrogramDataDummy;
    QwtPlotSpectrogram* _spectrogram;
    QwtPlot* _plot;
    QVBoxLayout _layout;
    QwtScaleWidget * _rightAxis;
    QwtPlotZoomer* _zoomer;
    
};


class plotWidget : public QWidget
{
   Q_OBJECT
public:
      void setData(cass::Histogram1DFloat* hist ) {
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
         _baseRect.setLeft( axis.position(0) );
         _baseRect.setRight( axis.position(hist->size()) );
         _baseRect.setTop( hist->max() );
         _baseRect.setBottom( hist->min() );
         _zoomer->setZoomBase(_baseRect);
         _plot.replot();
      };

protected:

      void initToolbar(QLayout& layout) {
         _toolbar = new QToolBar(tr("plot toolbar"), this);
         _act_zoomin  = new QAction( QIcon(":images/zoomin.png"), tr("&Zoom in"), this);
         _act_zoomout = new QAction( QIcon(":images/zoomout.png"), tr("&Zoom in"), this);
         _act_zoomin->setCheckable(true);
         _act_zoomout->setCheckable(true);
         _toolbar->addAction(_act_zoomin);
         _toolbar->addAction(_act_zoomout);
         layout.addWidget(_toolbar);
      };

    void initPlot(QLayout&  layout) {
         layout.addWidget(&_plot);
         _plot.replot();
         _curve.setPen( QPen(Qt::blue) );
         _zoomer = new QwtPlotZoomer(QwtPlot::xBottom, QwtPlot::yLeft,
                                     QwtPicker::DragSelection, QwtPicker::AlwaysOff,
                                     _plot.canvas());
         _zoomer->setMousePattern(QwtEventPattern::MouseSelect3,
                                  Qt::RightButton);
         _zoomer->setMousePattern(QwtEventPattern::MouseSelect2,
                                  Qt::RightButton, Qt::ControlModifier);
         _zoomer->setRubberBandPen(QPen(Qt::green));
         _zoomer->setMousePattern(QwtEventPattern::MouseSelect2, Qt::RightButton,
                                  Qt::ControlModifier);
         _zoomer->setMousePattern(QwtEventPattern::MouseSelect3, Qt::RightButton);
      };

      QwtPlot _plot;
      QwtPlotZoomer* _zoomer;
      QwtDoubleRect _baseRect;
      QwtPlotCurve _curve;

      QToolBar* _toolbar;
      QAction* _act_zoomin;
      QAction* _act_zoomout;

      CASSsoapProxy* _cass;
};

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
