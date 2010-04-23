//Copyright (C) 2010 Lutz Foucar

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
   * class that will plot the rates calculated in the given Ratemeter's
   * @author Lutz Foucar
   */
  class CASSSHARED_EXPORT RatePlotter : public QObject
  {
    Q_OBJECT;

  public:
    /** constructor.
     * @param inputrate the ratemeter of the input thread
     * @param analyzerate the ratemeter of the worker threads
     * @param parent the qt parent of this object
     */
    RatePlotter(Ratemeter &inputrate,Ratemeter &analyzerate, QObject *parent=0);

  private slots:
    /** slot to tell the plotter to plot the current rates*/
    void plot();

  private:
    QTimer     _timer;        //!< the timer that tells it to plot the rate
    Ratemeter &_inputrate;    //!< reference to the input Ratemeter
    Ratemeter &_analyzerate;  //!< reference to the workers (analysis) Ratemeter
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
