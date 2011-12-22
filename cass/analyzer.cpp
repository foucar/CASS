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


using namespace cass;
using namespace std;


// ============define static members==============
Analyzer::shared_pointer Analyzer::_instance;
QMutex Analyzer::_mutex;

Analyzer::shared_pointer Analyzer::instance()
{
  QMutexLocker lock(&_mutex);
  if(!_instance)
    _instance = shared_pointer(new Analyzer());
  return _instance;
}
//===================================================

Analyzer::Analyzer()
{
  _analyzer[ccd] = AnalysisBackend::shared_pointer(new CCD::Analysis());
  _analyzer[pnCCD] =  AnalysisBackend::shared_pointer(new pnCCD::Analysis());
}

void Analyzer::operator()(CASSEvent* cassevent)
{
  active_analyzers_t::const_iterator it (_activeAnalyzers.begin());
  for(; it!=_activeAnalyzers.end();++it)
    (*_analyzer[*it])(cassevent);
}

void Analyzer::loadSettings(size_t)
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

void Analyzer::saveSettings()
{
  /** @todo use for_each and bind here */
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
