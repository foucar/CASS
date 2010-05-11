//Copyright (C) 2010 Lutz Foucar

#include <iostream>

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
  /** @todo modify the by not modifying the cout flags*/
  //store the original flags of cout//
  std::ios_base::fmtflags original_flags = std::cout.flags();
  //set the precision for floating point to 2//
  std::cout.setf(std::ios::fixed, std::ios::floatfield);
  std::cout.setf(std::ios::showpoint);
  std::cout.precision(1);
  std::cout.width(4);
  //write the rate to the console//
  std::cout<<"\rInput: "<<_inputrate.calculateRate()<<"Hz | Analyze: "<<_analyzerate.calculateRate()<<"Hz  "<<std::flush;
  //restore the flags//
  std::cout.flags(original_flags);
}



// Local Variables:
// coding: utf-8
// mode: C++
// c-file-offsets: ((c . 0) (innamespace . 0))
// c-file-style: "Stroustrup"
// fill-column: 100
// End:
