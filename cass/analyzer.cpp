// Copyright (C) 2009 Jochen Küpper
// Copyright (C) 2009, 2010 Lutz Foucar

/**
 * @file analyzer.cpp file contains declaration of class handling the pre
 *                    analyzers
 *
 * @author Lutz Foucar
 */

#include <iostream>
#include <QtCore/QMutexLocker>

#include "analyzer.h"
#include "acqiris_analysis.h"
#include "ccd_analysis.h"
#include "pnccd_analysis.h"
#include "machine_analysis.h"
#include "cass_settings.h"


// ============define static members==============
cass::Analyzer *cass::Analyzer::_instance(0);
QMutex cass::Analyzer::_mutex;

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
//===================================================




cass::Analyzer::Analyzer()
{
  //create the analyzers//
  _analyzer[ccd]          = new CCD::Analysis();
  _analyzer[Acqiris]      = new ACQIRIS::Analysis();
  _analyzer[MachineData]  = new MachineData::Analysis();
  _analyzer[pnCCD]        = new pnCCD::Analysis();
}

cass::Analyzer::~Analyzer()
{
  //delete all analyzers
  for (analyzers_t::iterator it=_analyzer.begin() ; it != _analyzer.end(); ++it )
    delete (it->second);
}


void cass::Analyzer::processEvent(cass::CASSEvent* cassevent)
{
  //use the analyzers to analyze the event//
  //iterate through all active analyzers and send the cassevent to them//
  for(active_analyzers_t::const_iterator it = _activeAnalyzers.begin(); it != _activeAnalyzers.end();++it)
    (*_analyzer[*it])(cassevent);
}

void cass::Analyzer::loadSettings(size_t)
{
  CASSSettings param;
  param.beginGroup("PreAnalyzer");
  //install the requested analyzers//
  if(param.value("useCommercialCCDAnalyzer",true).toBool()) _activeAnalyzers.insert(ccd);         else _activeAnalyzers.erase(ccd);
  if(param.value("useAcqirisAnalyzer",false).toBool())      _activeAnalyzers.insert(Acqiris);     else _activeAnalyzers.erase(Acqiris);
  if(param.value("usepnCCDAnalyzer",true).toBool())         _activeAnalyzers.insert(pnCCD);       else _activeAnalyzers.erase(pnCCD);
  if(param.value("useMachineAnalyzer",false).toBool())      _activeAnalyzers.insert(MachineData); else _activeAnalyzers.erase(MachineData);
  param.endGroup();

  //iterate through all active analyzers and load the settings of them//
  for(active_analyzers_t::const_iterator it = _activeAnalyzers.begin(); it != _activeAnalyzers.end();++it)
    (*_analyzer[*it]).loadSettings();
}

void cass::Analyzer::saveSettings()
{
  //iterate through all active analyzers and load the settings of them//
  for(active_analyzers_t::const_iterator it = _activeAnalyzers.begin(); it != _activeAnalyzers.end();++it)
    (*_analyzer[*it]).saveSettings();
}



// Local Variables:
// coding: utf-8
// mode: C++
// c-file-offsets: ((c . 0) (innamespace . 0))
// c-file-style: "gnu"
// fill-column: 100
// End:
