//Copyright (C) 2010 Lutz Foucar

/**
 * @file ratemeter.cpp file contains definition of class calculating a rate
 *
 * @author Lutz Foucar
 */

#include <iostream>
#include <cmath>

#include "ratemeter.h"

cass::Ratemeter::Ratemeter(const double averagetime, QObject *parent)
    :QObject(parent),
    _counts(0),
    _rate(0),
    _averagetime(averagetime)
{
  //start the clock//
  _time.start();
}


double cass::Ratemeter::calculateRate()
{
  //how long since the last calculation?//
  const double elapsedtime = _time.elapsed();

  //calc rate//
  _rate = ((1 - std::exp(-elapsedtime/_averagetime))*_counts) +
                std::exp(-elapsedtime/_averagetime)*_rate;
  
  //reset values//
  _counts = 0.;
  _time.restart();
 
  return _rate;
}

void cass::Ratemeter::count()
{
  _counts += 1.;
}
