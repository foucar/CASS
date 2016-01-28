//Copyright (C) 2010,2013 Lutz Foucar

/**
 * @file rate_plotter.cpp file contains declaration of class to plot the rate
 *                        calculated by ratemeters
 *
 * @author Lutz Foucar
 */

#define __STDC_FORMAT_MACROS

#include <iostream>
#include <stdio.h>
#include <inttypes.h>

#include "rate_plotter.h"
#include "ratemeter.h"
#include "input_base.h"
#include "log.h"

using namespace std;
using namespace cass;

RatePlotter::RatePlotter(Ratemeter &inputrate,
                         Ratemeter &inputload,
                         Ratemeter &analyzerate,
                         int updateInterval,
                         const string &filename,
                         QObject *parent)
  : QThread(parent),
    _inputrate(inputrate),
    _inputload(inputload),
    _analyzerate(analyzerate),
    _interval(updateInterval),
    _filename(filename)
{
  Log::add(Log::INFO,"Status info will be written to " +
           (_filename==""?"cout":_filename));
}

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
    char shortsize('K');
    double load(_inputload.calculateRate()/1024.);
    if (load > 999.9)
    {
      load /= 1024.;
      shortsize = 'M';
    }
    if (load > 999.9)
    {
      load /= 1024.;
      shortsize = 'G';
    }
    if (load > 999.9)
    {
      load /= 1024.;
      shortsize = 'T';
    }
    snprintf(tmp, 255, "\rInput: %5.1fHz (%5.1f%cB/s) | Analyze: %5.1fHz | Processed: %5.1f%% | Events: %" PRIu64 "",
             _inputrate.calculateRate(),load,shortsize,
             _analyzerate.calculateRate(),
             InputBase::reference().processed()*100.,
             InputBase::reference().eventcounter());

    std::streambuf * buf;
    std::ofstream of;
    // taken from http://stackoverflow.com/questions/366955/obtain-a-stdostream-either-from-stdcout-or-stdofstreamfile
    if(_filename!="")
    {
      of.open(_filename.c_str());
      buf = of.rdbuf();
    }
    else
    {
      buf = cout.rdbuf();
    }
    std::ostream out(buf);

    out << tmp << flush;
  }
}
