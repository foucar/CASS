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

  /*! class that will plot the rates calculated in the given ratemeters

    @author Lutz Foucar
    */
  class CASSSHARED_EXPORT RatePlotter : public QObject
  {
    Q_OBJECT;

  public:
    /** constructor
      @param inputrate the rate of the input thread
      @param analyzerate the rate how fast the workers work on the events
      @param parent the qt parent of this object
      */
    RatePlotter(Ratemeter &inputrate,Ratemeter &analyzerate, QObject *parent=0);

  private slots:
    /** slot to tell the plotter to plot the current rate*/
    void plot();

  private:
    QTimer     _timer;        //!< the timer that tells it to plot the rate
    Ratemeter &_inputrate;    //!< reference to the input rate
    Ratemeter &_analyzerate;  //!< reference to the analyze rate
  };
}

#endif // RATEMETER_H
