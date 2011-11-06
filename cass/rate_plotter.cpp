//Copyright (C) 2010 Lutz Foucar

/**
 * @file rate_plotter.cpp file contains declaration of class to plot the rate
 *                        calculated by ratemeters
 *
 * @author Lutz Foucar
 */

#include <iostream>
#include <stdio.h>

#include "rate_plotter.h"
#include "ratemeter.h"

using namespace std;
using namespace cass;

RatePlotter::RatePlotter(Ratemeter &inputrate,
                         Ratemeter &analyzerate,
                         bool plot,
                         QObject *parent)
    :QObject(parent),
    _timer(this),
    _inputrate(inputrate),
    _analyzerate(analyzerate)
{
  //start the timer that will call the plot() function//
  connect (&_timer,SIGNAL(timeout()),this,SLOT(plot()));
  if (plot)
    _timer.start(1000);
}

void RatePlotter::plot()
{
  char tmp[256];
  snprintf(tmp, 255, "\rInput: %4.1fHz | Analyze: %4.1fHz",
           _inputrate.calculateRate(), _analyzerate.calculateRate());
  cout << tmp << flush;
}



// Local Variables:
// coding: utf-8
// mode: C++
// c-file-offsets: ((c . 0) (innamespace . 0))
// c-file-style: "Stroustrup"
// fill-column: 100
// End:
