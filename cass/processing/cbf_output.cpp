//Copyright (C) 2012 Lutz Foucar

/**
 * @file cbf_output.cpp output of 2d histograms into the cbf.
 *
 * @author Lutz Foucar
 */

#include <QtCore/QDateTime>

#include <stdint.h>
#include <iomanip>
#include <fstream>

#include "cbf_output.h"

#include "cass_event.h"
#include "cass_settings.h"
#include "log.h"
#include "convenience_functions.h"
#include "cbf_handle.hpp"

using namespace cass;
using namespace std;



pp1500::pp1500(const name_t &name)
  : Processor(name)
{
  loadSettings(0);
}

void pp1500::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("Processor");
  s.beginGroup(QString::fromStdString(name()));
  setupGeneral();
  bool allDepsAreThere(true);

  string hName(s.value("Name","Unknown").toString().toStdString());
  if (hName != "Unknown")
  {
    _pHist = setupDependency("",hName);
    allDepsAreThere = _pHist && allDepsAreThere;
  }

  string summaryName(s.value("SummaryName","Unknown").toString().toStdString());
  if (summaryName != "Unknown")
  {
    _summaryHist = setupDependency("",summaryName);
    allDepsAreThere = _summaryHist && allDepsAreThere;
  }

  bool ret (setupCondition());
  if (!(ret && allDepsAreThere))
    return;

  if (_pHist)
  {
    const result_t &hist(_pHist->result());
    if (hist.dim() != 2)
      throw invalid_argument("pp1500: The histogram '" + _pHist->name()
                             + "' is not a 2d histogram");
  }

  if (_summaryHist)
  {
    const result_t &sHist(_summaryHist->result());
    if (sHist.dim() != 2)
      throw invalid_argument("pp1500: The summary histogram '" + _summaryHist->name()
                             + "'is not a 2d histogram");
  }

  /** when requested add the first subdir to the filename and make sure that the
   *  directory exists.
   */
  _basefilename = s.value("FileBaseName",QString::fromStdString(_basefilename)).toString().toStdString();
  _maxFilePerSubDir = s.value("MaximumNbrFilesPerDir",-1).toInt();
  _filecounter = 0;
  if(_maxFilePerSubDir != -1)
    _basefilename = AlphaCounter::intializeDir(_basefilename);

  _hide = true;
  Log::add(Log::INFO,"Processor '" + name() +
           "' CBF Writer: " +
           "Basename '" + _basefilename + "'" +
           (_pHist? " Histname '" + _pHist->name() +"'" :"") +
           (_summaryHist? " Summary Histogram '" + _summaryHist->name() + "'":"") +
           ". Condition is '" + _condition->name() + "'");
}

const Processor::result_t &pp1500::result(const CASSEvent::id_t)
{
  throw logic_error("pp1500::result: '"+name()+"' should never be called");
}

void pp1500::processEvent(const CASSEvent &evt)
{
  /** return if there is no histogram to be written */
  if (!_pHist)
    return;

  /** return if the condition for this pp is false */
  if (!_condition->result(evt.id()).isTrue())
    return;

  QMutexLocker locker(&_lock);

  /** increment subdir in filename when they should be distributed and the
   *  counter exeeded the maximum amount of files per subdir
   */
  if (_maxFilePerSubDir == _filecounter)
  {
    _filecounter = 0;
    _basefilename = AlphaCounter::increaseDirCounter(_basefilename);
  }
  ++_filecounter;

  const result_t& hist(_pHist->result(evt.id()));

  QReadLocker lock(&hist.lock);

  /** create filename from base filename + event id */
  string filename(_basefilename + "_" + toString(evt.id()) + ".cbf");
  CBF::write(filename,hist.begin(),hist.shape());
}

void pp1500::aboutToQuit()
{
  /** return if there is no summary to be written */
  if (!_summaryHist)
    return;

  QMutexLocker locker(&_lock);
  const result_t& sHist(_summaryHist->result());
  QReadLocker lock(&sHist.lock);

  /** create filename from base filename, but first remove subdir from filename
   *   when they should be distributed
   */
  if (_maxFilePerSubDir != -1)
    _basefilename = AlphaCounter::removeAlphaSubdir(_basefilename);
  string filename(_basefilename + "_Summary.cbf");
  CBF::write(filename,sHist.begin(),sHist.shape());
}
