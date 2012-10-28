//Copyright (C) 2011 Lutz Foucar

/**
 * @file coltrims_analysis.cpp file contains the postprocessor specific for
 *                             coltrims analysis
 *
 * @author Lutz Foucar
 */

#include <stdexcept>
#include <cmath>
#include <algorithm>

#include <QtCore/QString>

#include "coltrims_analysis.h"

#include "acqiris_detectors_helper.h"
#include "histogram.h"
#include "cass_event.h"
#include "acqiris_device.h"
#include "cass.h"
#include "convenience_functions.h"
#include "cass_settings.h"
#include "log.h"

using namespace cass;
using namespace ACQIRIS;
using namespace std;


//----------------Electron Energy----------------------------------------------
pp5000::pp5000(PostProcessors &pp, const PostProcessors::key_t &key)
  :PostprocessorBackend(pp,key)

{
  loadSettings(0);
}

void pp5000::loadSettings(size_t)
{
  CASSSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(QString::fromStdString(_key));
  _detector = loadDelayDet(settings,5000,_key);
  _particle = loadParticle(settings,_detector,5000,_key);
  setupGeneral();
  if (!setupCondition())
    return;
  set1DHist(_result,_key);
  createHistList(2*cass::NbrOfWorkers);
  Log::add(Log::INFO,"PostProcessor '" + _key +
           "' extracts the total momentum of the particle '" + _particle +
           "' of detector '" + _detector +
           "' and then converts it to Energy assuming that it is an electron. " +
           "Condition is '" + _condition->key() + "'");
}

void pp5000::process(const CASSEvent &evt)
{
  DetectorBackend &rawdet(
        HelperAcqirisDetectors::instance(_detector)->detector(evt));
  DelaylineDetector &det (dynamic_cast<DelaylineDetector&>(rawdet));
  Particle &particle(det.particles()[_particle]);
  particleHits_t::iterator it(particle.hits().begin());
  _result->clear();
  _result->lock.lockForWrite();
  for (; it != particle.hits().end(); ++it)
  {
    double p = (*it)["roh"];
    double e_energy = p*p*13.6;
    dynamic_cast<Histogram1DFloat*>(_result)->fill(e_energy);
  }
  _result->lock.unlock();
}



//----------------PIPIPICO-------------------------------------------------------
pp5001::pp5001(PostProcessors &pp, const PostProcessors::key_t &key)
  :cass::PostprocessorBackend(pp,key)
{
  loadSettings(0);
}

void pp5001::loadSettings(size_t)
{
  CASSSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(QString::fromStdString(_key));
  _detector = settings.value("Detector","blubb").toString().toStdString();
  setupGeneral();
  if (!setupCondition())
    return;
  set2DHist(_result,_key);
  createHistList(2*cass::NbrOfWorkers);
  HelperAcqirisDetectors::instance(_detector)->loadSettings();
  Log::add(Log::INFO,"PostProcessor '" + _key +
           "' create a tripple coincidence Histogram of particles of '" + _detector +
           "'. Condition is '" + _condition->key() + "'");
}

void pp5001::process(const cass::CASSEvent &evt)
{
  DetectorBackend &rawdet(
        HelperAcqirisDetectors::instance(_detector)->detector(evt));
  DelaylineDetector &det01 (dynamic_cast<DelaylineDetector&>(rawdet));
  SignalProducer::signals_t::const_iterator it01(det01.mcp().output().begin());
  SignalProducer::signals_t::const_iterator end(det01.mcp().output().end());
  _result->clear();
  _result->lock.lockForWrite();
  for (; it01 != end;++it01)
  {
    SignalProducer::signals_t::const_iterator it02(it01+1);
    for (; it02 != end; ++it02)
    {

      SignalProducer::signals_t::const_iterator it03(it02+1);
      for (; it03 != end; ++it03)
      {
        dynamic_cast<Histogram2DFloat*>(_result)->fill((*it01)["time"]+(*it02)["time"],(*it03)["time"]);
      }
    }
  }
  _result->lock.unlock();
}


