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

#include <QtCore/QObject>
#include <QtCore/QTimer>

#include "cass.h"

namespace cass
{
  //forward declarations
  class Ratemeter;

  /** Plotting the rate of input and prcessing threads.
   *
   * class that will plot the rates calculated in the given Ratemeter's
   *
   * @todo this needs to be a thread so that it can plot withouth the signal
   *       slot mechanism
   *
   * @author Lutz Foucar
   */
  class RatePlotter : public QObject
  {
    Q_OBJECT;

  public:
    /** constructor.
     *
     * @param inputrate the ratemeter of the input thread
     * @param analyzerate the ratemeter of the worker threads
     * @param plot flag to tell the ratemeter to output the rates
     * @param parent the qt parent of this object
     */
    RatePlotter(Ratemeter &inputrate,
                Ratemeter &analyzerate,
                bool plot,
                QObject *parent=0);

  private slots:
    /** slot to tell the plotter to plot the current rates*/
    void plot();

  private:
    /** the timer that tells it to plot the rate */
    QTimer _timer;

    /** reference to the input Ratemeter */
    Ratemeter &_inputrate;

    /** reference to the workers (analysis) Ratemeter */
    Ratemeter &_analyzerate;
  };
}

#endif // RATEMETER_H



// Local Variables:
// coding: utf-8
// mode: C++
// c-file-offsets: ((c . 0) (innamespace . 0))
// c-file-style: "Stroustrup"
// fill-column: 100
// End:
