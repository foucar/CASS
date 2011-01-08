//Copyright (C) 2011 Lutz Foucar

/**
 * @file histoupdater.cpp file contains the classes that update histograms
 *
 * @author Lutz Foucar
 */
#include <cmath>
#include <limits>

#include "histo_updater.h"

#include "tcpclient.h"

using namespace lucassview;

HistogramUpdater::HistogramUpdater(const std::string &server, int port)
  :_server(server),
   _port(port),
   _timer(std::auto_ptr<TTimer>(new TTimer()))
{
  _timer->Connect("Timeout()", "HistogramUpdater",this, "upateHistograms()");
}

void HistogramUpdater::autoUpdate(double freq)
{
  using namespace std;
  if(freq < sqrt(numeric_limits<double>::epsilon()))
    _timer->Stop();
  else
  {
    _timer->Start(static_cast<int>(1.e3/freq));
  }
}

void HistogramUpdater::updateHistograms()
{

}

