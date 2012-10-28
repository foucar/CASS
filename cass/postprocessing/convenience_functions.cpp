// Copyright (C) 2010-2011 Lutz Foucar

/**
 * @file convenience_functions.cpp file contains definition of functions that
 *                                 help other postprocessors to do their job.
 *
 * @author Lutz Foucar
 */

#include <string>

#include "convenience_functions.h"

#include "cass_exceptions.h"
#include "histogram.h"
#include "cass_settings.h"
#include "acqiris_detectors_helper.h"
#include "delayline_detector.h"
#include "log.h"

using namespace cass;
using namespace ACQIRIS;
using namespace std;

void cass::set1DHist(HistogramBackend*& hist, PostProcessors::key_t key)
{
  //open the settings//
  CASSSettings param;
  param.beginGroup("PostProcessor");
  param.beginGroup(key.c_str());
  //create new histogram using the parameters//
  Log::add(Log::VERBOSEINFO,string("set1dHist(): Creating 1D histogram with") +
           " XNbrBins:" + toString(param.value("XNbrBins",1).toUInt()) +
           " XLow:" + toString(param.value("XLow",0).toFloat()) +
           " XUp:" + toString(param.value("XUp",0).toFloat()) +
           " XTitle:" + param.value("XTitle","x-axis").toString().toStdString());
  hist = new Histogram1DFloat(param.value("XNbrBins",1).toUInt(),
                              param.value("XLow",0).toFloat(),
                              param.value("XUp",0).toFloat(),
                              param.value("XTitle","x-axis").toString().toStdString());
}

void cass::set2DHist(HistogramBackend*& hist, PostProcessors::key_t key)
{
  //open the settings//
  CASSSettings param;
  param.beginGroup("PostProcessor");
  param.beginGroup(key.c_str());
  //create new histogram using the parameters//
  Log::add(Log::VERBOSEINFO,string("set2DHist(): Creating 2D histogram with") +
             " XNbrBins:" + toString(param.value("XNbrBins",1).toUInt()) +
             " XLow:" + toString(param.value("XLow",0).toFloat()) +
             " XUp:" + toString(param.value("XUp",0).toFloat()) +
             " XTitle:" + param.value("XTitle","x-axis").toString().toStdString() +
             " YNbrBins:" + toString(param.value("YNbrBins",1).toUInt()) +
             " YLow:" + toString(param.value("YLow",0).toFloat()) +
             " YUp:" + toString(param.value("YUp",0).toFloat()) +
             " YTitle:" + param.value("YTitle","y-axis").toString().toStdString());
  hist = new Histogram2DFloat(param.value("XNbrBins",1).toUInt(),
                              param.value("XLow",0).toFloat(),
                              param.value("XUp",0).toFloat(),
                              param.value("YNbrBins",1).toUInt(),
                              param.value("YLow",0).toFloat(),
                              param.value("YUp",0).toFloat(),
                              param.value("XTitle","x-axis").toString().toStdString(),
                              param.value("YTitle","y-axis").toString().toStdString());
}

string cass::ACQIRIS::loadDelayDet(CASSSettings &s,
                                   int ppNbr,
                                   const PostProcessors::key_t& key)
{
  string detector
      (s.value("Detector","blubb").toString().toStdString());
  HelperAcqirisDetectors::shared_pointer dethelp(HelperAcqirisDetectors::instance(detector));
  if (dethelp->detectortype() != Delayline)
    throw invalid_argument("pp" + toString(ppNbr) + "::loadSettings()'" + key +
                           "': Error detector '" + detector +
                           "' is not a Delaylinedetector.");
  dethelp->loadSettings();
  return detector;
}

string cass::ACQIRIS::loadParticle(CASSSettings &s,
                                   const string &detector,
                                   int ppNbr,
                                   const PostProcessors::key_t& key)
{
  string particle (s.value("Particle","NeP").toString().toStdString());
  const DelaylineDetector &det(
        dynamic_cast<const DelaylineDetector&>(HelperAcqirisDetectors::instance(detector)->detector()));
  if (det.particles().end() == det.particles().find(particle))
    throw invalid_argument("pp" + toString(ppNbr) + "::loadSettings()'" + key +
                           "': Error Particle '" + particle +
                           "' is not defined for detector '" + detector +"'");
  return particle;
}

