// Copyright (C) 2009 Jochen Küpper
// Copyright (C) 2009, 2010 Lutz Foucar

/**
 * @file analyzer.cpp file contains definition of class handling the pre
 *                    analyzers
 *
 * @author Lutz Foucar
 */

#include <iostream>
#include <QtCore/QMutexLocker>

#include "analyzer.h"
#include "ccd_analysis.h"
#include "pnccd_analysis.h"
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
  _analyzer[ccd]          = new CCD::Analysis();
  _analyzer[pnCCD]        = new pnCCD::Analysis();
}

cass::Analyzer::~Analyzer()
{
  analyzers_t::iterator it (_analyzer.begin());
  for (; it != _analyzer.end(); ++it )
    delete (it->second);
}

void cass::Analyzer::processEvent(cass::CASSEvent* cassevent)
{
  active_analyzers_t::const_iterator it (_activeAnalyzers.begin());
  for(; it!=_activeAnalyzers.end();++it)
    (*_analyzer[*it])(cassevent);
}

void cass::Analyzer::loadSettings(size_t)
{
  CASSSettings settings;
  settings.beginGroup("PreAnalyzer");
  if (settings.value("useCommercialCCDAnalyzer",true).toBool())
    _activeAnalyzers.insert(ccd);         else _activeAnalyzers.erase(ccd);
  if (settings.value("usepnCCDAnalyzer",true).toBool())
    _activeAnalyzers.insert(pnCCD);       else _activeAnalyzers.erase(pnCCD);
  active_analyzers_t::const_iterator it (_activeAnalyzers.begin());
  for(; it != _activeAnalyzers.end();++it)
    (*_analyzer[*it]).loadSettings();
}

void cass::Analyzer::saveSettings()
{
  active_analyzers_t::const_iterator it (_activeAnalyzers.begin());
  for(; it != _activeAnalyzers.end();++it)
    (*_analyzer[*it]).saveSettings();
}



// Local Variables:
// coding: utf-8
// mode: C++
// c-file-offsets: ((c . 0) (innamespace . 0))
// c-file-style: "gnu"
// fill-column: 100
// End:
