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

/** main function
 *
 * @author Lutz Foucar
 */
int main(int argc, char *argv[])
{
  TRint theApp("App", &argc, argv);
	//gROOT->ProcessLine(".L OnlineMacros.c");
  theApp.Run();
	return 0;
}
