// Copyright (C) 2010-2011 Lutz Foucar

/**
 * @file convenience_functions.cpp file contains definition of functions that
 *                                 help other processors to do their job.
 *
 * @author Lutz Foucar
 */

#include <string>

#include "convenience_functions.h"

#include "cass_exceptions.hpp"
#include "cass_settings.h"
#include "acqiris_detectors_helper.h"
#include "delayline_detector.h"
#include "log.h"

using namespace cass;
using namespace ACQIRIS;
using namespace std;

Processor::result_t::shared_pointer cass::set1DHist(const Processor::name_t &name)
{
  //open the settings//
  CASSSettings s;
  s.beginGroup("Processor");
  s.beginGroup(QString::fromStdString(name));
  //create new histogram using the parameters//
  Log::add(Log::VERBOSEINFO,string("set1dHist(): Creating 1D histogram with") +
           " XNbrBins:" + toString(s.value("XNbrBins",1).toUInt()) +
           " XLow:" + toString(s.value("XLow",0).toFloat()) +
           " XUp:" + toString(s.value("XUp",0).toFloat()) +
           " XTitle:" + s.value("XTitle","x-axis").toString().toStdString());
  return Processor::result_t::shared_pointer
      (new Processor::result_t(Processor::result_t::axe_t
                               (s.value("XNbrBins",1).toUInt(),
                               s.value("XLow",0).toFloat(),
                               s.value("XUp",0).toFloat(),
                               s.value("XTitle","x-axis").toString().toStdString())));
}

Processor::result_t::shared_pointer cass::set2DHist(const Processor::name_t &name)
{
  //open the settings//
  CASSSettings s;
  s.beginGroup("Processor");
  s.beginGroup(QString::fromStdString(name));
  //create new histogram using the parameters//
  Log::add(Log::VERBOSEINFO,string("set2DHist(): Creating 2D histogram with") +
             " XNbrBins:" + toString(s.value("XNbrBins",1).toUInt()) +
             " XLow:" + toString(s.value("XLow",0).toFloat()) +
             " XUp:" + toString(s.value("XUp",0).toFloat()) +
             " XTitle:" + s.value("XTitle","x-axis").toString().toStdString() +
             " YNbrBins:" + toString(s.value("YNbrBins",1).toUInt()) +
             " YLow:" + toString(s.value("YLow",0).toFloat()) +
             " YUp:" + toString(s.value("YUp",0).toFloat()) +
             " YTitle:" + s.value("YTitle","y-axis").toString().toStdString());
  return Processor::result_t::shared_pointer
      (new Processor::result_t(Processor::result_t::axe_t(
                              s.value("XNbrBins",1).toUInt(),
                              s.value("XLow",0).toFloat(),
                              s.value("XUp",0).toFloat(),
                              s.value("XTitle","x-axis").toString().toStdString()),
                            Processor::result_t::axe_t(
                              s.value("YNbrBins",1).toUInt(),
                              s.value("YLow",0).toFloat(),
                              s.value("YUp",0).toFloat(),
                              s.value("YTitle","y-axis").toString().toStdString())));
}

string cass::ACQIRIS::loadDelayDet(CASSSettings &s,
                                   int ppNbr,
                                   const string &key)
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
                                   const string &key)
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

