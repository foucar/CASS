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
  class CASSSHARED_EXPORT RatePlotter : public QObject
  {
    Q_OBJECT;

  public:
    RatePlotter(Ratemeter &inputrate,Ratemeter &analyzerate, QObject *parent=0);
    ~RatePlotter();

  private slots:
    void plot();

  private:
    QTimer     _timer;
    Ratemeter &_inputrate;
    Ratemeter &_analyzerate;
  };
}

#endif // RATEMETER_H
