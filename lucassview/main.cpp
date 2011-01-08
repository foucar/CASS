//Copyright (C) 2011 Lutz Foucar

/**
 * @file main.cpp file the main function of lucassview
 *
 * @author Lutz Foucar
 */
#include <TRint.h>
#include <TROOT.h>
#include <iostream>
#include <string>
#include <memory>

#include "histo_updater.h"

HistogramUpdater *gHistUpdater(0);
/** main function
 *
 * @author Lutz Foucar
 */
int main(int argc, char *argv[])
{
  using namespace std;
  TRint theApp("App", &argc, argv);
	//gROOT->ProcessLine(".L OnlineMacros.c");
  auto_ptr<HistogramUpdater> histUp(new HistogramUpdater("daq-amo-mon02",12321));
  gHistUpdater = histUp.get();
  theApp.Run();
	return 0;
}
