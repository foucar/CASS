//Copyright (C) 2011 Lutz Foucar

/**
 * @file lucassview/main.cpp file the main function of lucassview
 *
 * @author Lutz Foucar
 */
#include <TRint.h>
#include <TROOT.h>
#include <iostream>
#include <string>
#include <memory>

#include "histo_updater.h"


/** main function
 *
 * create an instance of the ROOT interpreter and the Histogramupdater. Assign
 * the global variable the the HistogramUpdater instance. Then run the startup
 * script before letting the ROOT eventloop take over.
 *
 * @author Lutz Foucar
 */
int main(int argc, char *argv[])
{
  using namespace std;
  TRint theApp("App", &argc, argv);
  auto_ptr<HistogramUpdater> histUp(new HistogramUpdater("daq-amo-mon02",12321));
  gCASSClient = histUp.get();
  gROOT->ProcessLine(".x lucassStartup.C");
  theApp.Run();
  return 0;
}
