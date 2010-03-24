//Copyright (C) 2010 lmf

#ifndef _CASS_RATEMETER_H_
#define _CASS_RATEMETER_H_

#include <vector>

#include <QtCore/QObject>
#include <QtCore/QTime>

#include "cass.h"

namespace cass
{
  //class calculating a rate in Hz//
  //slot count has to be called to increase the count//
  //the current rate will be updated once the rate is retrieved//
  class CASSSHARED_EXPORT Ratemeter : public QObject
  {
    Q_OBJECT;

  public:
    Ratemeter(const double averagetime=1, QObject *parent=0);
    ~Ratemeter();

    //retrieve the rate//
    double calculateRate();

  public slots:
    //increase the counts
    void count();

  private:
    QTime         _time;        //the time to stop the timeinterval
    double        _counts;      //a counter that will increase with each call to count
    double        _rate;        //the current rate
    const double  _averagetime; //time constant with which the rate will decrease
  };
}

#endif // RATEMETER_H
