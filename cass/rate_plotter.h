//Copyright (C) 2010 lmf

#ifndef _RATE_PLOTTER_H_
#define _RATE_PLOTTER_H_

#include <vector>

#include <QtCore/QObject>
#include <QtCore/QTimer>

#include "cass.h"

namespace cass
{
  class Ratemeter;
  //class that will plot the rates calculated in the given ratemeters
  class CASSSHARED_EXPORT RatePlotter : public QObject
  {
    Q_OBJECT;

  public:
    RatePlotter(Ratemeter &inputrate,Ratemeter &analyzerate, QObject *parent=0);
    ~RatePlotter();

  private slots:
    void plot();

  private:
    QTimer     _timer;        //the timer that tells it to plot the rate
    Ratemeter &_inputrate;    //reference to the input rate
    Ratemeter &_analyzerate;  //reference to the analyze rate
  };
}

#endif // RATEMETER_H
