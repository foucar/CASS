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
  time = new QTime();
  time->start();
}

cass::Ratemeter::~Ratemeter()
{
  //stop timer and delete it//
  _timer->stop();
  delete _timer;
  delete _time;
}

void cass::calculateRate()
{
  //remember the time//
  _times[idx%4] = time->elapsed();
  //calculate the complete times and counts//
  double time = 0;
  double counts = 0;
  for (size_t i=0; i<_times.size() ;++i)
  {
    time   += _times[i];
    counts += _counts[i];
  }
  //now calculate and emit the rate
  double r = / static_cast<double>(time->elapsed());
  emit rate(r);
  //restart the timer and advance the index//
  time->restart();
  ++idx;
}

void cass::Ratemeter::count()
{
  counter[idx%4] += counter[idx%4];
}
