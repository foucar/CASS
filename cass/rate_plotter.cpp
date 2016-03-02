//Copyright (C) 2010,2013,2016 Lutz Foucar

/**
 * @file rate_plotter.cpp file contains declaration of class to plot the rate
 *                        calculated by ratemeters
 *
 * @author Lutz Foucar
 */

#define __STDC_FORMAT_MACROS

#include <iostream>
#include <sstream>
#include <iomanip>
#include <stdio.h>
#include <inttypes.h>

#include "rate_plotter.h"
#include "ratemeter.h"
#include "input_base.h"
#include "log.h"
#include "cass_settings.h"

using namespace std;
using namespace cass;

RatePlotter::RatePlotter(Ratemeter &inputrate,
                         Ratemeter &inputload,
                         Ratemeter &analyzerate,
                         QObject *parent)
  : QThread(parent),
    _inputrate(inputrate),
    _inputload(inputload),
    _analyzerate(analyzerate)
{
  CASSSettings s;
  s.beginGroup("ProcessingStatistics");
  _showInfo = s.value("ShowInfo",true).toBool();
  _filename = s.value("Output","").toString().toStdString();
  _interval = s.value("UpdateInterval",1).toInt();
  _showInputRate = s.value("ShowInputRate",true).toBool();
  _showInputLoad= s.value("ShowInputLoad",true).toBool();
  _showAnalysisRate = s.value("ShowAnalysisRate",true).toBool();
  _showProcessRatio = s.value("ShowProcessRatio",true).toBool();
  _showNProcessedEvents = s.value("ShowNbrProcessedEvents",false).toBool();
  _newLine = s.value("NewLine",false).toBool();
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
  if (!_showInfo)
    return;

  while(true)
  {
    sleep(_interval);
    stringstream output;
    if (!_newLine)
      output <<"\r";
    if (_showInputRate)
    {
      output << "Input: " << std::setw(5) << std::fixed << std::setprecision(1)
             << _inputrate.calculateRate() << "Hz";
    }
    if (_showInputLoad)
    {
      double load(_inputload.calculateRate()/1024.);
      string size("B/s");
      if (load > 999.9)
      {
        load /= 1024.;
        size = "KB/s";
      }
      if (load > 999.9)
      {
        load /= 1024.;
        size = "MB/s";
      }
      if (load > 999.9)
      {
        load /= 1024.;
        size = "GB/s";
      }
      if (load > 999.9)
      {
        load /= 1024.;
        size = "TB/s";
      }
      output << " | Load: "
             << std::setw(5) << std::fixed << std::setprecision(1)
             << load << size;
    }
    if (_showAnalysisRate)
    {
      output << " | Analyze: "
             << std::setw(5) << std::fixed << std::setprecision(1)
             << _analyzerate.calculateRate() << "Hz";
    }
    if (_showProcessRatio)
    {
      output << " | Processed: "
             << std::setw(5) << std::fixed << std::setprecision(1)
             << InputBase::reference().processed()*100. << "%";
    }
    if (_showNProcessedEvents)
    {
      output << " | Events: "
             << std::setw(7)
             << InputBase::reference().eventcounter();
    }

//    char tmp[256];
//    snprintf(tmp, 255, "\rInput: %5.1fHz (%5.1f%cB/s) | Analyze: %5.1fHz | Processed: %5.1f%% | Events: %" PRIu64 "",
//
//             _inputrate.calculateRate(),load,shortsize,
//             _analyzerate.calculateRate(),
//             InputBase::reference().processed()*100.,
//             InputBase::reference().eventcounter());

    // taken from http://stackoverflow.com/questions/366955/obtain-a-stdostream-either-from-stdcout-or-stdofstreamfile
    std::streambuf * buf;
    std::ofstream of;
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

    out << output << flush;
  }
}
