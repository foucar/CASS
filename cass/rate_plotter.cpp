//Copyright (C) 2010,2013 Lutz Foucar

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
                         Ratemeter &inputload,
                         Ratemeter &analyzerate,
                         int updateInterval,
                         QObject *parent)
  : QThread(parent),
    _inputrate(inputrate),
    _inputload(inputload),
    _analyzerate(analyzerate),
    _interval(updateInterval)
{}

RatePlotter::~RatePlotter()
{
  if(isRunning())
    terminate();
  wait();
}

void RatePlotter::run()
{
  while(true)
  {
    sleep(_interval);
    char tmp[256];
    snprintf(tmp, 255, "\rInput: %4.1fHz (%4.1fMB/s) | Analyze: %4.1fHz",
             _inputrate.calculateRate(),_inputload.calculateRate(),
             (_analyzerate.calculateRate()/1024/1024));
    cout << tmp << flush;
  }
}
