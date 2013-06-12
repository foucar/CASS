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

pp72::pp72(const name_t &name)
  : PostProcessor(name)
{
  loadSettings(0);
}

void pp72::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("PostProcessor");
  s.beginGroup(QString::fromStdString(name()));
  setupGeneral();
  _table = setupDependency("TableName");
  bool ret (setupCondition());
  if (!(ret && _table))
    return;
  _colIdx = s.value("ColumnIndex",0).toUInt();

  size_t maxIdx(_table->result().axis()[HistogramBackend::xAxis].size());
  if (_colIdx >= maxIdx)
    throw runtime_error("pp72::loadSettings(): '" + name() + "' The requested " +
                        "column index '" + toString(_colIdx) + " 'exeeds the " +
                        "maximum possible index value '" + toString(maxIdx) + "'");

  createHistList(tr1::shared_ptr<Histogram1DFloat>(new Histogram1DFloat()));
  Log::add(Log::INFO,"PostProcessor '" + name() +
           "' retrieves column with index '" + toString(_colIdx) +
           "' from table " + _table->name() + "' .Condition on postprocessor '" +
           _condition->name() + "'");
}

void pp72::process(const CASSEvent& evt, HistogramBackend &res)
{
  const Histogram2DFloat& table
      (dynamic_cast<const Histogram2DFloat&>(_table->result(evt.id())));
  const HistogramFloatBase::storage_t &tableContents(table.memory());
  Histogram1DFloat &col(dynamic_cast<Histogram1DFloat&>(res));

  QReadLocker lock(&table.lock);

  col.clearline();

  const size_t nCols(table.axis()[HistogramBackend::xAxis].size());
  const size_t nRows(table.axis()[HistogramBackend::yAxis].size());

  for (size_t row=0; row < nRows; ++row)
    col.append(tableContents[row*nCols + _colIdx]);

  col.nbrOfFills()=1;
}




// ***  pp 73 returns subset of table with condition on rows ***

pp73::pp73(const name_t &name)
  : PostProcessor(name)
{
  loadSettings(0);
}

void pp73::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("PostProcessor");
  s.beginGroup(QString::fromStdString(name()));
  setupGeneral();
  _table = setupDependency("TableName");
  bool ret (setupCondition());
  if (!(ret && _table))
    return;
  _colIdx = s.value("ColumnIndex",0).toUInt();
  _bounds = make_pair(s.value("LowerBound",0.f).toFloat(),
                      s.value("UpperBound",1.f).toFloat());

  size_t tableSize(_table->result().axis()[HistogramBackend::xAxis].size());
  if (_colIdx >= tableSize)
    throw runtime_error("pp73::loadSettings(): '" + name() + "' The requested " +
                        "column index '" + toString(_colIdx) + " 'exeeds the " +
                        "maximum possible index value '" + toString(tableSize) + "'");

  createHistList(tr1::shared_ptr<Histogram2DFloat>(new Histogram2DFloat(tableSize)));
  Log::add(Log::INFO,"PostProcessor '" + name() +
           "' retrieves subset of table in '" + _table->name() + "'. UpperBound '" +
           toString(_bounds.first) + "' LowerBound '" + toString(_bounds.second) +
           "' on values in column with index '" + toString(_colIdx) +
           "'. Condition on postprocessor '" + _condition->name() + "'");
}

void pp73::process(const CASSEvent& evt, HistogramBackend &res)
{
  const Histogram2DFloat& table
      (dynamic_cast<const Histogram2DFloat&>(_table->result(evt.id())));
  const HistogramFloatBase::storage_t &tableContents(table.memory());
  Histogram2DFloat &result(dynamic_cast<Histogram2DFloat&>(res));

  QReadLocker lock(&table.lock);

  HistogramFloatBase::storage_t::const_iterator tableIt(tableContents.begin());

  result.clearTable();

  const size_t nRows(table.axis()[HistogramBackend::yAxis].size());
  const size_t nCols(table.axis()[HistogramBackend::xAxis].size());

  HistogramFloatBase::storage_t rows;
  for (size_t rowIdx=0; rowIdx < nRows; ++rowIdx)
  {
    if(_bounds.first <= tableIt[_colIdx] && tableIt[_colIdx] < _bounds.second)
      rows.insert(rows.end(),tableIt,tableIt+nCols);
    tableIt += nCols;
  }
  result.appendRows(rows);

  result.nbrOfFills()=1;
}
