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
        QwtPlotZoomer(canvas)
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
        text.setBackgroundBrush( QBrush( bg ));
        return text;
    }
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
        _boundRect(0.0, 0.0, 100.0, 100.0), _hist(NULL),  QwtRasterData(QwtDoubleRect(0.0, 0.0, 1024, 1024.0)), _interval(QwtDoubleInterval(0.0, 16383.0))
    {
        //setBoundingRect(_boundRect);
    }

    ~spectrogramData()
    {
        delete _hist;
    }

    spectrogramData( cass::Histogram2DFloat* hist, QwtDoubleRect brect, QwtDoubleInterval interval) :
        _hist(hist), _boundRect(brect), _interval(interval), QwtRasterData(QwtDoubleRect(0.0, 20.0, 1000, 2000.0))
    {
        //setBoundingRect(_boundRect);
    }
    
    void setHistogram(cass::Histogram2DFloat* hist)
    {
        //delete _hist;   // don't delete: spectrogram keeps a shallow copy of spectrogramdata and calls destructor in setData.
        _hist = hist;
        if (_hist) {
            _interval.setMinValue( _hist->min() );
            _interval.setMaxValue( _hist->max() );
            std::pair<size_t,size_t> shape(_hist->shape());
            _boundRect.setLeft( 0.0 );
            _boundRect.setRight( 0.0 );
            _boundRect.setTop( shape.second );
            _boundRect.setBottom( shape.first );
            std::cout << " hist 2 d shape.first: " << shape.first << " second: " << shape.second << std::endl;
            std::cout << " hist min : " << _hist->min() << " max: " << _hist->max() << std::endl;
        }
        //setBoundingRect( _boundRect );
    }

    virtual QwtRasterData *copy() const
    {
        return new spectrogramData(_hist, _boundRect, _interval);
    }

    virtual QwtDoubleInterval range() const
    {
        return _interval;
    }

    virtual double value(double x, double y) const
    {
        if (_hist) if (x>1) if (x<1024) if (y>1) if (y<1024) return (*_hist)(x,y);
        return 0.0;
    }
protected:
    cass::Histogram2DFloat* _hist;
    QwtDoubleInterval _interval;
    QwtDoubleRect _boundRect;
};

class spectrogramWidget : public QWidget
{
    Q_OBJECT
public:
    spectrogramWidget() {
        _spectrogramData = new spectrogramData;
        _spectrogramDataDummy = new spectrogramDataDummy;
        _spectrogram = new QwtPlotSpectrogram();
        _plot = new QwtPlot;

        QwtLogColorMap colorMap(Qt::darkCyan, Qt::red);
        colorMap.addColorStop(0.1, Qt::cyan);
        colorMap.addColorStop(0.6, Qt::green);
        colorMap.addColorStop(0.95, Qt::yellow);
        _spectrogram->setColorMap(colorMap);

        _spectrogram->setData(*_spectrogramData);
        _spectrogram->attach(_plot);

    // A color bar on the right axis
    QwtScaleWidget *rightAxis = _plot->axisWidget(QwtPlot::yRight);
    rightAxis->setTitle("Intensity");
    rightAxis->setColorBarEnabled(true);
    rightAxis->setColorMap(_spectrogram->data().range(),
        _spectrogram->colorMap());

    _plot->setAxisScale(QwtPlot::yRight,
        _spectrogram->data().range().minValue(),
        _spectrogram->data().range().maxValue() );
    _plot->enableAxis(QwtPlot::yRight);
    

        QwtPlotZoomer* zoomer = new MyZoomer(_plot->canvas());
        zoomer->setMousePattern(QwtEventPattern::MouseSelect2,
           Qt::RightButton, Qt::ControlModifier);
        zoomer->setMousePattern(QwtEventPattern::MouseSelect3,
           Qt::RightButton);
        _layout.addWidget(_plot);
        setLayout(&_layout);
        _spectrogram->setDisplayMode(QwtPlotSpectrogram::ContourMode, true);
        _plot->plotLayout()->setAlignCanvasToScales(true);
        _plot->replot();
        
    }
    
    void setData(cass::Histogram2DFloat* hist) {

        _spectrogram->setData(*_spectrogramDataDummy); //hack
        _spectrogramData->setHistogram(hist);    //QwtLinearColorMap colorMap(Qt::darkCyan, Qt::red);
        _spectrogram->setData(*_spectrogramData);   //hack
        _spectrogram->invalidateCache();
        _spectrogram->itemChanged();

        _plot->replot();
        
    };
protected:
    spectrogramData* _spectrogramData;
    spectrogramDataDummy* _spectrogramDataDummy;
    QwtPlotSpectrogram* _spectrogram;
    QwtPlot* _plot;
    QVBoxLayout _layout;
    
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
