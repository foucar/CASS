//Copyright (C) 2010 Lutz Foucar

/**
 * @file delayline_detector.cpp file contains the implementatio of classes that
 *                              describe a delayline detector.
 *
 * @author Lutz Foucar
 */

#include <list>
#include <QtCore/QStringList>
#include <QtCore/QString>

#include "delayline_detector.h"

#include "cass_settings.h"
#include "detector_analyzer_backend.h"

using namespace cass::ACQIRIS;

void AnodeLayer::associate(const CASSEvent &evt)
{
  wireends_t::iterator it (_wireends.begin());
  for (; it != _wireends.end(); ++it)
    (*it).second.associate(evt);
}

void AnodeLayer::loadSettings(CASSSettings &s)
{
  s.beginGroup("One");
  _wireends['1'].loadSettings(s);
  s.endGroup();
  s.beginGroup("Two");
  _wireends['2'].loadSettings(s);
  s.endGroup();
}

DelaylineDetector::~DelaylineDetector()
{
//  delete _analyzer;
}

void DelaylineDetector::associate(const CASSEvent & evt)
{
  _newEventAssociated = true;
  _hits.clear();
  _mcp.associate(evt);
  anodelayers_t::iterator it (_anodelayers.begin());
  for (; it != _anodelayers.end(); ++it)
    (*it).second.associate(evt);
  particles_t::iterator pit (_particles.begin());
  for (; pit != _particles.end();++pit)
    (*pit).second.associate(this);
}

void DelaylineDetector::loadSettings(CASSSettings &s)
{
  using namespace std;
  s.beginGroup(_name.c_str());
  s.beginGroup("MCP");
  _mcp.loadSettings(s);
  s.endGroup();
  DelaylineType delaylinetype
      (static_cast<DelaylineType>(s.value("DelaylineType",Quad).toInt()));
  switch (delaylinetype)
  {
  case Hex:
    s.beginGroup("ULayer");
    _anodelayers['U'].loadSettings(s);
    s.endGroup();
    s.beginGroup("VLayer");
    _anodelayers['V'].loadSettings(s);
    s.endGroup();
    s.beginGroup("WLayer");
    _anodelayers['W'].loadSettings(s);
    s.endGroup();
    break;
  case Quad:
    s.beginGroup("XLayer");
    _anodelayers['X'].loadSettings(s);
    s.endGroup();
    s.beginGroup("YLayer");
    _anodelayers['Y'].loadSettings(s);
    s.endGroup();
    break;
  default:
    throw std::invalid_argument("delayline type does not exist");
    break;
  }
//  delete _analyzer;
  _analyzer =
      DetectorAnalyzerBackend::instance(static_cast<DetectorAnalyzerType>(s.value("AnalysisMethod",DelaylineSimple).toInt()));
  _analyzer->loadSettings(s, *this);
  s.beginGroup("Particles");
  _particles.clear();
  QStringList particlesNameList(s.childGroups());
  QStringList::const_iterator pNamesIt (particlesNameList.begin());
  for (; pNamesIt != particlesNameList.end();++pNamesIt)
  {
    s.beginGroup(*pNamesIt);
    string particleName (pNamesIt->toStdString());
    _particles[particleName].loadSettings(s);
    s.endGroup();
  }
  s.endGroup();
  s.endGroup();
}

detectorHits_t& DelaylineDetector::hits()
{
  bool newEventAssociated (_newEventAssociated);
  _newEventAssociated = false;
  return (newEventAssociated)? (*_analyzer)(_hits):_hits;
}
