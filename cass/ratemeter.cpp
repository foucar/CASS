//Copyright (C) 2010 lmf
#include <iostream>

#include "ratemeter.h"

cass::Ratemeter::Ratemeter(QObject *parent)
    :QObject(parent),
    _counter(4,0),
    _times(4,0),
    _idx(0)
{
  //start the clock//
  _time.start();
}

cass::Ratemeter::~Ratemeter()
{
  //stop timer and delete it//
  _timer->stop();
  delete _time;
}

double cass::Ratemeter::calculateRate()
{
  //remember the time//
  _times[_idx%4] = _time.elapsed();
  //calculate the complete times and counts//
  double time = 0;
  double counts = 0;
  for (size_t i=0; i<_times.size() ;++i)
  {
    time   += _times[i];
    counts += _counter[i];
  }
   
  //now calculate and emit the rate
  double r = counts / (time*1e-3);
  //restart the timer and advance the index//
  _time.restart();
  ++_idx;
  
  //reset the curent counter before adding//
  _counter[_idx%4] = 0.;

  //return the calculated rate//
  return r;
}

void cass::Ratemeter::count()
{
  _counter[_idx%4] += 1.;
}
