// Copyright (C) 2009 Jochen Küpper
// Copyright (C) 2009,2010 Lutz Foucar

#include <iostream>
#include <QtCore/QMutexLocker>
#include "analyzer.h"
#include "acqiris_analysis.h"
#include "ccd_analysis.h"
#include "pnccd_analysis.h"
#include "machine_analysis.h"


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


//void cass::AnalysisParameter::load()
//{
//  //sync before loading//
//  sync();
//  _useCCD     = value("useCommercialCCDAnalyzer",true).toBool();
//  _useAcqiris = value("useAcqirisAnalyzer",false).toBool();
//  _useMachine = value("useMachineAnalyzer",true).toBool();
//  _usepnCCD   = value("usepnCCDAnalyzer",true).toBool();
//  std::cout<<std::boolalpha<<"useCommercialCCDAnalyzer "<<_useCCD<<std::endl;
//}
//
//void cass::AnalysisParameter::save()
//{
//  setValue("useComercialCCDAnalyzer",_useCCD);
//  setValue("useAcqirisAnalyzer",_useAcqiris);
//  setValue("useMachineAnalyzer",_useMachine);
//  setValue("usepnCCDAnalyzer",_usepnCCD);
//}





cass::Analyzer::Analyzer()
{
  //create the analyzers//
  _analyzer[ccd]          = new CCD::Analysis();
  _analyzer[Acqiris]      = new ACQIRIS::Analysis();
  _analyzer[MachineData]  = new MachineData::Analysis();
  _analyzer[pnCCD]        = new pnCCD::Analysis();
  // look what analysis is interesting to the user
  loadSettings(0);
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
  QSettings param;
  param.beginGroup("PreAnalyzer");
  //install the requested analyzers//
  if(param.value("useCommercialCCDAnalyzer",true).toBool()) _activeAnalyzers.insert(ccd);         else _activeAnalyzers.erase(ccd);
  if(param.value("useAcqirisAnalyzer",false).toBool())      _activeAnalyzers.insert(Acqiris);     else _activeAnalyzers.erase(Acqiris);
  if(param.value("usepnCCDAnalyzer",true).toBool())         _activeAnalyzers.insert(pnCCD);       else _activeAnalyzers.erase(pnCCD);
  if(param.value("useMachineAnalyzer",false).toBool())      _activeAnalyzers.insert(MachineData); else _activeAnalyzers.erase(MachineData);
  param.endGroup();

  //iterate through all analyzers and load the settings of them//
  for (analyzers_t::iterator it=_analyzer.begin();it != _analyzer.end();++it )
    it->second->loadSettings();
}

void cass::Analyzer::saveSettings()
{
  //  //save the AnalyzerParameters//
  //  _param.save();
  //iterate through all analyzers and load the settings of them//
  for (analyzers_t::iterator it=_analyzer.begin() ; it != _analyzer.end(); ++it )
    it->second->saveSettings();
}



// Local Variables:
// coding: utf-8
// mode: C++
// c-file-offsets: ((c . 0) (innamespace . 0))
// c-file-style: "gnu"
// fill-column: 100
// End:
