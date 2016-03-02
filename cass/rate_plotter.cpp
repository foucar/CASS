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
#include "result.hpp"
#include "processor_manager.h"

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
  stringstream output;
  output << "ProcessingInfo: ";
  s.beginGroup("ProcessingInfo");
  _showInfo = s.value("ShowInfo",true).toBool();
  output << "ShowInfo '"<< std::boolalpha << _showInfo << "', ";
  _filename = s.value("Output","").toString().toStdString();
  output << "Output to '" <<(_filename == "" ? "COUT":_filename) <<"', ";
  _interval = s.value("UpdateInterval",1).toInt();
  output << "UpdateInterval '" << _interval << "', ";
  _showInputRate = s.value("ShowInputRate",true).toBool();
  output << "ShowInputRate '"<< std::boolalpha << _showInputRate << "', ";
  _showInputLoad= s.value("ShowInputLoad",true).toBool();
  output << "ShowInputLoad '"<< std::boolalpha << _showInputLoad << "', ";
  _showAnalysisRate = s.value("ShowAnalysisRate",true).toBool();
  output << "ShowAnalysisRate '"<< std::boolalpha << _showAnalysisRate << "', ";
  _showProcessRatio = s.value("ShowProcessRatio",true).toBool();
  output << "ShowProcessRatio '"<< std::boolalpha << _showProcessRatio << "', ";
  _showNProcessedEvents = s.value("ShowNbrProcessedEvents",false).toBool();
  output << "ShowNbrProcessEvents '"<< std::boolalpha << _showNProcessedEvents << "', ";
  _newLine = s.value("NewLine",false).toBool();
  output << "NewLine '"<< std::boolalpha << _newLine << "', ";
  output << "ValueProcessors: ";
  int size = s.beginReadArray("ValueProcessors");
  for (int i = 0; i < size; ++i)
  {
    s.setArrayIndex(i);
    ProcProperties proc;
    proc.name =  s.value("Name","Unknown").toString().toStdString();
    proc.fieldWidth =  s.value("FieldWidth",10).toInt();
    proc.precision = s.value("Precision",7).toInt();
    if (proc.name != "Unknown")
    {
      _procs.push_back(proc);
      output << "Name '" << proc.name <<", "
             << "FieldWidth '" << proc.fieldWidth << ", "
             << "Precision '" << proc.precision << ", ";
    }
  }
  Log::add(Log::INFO,output.str());
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
             << InputBase::reference().progress()*100. << "%";
    }
    if (_showNProcessedEvents)
    {
      output << " | Events: "
             << std::setw(7)
             << InputBase::reference().eventcounter();
    }
    for (proclist_t::const_iterator it(_procs.begin()); it !=_procs.end(); ++it)
    {
      try
      {
        QWriteLocker pplock(&ProcessorManager::instance()->lock);
        Processor::result_t::shared_pointer result
            (ProcessorManager::reference().getProcessor(it->name).resultCopy(0));
        if (result->dim() != 0)
        {
          continue;
        }
        else
        {
          output << " | " << result->name() << ": "
                 << std::setw(it->fieldWidth)
                 << std::fixed << std::setprecision(it->precision)
                 << result->getValue();
        }

      }
      catch(const InvalidResultError& error)
      {
        Log::add(Log::ERROR,string("ProcessingInfo: ") + error.what());
      }
      catch(const InvalidProcessorError& error)
      {
        Log::add(Log::ERROR,string("ProcessingInfo: ") + error.what());
      }
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
