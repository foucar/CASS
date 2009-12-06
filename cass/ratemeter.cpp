#include <iostream>

#include "ratemeter.h"

cass::Ratemeter::Ratemeter()
    :QObject(),
    _counter(4,0),
    _times(4,0),
    _idx(0)
{
  //create a timer that will call this at given interval//
  _timer = new QTimer();
  connect(_timer, SIGNAL(timeout()), this, SLOT(calculateRate()));
  _timer->start(500);

  //create a clock that will time how long it took//
  _time = new QTime();
  _time->start();
}

cass::Ratemeter::~Ratemeter()
{
  //stop timer and delete it//
  _timer->stop();
  delete _timer;
  delete _time;
}

void cass::Ratemeter::calculateRate()
{
  //remember the time//
  _times[_idx%4] = _time->elapsed();
  //calculate the complete times and counts//
  double time = 0;
  double counts = 0;
  for (size_t i=0; i<_times.size() ;++i)
  {
    time   += _times[i];
    counts += _counter[i];
  }
  //now calculate and emit the rate
  double r = counts / (time*1e3);
  emit rate(r);
  //restart the timer and advance the index//
  _time->restart();
  ++_idx;
}

void cass::Ratemeter::count()
{
  _counter[_idx%4] += 1.;
}
