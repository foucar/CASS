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

using namespace cass;


//----------------Electron Energy----------------------------------------------
pp5000::pp5000(PostProcessors &pp, const PostProcessors::key_t &key)
  :PostprocessorBackend(pp,key)

{
  loadSettings(0);
}

void pp5000::loadSettings(size_t)
{
  using namespace cass::ACQIRIS;
  using namespace std;
  CASSSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  _detector = loadDelayDet(settings,5000,_key);
  _particle = settings.value("Particle","NeP").toString().toStdString();
  setupGeneral();
  if (!setupCondition())
    return;
  set1DHist(_result,_key);
  createHistList(2*cass::NbrOfWorkers);
  cout<<endl<< "PostProcessor '"<<_key
      <<"' extracts the total momentum of the particle '"<<_particle
      <<"' of detector '"<<_detector
      <<"' and then converts it to Energy assuming that it is an electron. "
      <<"Condition is '"<<_condition->key()<<"'"
      <<endl;
}

void pp5000::process(const CASSEvent &evt)
{
  using namespace cass::ACQIRIS;
  using namespace std;
  DelaylineDetector *det
      (dynamic_cast<DelaylineDetector*>(HelperAcqirisDetectors::instance(_detector)->detector(evt)));
  Particle &particle(det->particles()[_particle]);
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

