//Copyright (C) 2012 Lutz Foucar

/**
 * @file cbf_output.cpp output of 2d histograms into the cbf.
 *
 * @author Lutz Foucar
 */

#include <QtCore/QDateTime>

#include <stdint.h>
#include <iomanip>

#include "cbf_output.h"
#include "histogram.h"
#include "cass_event.h"
#include "cass_settings.h"
#include "machine_device.h"
#include "log.h"

using namespace cass;
using namespace std;


pp1500::pp1500(PostProcessors &pp, const PostProcessors::key_t &key, const string& outfilename)
  : PostprocessorBackend(pp,key),
    _basefilename(outfilename)
{
  loadSettings(0);
}

void pp1500::loadSettings(size_t)
{
  CASSSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(QString::fromStdString(_key));
  setupGeneral();
  _pHist = setupDependency("HistName");
  bool ret (setupCondition());
  if (!(ret && _pHist))
    return;
  const HistogramBackend &hist(_pHist->getHist(0));
  if (hist.dimension() != 2)
    throw invalid_argument("pp1500: The histogram that should be written to hdf5 is not a 2d histogram");

  _write = false;
  _hide = true;
  _result = new Histogram0DFloat();
  createHistList(2*cass::NbrOfWorkers,true);
  Log::add(Log::INFO,"PostProcessor '" + _key +
           "' will write histograms '" + _pHist->key() +"' to cbf file with '" +
           _basefilename + "' as basename. Condition is '" + _condition->key() + "'");
}

void pp1500::process(const CASSEvent &evt)
{
  QMutexLocker locker(&_lock);
  const Histogram2DFloat& hist
      (dynamic_cast<const Histogram2DFloat&>((*_pHist)(evt)));
  /** create filename from base filename + event id */
  string filename(_basefilename + "_" + toString(evt.id()) + ".cbf");
  /** create the cbf file */

  /** write histogram data */
  hist.lock.lockForRead();

  //put code here

  /** close file */
  hist.lock.unlock();

  //put code here
}
