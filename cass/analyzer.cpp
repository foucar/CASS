// Copytight (C) 2009 Jochen Küpper

#include <iostream>

#include "analyzer.h"
#include "remi_analysis.h"
#include "vmi_analysis.h"
#include "pnccd_analysis.h"
#include "machine_analysis.h"
#include "database.h"

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

