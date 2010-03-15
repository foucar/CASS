//Copyright (C) 2010 lmf

#ifndef _CASS_RATEMETER_H_
#define _CASS_RATEMETER_H_

#include <vector>

#include <QtCore/QObject>
#include <QtCore/QTime>

#include "cass.h"

namespace cass
{
  class CASSSHARED_EXPORT Ratemeter : public QObject
  {
    Q_OBJECT;

  public:
    Ratemeter(QObject *parent=0);
    ~Ratemeter();

    double calculateRate();

  public slots:
    void count();

  private:
    QTime               _time;
    std::vector<double> _counter;
    std::vector<double> _times;
    size_t              _idx;
  };
}

#endif // RATEMETER_H
