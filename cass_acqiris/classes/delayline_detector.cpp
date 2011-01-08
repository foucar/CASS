//Copyright (C) 2010 Lutz Foucar

/**
 * @file delayline_detector.cpp file contains the implementatio of classes that
 *                              describe a delayline detector.
 *
 * @author Lutz Foucar
 */

#include "delayline_detector.h"

#include "cass_settings.h"
#include "detector_analyzer_backend.h"

using namespace cass::ACQIRIS;

void AnodeLayer::associate(const CASSEvent &evt)
{
  wireends_t::iterator it (_wireend.begin());
  for (; it != _wireend.end(); ++it)
    (*it).second.associate(evt);
}

void AnodeLayer::loadSettings(CASSSettings &s)
{
  s.beginGroup("One");
  _wireend['1'].loadSettings(s);
  s.endGroup();
  s.beginGroup("Two");
  _wireend['2'].loadSettings(s);
  s.endGroup();


  //  _tsLow  = p->value("LowerTimesumConditionLimit",0.).toDouble();
  //  _tsHigh = p->value("UpperTimesumConditionLimit",20000.).toDouble();
  //  _sf     = p->value("Scalefactor",0.5).toDouble();
}

void DelaylineDetector::associate(const CASSEvent & evt)
{
  _newEventAssociated = true;
  _hits.clear();
  _mcp.associate(evt);
  anodelayers_t::iterator it (_anodelayers.begin());
  for (; it != _anodelayers.end(); ++it)
    (*it).second.associate(evt);
}

void DelaylineDetector::loadSettings(CASSSettings &s)
{
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
  delete _analyzer;
  _analyzer =
      DetectorAnalyzerBackend::instance(static_cast<DetectorAnalyzerType>(s.value("AnalysisMethod",DelaylineSimple).toInt()));
  _analyzer->loadSettings(s);
  s.endGroup();



  //  _runtime      = s.value("Runtime",150).toDouble();
  //  _mcpRadius    = s.value("McpRadius",44.).toDouble();
  //  _angle        = s.value("Angle",0.).toDouble()*3.1415/180.;
}

DelaylineDetector::hits_t& DelaylineDetector::hits()
{
  bool newEventAssociated (_newEventAssociated);
  _newEventAssociated = false;
  return (newEventAssociated)? (*_analyzer)(_hits):_hits;
}
