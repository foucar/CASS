// Copyright (C) 2010-2011 Lutz Foucar

/**
 * @file convenience_functions.cpp file contains definition of functions that
 *                                 help other postprocessors to do their job.
 *
 * @author Lutz Foucar
 */

#include "convenience_functions.h"

#include "cass_exceptions.h"
#include "histogram.h"
#include "cass_settings.h"
#include "acqiris_detectors_helper.h"
#include "delayline_detector.h"

using namespace cass;
using namespace cass::ACQIRIS;

void cass::set1DHist(HistogramBackend*& hist, PostProcessors::key_t key)
{
  //open the settings//
  CASSSettings param;
  param.beginGroup("PostProcessor");
  param.beginGroup(key.c_str());
  //create new histogram using the parameters//
  VERBOSEOUT(std::cerr << "Creating 1D histogram with"
             <<" XNbrBins:"<<param.value("XNbrBins",1).toUInt()
             <<" XLow:"<<param.value("XLow",0).toFloat()
             <<" XUp:"<<param.value("XUp",0).toFloat()
             <<" XTitle:"<<param.value("XTitle","x-axis").toString().toStdString()
             <<std::endl);
  hist = new cass::Histogram1DFloat(param.value("XNbrBins",1).toUInt(),
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
  VERBOSEOUT(std::cerr << "Creating 2D histogram with"
             <<" XNbrBins:"<<param.value("XNbrBins",1).toUInt()
             <<" XLow:"<<param.value("XLow",0).toFloat()
             <<" XUp:"<<param.value("XUp",0).toFloat()
             <<" XTitle:"<<param.value("XTitle","x-axis").toString().toStdString()
             <<" YNbrBins:"<<param.value("YNbrBins",1).toUInt()
             <<" YLow:"<<param.value("YLow",0).toFloat()
             <<" YUp:"<<param.value("YUp",0).toFloat()
             <<" YTitle:"<<param.value("YTitle","y-axis").toString().toStdString()
             <<std::endl);
  hist = new cass::Histogram2DFloat(param.value("XNbrBins",1).toUInt(),
                                    param.value("XLow",0).toFloat(),
                                    param.value("XUp",0).toFloat(),
                                    param.value("YNbrBins",1).toUInt(),
                                    param.value("YLow",0).toFloat(),
                                    param.value("YUp",0).toFloat(),
                                    param.value("XTitle","x-axis").toString().toStdString(),
                                    param.value("YTitle","y-axis").toString().toStdString());
}

std::string cass::ACQIRIS::loadDelayDet(CASSSettings &s,
                                        int ppNbr,
                                        const PostProcessors::key_t& key)
{
  using namespace std;
  string detector
      (s.value("Detector","blubb").toString().toStdString());
  HelperAcqirisDetectors *dethelp (HelperAcqirisDetectors::instance(detector));
  if (dethelp->detectortype() != Delayline)
  {
    stringstream ss;
    ss <<"pp"<<ppNbr<<"::loadSettings()'"<<key<<"': Error detector '"<<detector<<"' is not a Delaylinedetector.";
    throw (invalid_argument(ss.str()));
  }
  dethelp->loadSettings();
  return detector;
}

std::string cass::ACQIRIS::loadParticle(CASSSettings &s,
                                        const std::string &detector,
                                        int ppNbr,
                                        const PostProcessors::key_t& key)
{
  using namespace std;
  string particle (s.value("Particle","NeP").toString().toStdString());
  const DelaylineDetector *det
      (dynamic_cast<const DelaylineDetector*>(HelperAcqirisDetectors::instance(detector)->detector()));
  if (det->particles().end() == det->particles().find(particle))
  {
    stringstream ss;
    ss <<"pp"<<ppNbr<<"::loadSettings()'"<<key<<"': Error Particle '"<<particle
        <<"' is not defined for detector '"<<detector<<"'";
    throw invalid_argument(ss.str());
  }
  return particle;
}

