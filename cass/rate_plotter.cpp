//Copyright (C) 2010 Lutz Foucar

#include <iostream>
#include <stdio.h>

#include "rate_plotter.h"
#include "ratemeter.h"

cass::RatePlotter::RatePlotter(Ratemeter &inputrate,Ratemeter &analyzerate, QObject *parent)
    :QObject(parent),
    _timer(this),
    _inputrate(inputrate),
    _analyzerate(analyzerate)
{
  //start the timer that will call the plot() function//
  connect (&_timer,SIGNAL(timeout()),this,SLOT(plot()));
  _timer.start(1000);
}

void cass::RatePlotter::plot()
{
  char tmp[256];
  snprintf(tmp, 255, "\rInput: %4.1fHz | Analyze: %4.1fHz",
           _inputrate.calculateRate(), _analyzerate.calculateRate());
  std::cout << tmp << std::flush;
}



// Local Variables:
// coding: utf-8
// mode: C++
// c-file-offsets: ((c . 0) (innamespace . 0))
// c-file-style: "Stroustrup"
// fill-column: 100
// End:
