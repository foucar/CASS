//Copyright (C) 2010 Lutz Foucar

/**
 * @file ratemeter.h file contains declaration of class calculating a rate
 *
 * @author Lutz Foucar
 */

#ifndef _CASS_RATEMETER_H_
#define _CASS_RATEMETER_H_

#include <vector>

#include <QtCore/QObject>
#include <QtCore/QTime>

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
  class CASSSHARED_EXPORT Ratemeter : public QObject
  {
    Q_OBJECT;

  public:
    /** constuctor
     *
     * @param averagetime time in seconds over which you want to average the
     *        rate. Default is 1 s.
     * @param parent the qt parent of this object
     */
    Ratemeter(const double averagetime=1, QObject *parent=0);

    /** retrieve the rate
     *
     * the rate is calculated using the formular found in wiki at
     * @see http://en.wikipedia.org/wiki/Moving_average
     * It uses the elapsed time since the last time we called calculateRate() and
     * the current value is the number of counts that we acquired since the last
     * time.
     */
    double calculateRate();

  public slots:
    /** increase the counts */
    void count();

  private:
    QTime         _time;        //!< the time to stop the timeinterval
    double        _counts;      //!< a counter that will increase with each call to count
    double        _rate;        //!< the current rate
    const double  _averagetime; //!< time constant with which the rate will decrease
  };
}

#endif // RATEMETER_H
