#ifndef PLOTWIDGET_H 
#define PLOTWIDGET_H


#include <string>
#include <iostream>

#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_zoomer.h>
#include <qdialog.h>
#include <QDockWidget>
#include <QLabel>
#include <QLayout>
#include <QToolBar>
#include <QAction>
#include <QFont>
#include <QQueue>

#include "../cass/cass_event.h"
#include "../cass/serializer.h"
#include "../cass/cass.h"
#include "../cass/histogram.h"
#include "../cass/postprocessing/postprocessor.h"
#include "soapCASSsoapProxy.h"


// prototypes:
class cassData;


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
