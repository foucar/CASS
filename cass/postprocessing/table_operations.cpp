// Copyright (C) 2013 Lutz Foucar

/**
 * @file table_operations.cpp contains postprocessors that will operate
 *                            on table like histograms of other postprocessors.
 *
 * @author Lutz Foucar
 */

#include "table_operations.h"

#include "log.h"
#include "cass_settings.h"

using namespace cass;
using namespace std;



// ***  pp 72 returns column of a table ***

pp72::pp72(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key)
{
  loadSettings(0);
}

void pp72::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("PostProcessor");
  s.beginGroup(QString::fromStdString(_key));
  setupGeneral();
  _table = setupDependency("TableName");
  bool ret (setupCondition());
  if (!(ret && _table))
    return;
  _colIdx = s.value("ColumnIndex",0).toUInt();

  size_t maxIdx(_table->getHist(0).axis()[HistogramBackend::xAxis].size());
  if (_colIdx >= maxIdx)
    throw runtime_error("pp72::loadSettings(): '" + _key + "' The requested " +
                        "column index '" + toString(_colIdx) + " 'exeeds the " +
                        "maximum possible index value '" + toString(maxIdx) + "'");

  _result = new Histogram1DFloat();
  createHistList(2*cass::NbrOfWorkers);
  Log::add(Log::INFO,"PostProcessor '" + _key +
           "' retrieves column with index '" + toString(_colIdx) +
           "' from table " + _table->key() + "' .Condition on postprocessor '" +
           _condition->key() + "'");
}

void pp72::process(const cass::CASSEvent& evt)
{
  const Histogram2DFloat& table
      (dynamic_cast<const Histogram2DFloat&>((*_table)(evt)));
  const HistogramFloatBase::storage_t &tableContents(table.memory());
  Histogram1DFloat &col(dynamic_cast<Histogram1DFloat&>(*_result));

  table.lock.lockForRead();
  col.lock.lockForWrite();

  col.clearline();

  size_t nCols(table.axis()[HistogramBackend::xAxis].size());
  size_t nRows(table.axis()[HistogramBackend::yAxis].size());

  for (size_t row=0; row < nRows; ++row)
    col.append(tableContents[row*nCols + _colIdx]);

  col.nbrOfFills()=1;
  col.lock.unlock();
  table.lock.unlock();
}


