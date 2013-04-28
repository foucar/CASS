//Copyright (C) 2013 Lutz Foucar

/**
 * @file cbf_output.cpp output of 2d histograms into the cbf.
 *
 * @author Lutz Foucar
 */

#include <algorithm>
#include <functional>
#include <numeric>

#include "autocorrelation.h"
#include "histogram.h"
#include "cass_event.h"
//#include "cass_settings.h"
#include "log.h"

using namespace cass;
using namespace std;


pp310::pp310(PostProcessors &pp, const PostProcessors::key_t &key)
  : PostprocessorBackend(pp,key)
{
  loadSettings(0);
}

void pp310::loadSettings(size_t)
{
//  CASSSettings s;
//  s.beginGroup("PostProcessor");
//  s.beginGroup(QString::fromStdString(_key));
  setupGeneral();
  _hist = setupDependency("HistName");
  bool ret (setupCondition());
  if (!(ret && _hist))
    return;
  setup(dynamic_cast<const Histogram2DFloat&>(_hist->getHist(0)));

  Log::add(Log::INFO,"PostProcessor '" + _key +
           "' will calculate the autocorrelation of '" + _hist->key() +
           "'. Condition is '" + _condition->key() + "'");
}

void pp310::histogramsChanged(const HistogramBackend* in)
{
  QWriteLocker lock(&_histLock);
  /** return when there is no incomming histogram */
  if(!in)
    return;
  /** return when the incomming histogram is not a direct dependant */
  if (find(_dependencies.begin(),_dependencies.end(),in->key()) == _dependencies.end())
    return;

  setup(dynamic_cast<const Histogram2DFloat&>(*in));

  /** notify all pp that depend on us that our histograms have changed */
  PostProcessors::keyList_t dependands (_pp.find_dependant(_key));
  PostProcessors::keyList_t::iterator it (dependands.begin());
  for (; it != dependands.end(); ++it)
    _pp.getPostProcessor(*it).histogramsChanged(_result);
  Log::add(Log::VERBOSEINFO,"Postprocessor '" + _key +
             "': histograms changed => delete existing histo" +
             " and create new one from input");
}

void pp310::setup(const Histogram2DFloat &srcImageHist)
{
  if (srcImageHist.dimension() != 2)
    throw invalid_argument("pp310:setup: '" + _key +
                           "' The input histogram is not a 2d histogram");
  _result = srcImageHist.clone();
  createHistList(2*cass::NbrOfWorkers,true);
}

void pp310::process(const CASSEvent &evt)
{
  typedef HistogramFloatBase::storage_t::const_iterator const_iterator;
  const Histogram2DFloat& hist
      (dynamic_cast<const Histogram2DFloat&>((*_hist)(evt)));
  const Histogram2DFloat::storage_t& histdata( hist.memory() );

  Histogram2DFloat &result(dynamic_cast<Histogram2DFloat&>(*_result));
  Histogram2DFloat::storage_t& resultdata( result.memory() );

  hist.lock.lockForRead();
  result.lock.lockForWrite();

  size_t nCols(hist.axis()[HistogramBackend::xAxis].size());
  size_t nRows(hist.axis()[HistogramBackend::yAxis].size());

  HistogramFloatBase::storage_t row_buffer(nCols);
  /** go through each row (radius) of input */
  for (size_t row(0); row < nRows; ++row)
  {
    const_iterator rowStart(histdata.begin() + row*nCols);
    const_iterator rowEnd(histdata.begin() + row*nCols + nCols);
    /** go through each col (phi) of input */
    for (size_t col(0); col < nCols; ++col)
    {
      /** rotate the input to get the current d_phi and put result into buffer */
      rotate_copy(rowStart, rowStart+col, rowEnd, row_buffer.begin());

      /** multiply the original phi with the rotated phi and put result into buffer */
      transform(rowStart,rowEnd,row_buffer.begin(),row_buffer.begin(),multiplies<float>());

      /** sum up the contents of the buffer which gives the correlation value */
      const float autocorval(accumulate(row_buffer.begin(),row_buffer.end(),0));

      /** put value into correlatoin value at position radius, d_phi */
      resultdata[row*nCols + col] = autocorval;
    }
  }
  result.lock.unlock();
  hist.lock.unlock();
}
