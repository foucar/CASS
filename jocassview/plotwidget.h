#ifndef PLOTWIDGET_H 
#define PLOTWIDGET_H


#include <string>
#include <iostream>

#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_zoomer.h>
#include <qdialog.h>
#include <QDockWidget>
#include <QLayout>
#include <QComboBox>
#include <QPushButton>

#include "../cass/cass_event.h"
#include "../cass/serializer.h"
#include "../cass/cass.h"
#include "../cass/histogram.h"
#include "../cass/postprocessing/postprocessor.h"
#include "soapCASSsoapProxy.h"


// prototypes:
class cassData;

//class plotWidget : public QDockWidget
class plotWidget : public QWidget
{
   Q_OBJECT
public:
      //
      plotWidget(CASSsoapProxy* cass=NULL) : _cass(cass), _btn_getHist(tr("get Histogram")) {
         _layout.addWidget(&_plot);
         setLayout(&_layout);
         _plot.replot();
	 _curve.setPen( QPen(Qt::blue) );
      };
      void setData(cass::Histogram1DFloat* hist ) {
         //QVector<cass::HistogramFloatBase::value_t> &qdata = QVector::fromStdVector ( data.memory() );
         //QVector<cass::HistogramFloatBase::value_t> qdata(hist.size());
         QVector<double> qdata(hist->size());
         //QVector<cass::HistogramFloatBase::value_t> qx(hist.size());
         QVector<double> qx(hist->size());
         const cass::AxisProperty &axis = hist->axis()[0];
         for (int ii=0;ii<hist->size();ii++) {
            qx[ii]=static_cast<double>(axis.position(ii));
            qdata[ii]=static_cast<double>(hist->bin(ii));
         }
         _curve.attach(&_plot);
         _curve.setData(static_cast<QwtArray<double> >(qx), static_cast<QwtArray<double> >(qdata));
         _plot.replot();
      };
protected slots:
      //void getHistogram();
      //void getList();

protected:
      QwtPlot _plot;
      CASSsoapProxy* _cass;
      QVBoxLayout _layout;
      QPushButton _btn_getHist;
      QwtPlotCurve _curve;
      std::string getHistogram_mime(size_t type);
private:
      //
};

#endif
