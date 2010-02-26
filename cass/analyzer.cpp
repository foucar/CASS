// Copytight (C) 2009 Jochen Küpper

#include <iostream>
#include <QtCore/QMutexLocker>
#include "analyzer.h"
#include "acqiris_analysis.h"
#include "ccd_analysis.h"
#include "pnccd_analysis.h"
#include "machine_analysis.h"


// define static members
cass::Analyzer *cass::Analyzer::_instance(0);
QMutex cass::Analyzer::_mutex;


cass::Analyzer::Analyzer()
{
  //create the analyzers//
  _analyzer[ccd]          = new CCD::Analysis();
  _analyzer[pnCCD]        = new pnCCD::Analysis();
  _analyzer[Acqiris]      = new ACQIRIS::Analysis();
  _analyzer[MachineData]  = new MachineData::Analysis();

  //set up what analysis is interesting//
  _activeAnalyzers.insert(ccd);
  _activeAnalyzers.insert(pnCCD);
  _activeAnalyzers.insert(Acqiris);
  _activeAnalyzers.insert(MachineData);
}

cass::Analyzer::~Analyzer()
{
  //delete all analyzers
  for (analyzers_t::iterator it=_analyzer.begin() ; it != _analyzer.end(); ++it )
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
  //iterate through all active analyzers and send the cassevent to them//
  for(active_analyzers_t::const_iterator it = _activeAnalyzers.begin(); it != _activeAnalyzers.end();++i)
    (*_analyzer[*it])(cassevent);
}

void cass::Analyzer::loadSettings()
{
  //iterate through all analyzers and load the settings of them//
  for (analyzers_t::iterator it=_analyzer.begin() ; it != _analyzer.end(); ++it )
    it->second->loadSettings();
}

void cass::Analyzer::saveSettings()
{
  //iterate through all analyzers and load the settings of them//
  for (analyzers_t::iterator it=_analyzer.begin() ; it != _analyzer.end(); ++it )
    it->second->saveSettings();
}

