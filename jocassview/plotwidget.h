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

#include "../cass/cass_event.h"
#include "../cass/serializer.h"
#include "../cass/cass.h"
#include "../cass/histogram.h"
#include "../cass/postprocessing/postprocessor.h"
#include "soapCASSsoapProxy.h"


// prototypes:
class cassData;

//class plotWidget : public QDockWidget
class plotWidget0D: public QWidget
{
public:
    plotWidget0D() {
        _lblValue = new QLabel;
        _lblValue->setAlignment(Qt::AlignHCenter);
        QFont font(_lblValue->font());
        font.setPointSize(20);
        _lblValue->setFont(font);
        _layout.addWidget(_lblValue);
        setLayout(&_layout);
    };
    void setData(cass::Histogram0DFloat* hist ) {
        setValue(hist->getValue());
    };
    void setValue(float val) {
        _lblValue->setText( QString::number(val) );
    };
private:
    QLabel* _lblValue;
    QVBoxLayout _layout;
};

class plotWidget : public QWidget
{
   Q_OBJECT
public:
      //
      plotWidget(CASSsoapProxy* cass=NULL) : _cass(cass) {
         _layout.addWidget(&_plot);
         setLayout(&_layout);
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
protected slots:
      //void getHistogram();
      //void getList();

protected:
      QwtPlot _plot;
      QwtPlotZoomer* _zoomer;
      QwtDoubleRect _baseRect;
      CASSsoapProxy* _cass;
      QVBoxLayout _layout;
      QwtPlotCurve _curve;
      std::string getHistogram_mime(size_t type);
private:
      //
};

#endif
