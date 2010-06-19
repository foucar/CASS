// Copyright (C) 2010 Lutz Foucar

#include "convenience_functions.h"
#include "cass_exceptions.h"
#include "histogram.h"

using namespace cass;

void cass::set1DHist(cass::HistogramBackend*& hist, PostProcessors::key_t key)
{
  //open the settings//
  QSettings param;
  param.beginGroup("PostProcessor");
  param.beginGroup(key.c_str());
  //create new histogram using the parameters//
  VERBOSEOUT(std::cerr << "Creating 1D histogram with"
             <<" XNbrBins:"<<param.value("XNbrBins",1).toUInt()
             <<" XLow:"<<param.value("XLow",0).toFloat()
             <<" XUp:"<<param.value("XUp",0).toFloat()
             <<std::endl);
  hist = new cass::Histogram1DFloat(param.value("XNbrBins",1).toUInt(),
                                    param.value("XLow",0).toFloat(),
                                    param.value("XUp",0).toFloat());
}

void cass::set2DHist(cass::HistogramBackend*& hist, PostProcessors::key_t key)
{
  //open the settings//
  QSettings param;
  param.beginGroup("PostProcessor");
  param.beginGroup(key.c_str());
  //create new histogram using the parameters//
  VERBOSEOUT(std::cerr << "Creating 2D histogram with"
             <<" XNbrBins:"<<param.value("XNbrBins",1).toUInt()
             <<" XLow:"<<param.value("XLow",0).toFloat()
             <<" XUp:"<<param.value("XUp",0).toFloat()
             <<" YNbrBins:"<<param.value("YNbrBins",1).toUInt()
             <<" YLow:"<<param.value("YLow",0).toFloat()
             <<" YUp:"<<param.value("YUp",0).toFloat()
             <<std::endl);
  hist = new cass::Histogram2DFloat(param.value("XNbrBins",1).toUInt(),
                                    param.value("XLow",0).toFloat(),
                                    param.value("XUp",0).toFloat(),
                                    param.value("YNbrBins",1).toUInt(),
                                    param.value("YLow",0).toFloat(),
                                    param.value("YUp",0).toFloat());
}
