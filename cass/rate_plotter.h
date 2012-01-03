//Copyright (C) 2010 Lutz Foucar

/**
 * @file rate_plotter.h file contains declaration of class to plot the rate
 *                      calculated by ratemeters
 *
 * @author Lutz Foucar
 */

#ifndef _RATE_PLOTTER_H_
#define _RATE_PLOTTER_H_

#include <vector>
#include <tr1/memory>

#include <QtCore/QThread>

#include "cass.h"

namespace cass
{
//forward declarations
class Ratemeter;

/** Plotting the rate of input and prcessing threads.
 *
 * class that will plot the rates calculated in the given Ratemeter's
 *
 * @author Lutz Foucar
 */
class RatePlotter : public QThread
{
public:
  /** a shared pointer of this type */
  typedef std::tr1::shared_ptr<RatePlotter> shared_pointer;

  /** constructor.
   *
   * @param inputrate the ratemeter of the input thread
   * @param analyzerate the ratemeter of the worker threads
   * @param updateInterval the interval in which the rate will be plottet.
   *        Default is 1
   * @param parent the qt parent of this object
   */
  RatePlotter(Ratemeter &inputrate,
              Ratemeter &analyzerate,
              int updateInterval=1,
              QObject *parent=0);

  /** destructor
   *
   * checks whether thread is still running in which case it will be terminated.
   * Then waits until thread has finished.
   */
  virtual ~RatePlotter();

protected:
  /** the plotting loop
   *
   * sleep for interval time and then retrieve the rate from the ratemeters
   * and plot it.
   */
  void run();

private:
  /** reference to the input Ratemeter */
  Ratemeter &_inputrate;

  /** reference to the workers (analysis) Ratemeter */
  Ratemeter &_analyzerate;

  /** the interval in which the rate is plottet in s */
  int _interval;
};
}//end namespace cass

#endif // RATEMETER_H



// Local Variables:
// coding: utf-8
// mode: C++
// c-file-offsets: ((c . 0) (innamespace . 0))
// c-file-style: "Stroustrup"
// fill-column: 100
// End:
