//Copyright (C) 2010 Lutz Foucar

/**
 * @file ratemeter.h file contains declaration of class calculating a rate
 *
 * @author Lutz Foucar
 */

#ifndef _CASS_RATEMETER_H_
#define _CASS_RATEMETER_H_

#include <vector>

#include <QtCore/QTime>
#include <QtCore/QMutex>

#include "cass.h"

namespace cass
{
/** class calculating a rate in Hz.
 *
 * slot count has to be called to increase the count
 * the current rate will be updated once the rate is retrieved
 *
 * @author Lutz Foucar
 */
class Ratemeter
{
public:
  /** constuctor
   *
   * @param averagetime time in seconds over which you want to average the
   *        rate. Default is 1 s.
   */
  Ratemeter(const double averagetime=1);

  /** retrieve the rate
   *
   * the rate is calculated using the formular found in wiki at
   * @see http://en.wikipedia.org/wiki/Moving_average
   * It uses the elapsed time since the last time we called calculateRate() and
   * the current value is the number of counts that we acquired since the last
   * time.
   */
  double calculateRate();

  /** increase the counts
   *
   * this function is locked by a mutex to make it reentrant
   *
   * @param increase the value to increase by. Default is 1.
   */
  void count(double increase=1.);

private:
  /** mutex to make function counting reentrant */
  QMutex _mutex;

  /** the time to stop the timeinterval */
  QTime _time;

  /** a counter that will increase with each call to count*/
  double _counts;

  /** the current rate */
  double _rate;

  /** time constant with which the rate will decrease */
  const double  _averagetime;
};
} //indenation

#endif // RATEMETER_H
