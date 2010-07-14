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
  const double time = _time.elapsed();

  //use formular found in wiki at
  //http://en.wikipedia.org/wiki/Moving_average//
  //in this the running average time is 2s//
  _rate = (1 - std::exp(-time/_averagetime))*_counts + 
               std::exp(-time/_averagetime)*_rate;
  
  //reset the curent counter before adding//
  _counts = 0.;
  //restart the timer and advance the index//
  _time.restart();
 
  //return the calculated rate//
  return _rate;
}

void cass::Ratemeter::count()
{
  _counts += 1.;
}
