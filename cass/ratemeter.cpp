//Copyright (C) 2010 Lutz Foucar

/**
 * @file ratemeter.cpp file contains definition of class calculating a rate
 *
 * @author Lutz Foucar
 */

#include <iostream>
#include <cmath>

#include "ratemeter.h"

using namespace cass;
using namespace std;

Ratemeter::Ratemeter(const double averagetime)
  : _counts(0),
    _rate(0),
    _averagetime(averagetime)
{
  //start the clock//
  _time.start();
}

double Ratemeter::calculateRate()
{
  //how long since the last calculation?//
  const double elapsedtime = _time.elapsed();

  //calc rate//
  _rate = ((1 - exp(-elapsedtime/_averagetime))*_counts) +
                exp(-elapsedtime/_averagetime)*_rate;
  
  //reset values//
  _counts = 0.;
  _time.restart();
 
  return _rate;
}

void Ratemeter::count(double increase)
{
  QMutexLocker lock(&_mutex);
  _counts += increase;
}
