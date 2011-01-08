//Copyright (C) 2011 Lutz Foucar

/**
 * @file histoupdater.cpp file contains the classes that update histograms
 *
 * @author Lutz Foucar
 */

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

void HistogramUpdater::updateHistograms()
{

}

void HistogramUpdater::autoUpdate(double freq)
{

}
