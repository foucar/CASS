// Copytight (C) 2009 Jochen Küpper

#include <iostream>
#include <QtCore/QMutexLocker>
#include "analyzer.h"
#include "remi_analysis.h"
#include "vmi_analysis.h"
#include "pnccd_analysis.h"
#include "machine_analysis.h"
#include "database.h"


// define static members
cass::Analyzer *cass::Analyzer::_instance(0);
QMutex cass::Analyzer::_mutex;


cass::Analyzer::Analyzer()
{
  //create the analyzers//
  _analyzer[VMI]          = new cass::VMI::Analysis();
  _analyzer[pnCCD]        = new cass::pnCCD::Analysis();
  _analyzer[REMI]         = new cass::REMI::Analysis();
  _analyzer[MachineData]  = new cass::MachineData::Analysis();
}

cass::Analyzer::~Analyzer()
{
  for (std::map<Analyzers,cass::AnalysisBackend*>::iterator it=_analyzer.begin() ; it != _analyzer.end(); ++it )
    delete (it->second);
}

cass::Analyzer* cass::Analyzer::instance()
{
  QMutexLocker locker(&_mutex);
  if(0 == _instance)
    _instance = new Analyzer();
  return _instance;
}

void cass::Analyzer::destroy()
{
  QMutexLocker locker(&_mutex);
  delete _instance;
  _instance = 0;
}


void cass::Analyzer::processEvent(cass::CASSEvent* cassevent)
{
  //use the analyzers to analyze the event//
  //iterate through all analyzers and send the cassevent to them//
  for (std::map<Analyzers,cass::AnalysisBackend*>::iterator it=_analyzer.begin() ; it != _analyzer.end(); ++it )
    (*(it->second))(cassevent);
}

void cass::Analyzer::loadSettings()
{
  //iterate through all analyzers and load the settings of them//
  for (std::map<Analyzers,cass::AnalysisBackend*>::iterator it=_analyzer.begin() ; it != _analyzer.end(); ++it )
    it->second->loadSettings();
}

void cass::Analyzer::saveSettings()
{
  //iterate through all analyzers and load the settings of them//
  for (std::map<Analyzers,cass::AnalysisBackend*>::iterator it=_analyzer.begin() ; it != _analyzer.end(); ++it )
    it->second->saveSettings();
}

