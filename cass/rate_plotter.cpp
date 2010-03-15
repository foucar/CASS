//Copyright (C) 2010 lmf
#include <iostream>

#include "rate_plotter.h"

#include "ratemeter.h"

cass::RatePlotter::RatePlotter(Ratemeter &inputrate,Ratemeter &analyzerate, QObject *parent)
    :QObject(parent),
    _timer(this),
    _inputrate(inputrate),
    _analyzerate(analyzerate)
{
  //start the timer//
  connect (&_timer,SIGNAL(timeout()),this,SLOT(plot()));
  _timer.start(1000);
}

cass::RatePlotter::plot()
{
  //store the original flags of cout//
  std::ios_base::fmtflags original_flags = std::cout.flags();
  //set the precision for floating point to 2//
  std::cout.setf(ios::fixed, ios::floatfield);
  std::cout.setf(ios::showpoint);
  std::cout.precision(2);
  std::cout.width(4);

  std::cout<<"\rInput: "<<_inputrate.calculateRate()<<"Hz | Analyze: "<<_analyzerate.calculateRate()<<"Hz"<<std::endl;
  //restore the flags//
  std::cout.flags(original_flags);
}
