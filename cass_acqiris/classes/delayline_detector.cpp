//Copyright (C) 2010 Lutz Foucar

/**
 * @file delayline_detector.cpp file contains the implementatio of classes that
 *                              describe a delayline detector.
 *
 * @author Lutz Foucar
 */

#include <QtCore/QStringList>
#include <QtCore/QString>

#include "delayline_detector.h"

#include "cass_settings.h"
#include "detector_analyzer_backend.h"

using namespace cass::ACQIRIS;

void AnodeLayer::associate(const CASSEvent &evt)
{
//  std::cout <<"    AnodeLay 1"<<std::endl;
  wireends_t::iterator it (_wireends.begin());
//  std::cout <<"    AnodeLay 2"<<std::endl;
  for (; it != _wireends.end(); ++it)
  {
//    std::cout <<"    AnodeLay 3  "<<it->first<<std::endl;
    (*it).second.associate(evt);
  }
//  std::cout <<"    AnodeLay 4"<<std::endl;
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

void DelaylineDetector::associate(const CASSEvent & evt)
{
//  std::cout << "   DelayDet 1"<<std::endl;
  _newEventAssociated = true;
//  std::cout << "   DelayDet 2"<<std::endl;
  _hits.clear();
//  std::cout << "   DelayDet 3"<<std::endl;
  _mcp.associate(evt);
//  std::cout << "   DelayDet 4"<<std::endl;
  anodelayers_t::iterator it (_anodelayers.begin());
//  std::cout << "   DelayDet 5"<<std::endl;
  for (; it != _anodelayers.end(); ++it)
  {
//    std::cout << "   DelayDet 6 "<<it->first<<std::endl;
    (*it).second.associate(evt);
  }
//  std::cout << "   DelayDet 7"<<std::endl;
  particles_t::iterator pit (_particles.begin());
//  std::cout << "   DelayDet 8"<<std::endl;
  for (; pit != _particles.end();++pit)
    (*pit).second.associate(this);
//  std::cout << "   DelayDet 9"<<std::endl;
}

void DelaylineDetector::loadSettings(CASSSettings &s)
{
  using namespace std;
  s.beginGroup(_name.c_str());
  DelaylineType delaylinetype
      (static_cast<DelaylineType>(s.value("DelaylineType",Hex).toInt()));
  s.beginGroup("MCP");
  _mcp.loadSettings(s);
  s.endGroup();
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
//  std::cout << " DelayDet 1"<<std::endl;
  bool newEventAssociated (_newEventAssociated);
//  std::cout << " DelayDet 2 "<<std::boolalpha<<_newEventAssociated<<std::endl;
  _newEventAssociated = false;
//  std::cout << " DelayDet 3 "<<_analyzer.get()<<std::endl;
  return (newEventAssociated)? (*_analyzer)(_hits):_hits;
//  std::cout << " DelayDet 4"<<std::endl;
}
