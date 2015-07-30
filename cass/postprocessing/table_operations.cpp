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
#include "convenience_functions.h"

using namespace cass;
using namespace std;



// ***  pp 72 returns column of a table ***

pp72::pp72(const name_t &name)
  : Processor(name)
{
  loadSettings(0);
}

void pp72::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("Processor");
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
  Log::add(Log::INFO,"Processor '" + name() +
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
  : Processor(name)
{
  loadSettings(0);
}

void pp73::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("Processor");
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
  Log::add(Log::INFO,"Processor '" + name() +
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




// *** pp 74 retrieve a specific value of a specific row ***

pp74::pp74(const name_t &name)
  : Processor(name)
{
  loadSettings(0);
}

void pp74::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("Processor");
  s.beginGroup(QString::fromStdString(name()));
  setupGeneral();
  _table = setupDependency("TableName");
  bool ret (setupCondition());
  if (!(ret && _table))
    return;
  _colIdx = s.value("ColumnIndex",0).toUInt();
  _rowIdx = s.value("RowIndex",0).toUInt();

  size_t tableSize(_table->result().axis()[HistogramBackend::xAxis].size());
  if (_colIdx >= tableSize)
    throw runtime_error("pp73::loadSettings(): '" + name() + "' The requested " +
                        "column index '" + toString(_colIdx) + " 'exeeds the " +
                        "maximum possible index value '" + toString(tableSize) + "'");

  createHistList(tr1::shared_ptr<Histogram0DFloat>(new Histogram0DFloat()));

  Log::add(Log::INFO,"Processor '" + name() +
           "' retrieves the value of row '" + toString(_rowIdx) +
           "' and column '" + toString(_colIdx) + "' from table '" +
           _table->name() + "'. Condition on postprocessor '" +
           _condition->name() + "'");
}

void pp74::process(const CASSEvent& evt, HistogramBackend &res)
{
  const Histogram2DFloat& table
      (dynamic_cast<const Histogram2DFloat&>(_table->result(evt.id())));
  const HistogramFloatBase::storage_t &tableContents(table.memory());
  Histogram0DFloat &result(dynamic_cast<Histogram0DFloat&>(res));

  QReadLocker lock(&table.lock);

  result.clear();

  const size_t nRows(table.axis()[HistogramBackend::yAxis].size());
  const size_t nCols(table.axis()[HistogramBackend::xAxis].size());

  if (_rowIdx >= nRows)
    throw invalid_argument("pp74::process(): '" + name() + "' The requested row index '" +
                           toString(_rowIdx) + "' is too big for a table with '" +
                           toString(nRows) + "' rows.");

  const HistogramFloatBase::value_t val(tableContents[_rowIdx * nCols + _colIdx]);

  result.fill(val);
  result.nbrOfFills()=1;
}





// ***  pp 79 generates a 2d histogram from 2 columns***

pp79::pp79(const name_t &name)
  : Processor(name)
{
  loadSettings(0);
}

void pp79::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("Processor");
  s.beginGroup(QString::fromStdString(name()));
  setupGeneral();
  _table = setupDependency("TableName");
  bool ret (setupCondition());
  if (!(ret && _table))
    return;
  _xcolIdx = s.value("XColumnIndex",0).toUInt();
  _ycolIdx = s.value("YColumnIndex",0).toUInt();

  size_t maxIdx(_table->result().axis()[HistogramBackend::xAxis].size());
  if (_xcolIdx >= maxIdx)
    throw runtime_error("pp72::loadSettings(): '" + name() + "' The requested " +
                        "x column index '" + toString(_xcolIdx) + " 'exeeds the " +
                        "maximum possible index value '" + toString(maxIdx) + "'");
  if (_ycolIdx >= maxIdx)
    throw runtime_error("pp72::loadSettings(): '" + name() + "' The requested " +
                        "y column index '" + toString(_ycolIdx) + " 'exeeds the " +
                        "maximum possible index value '" + toString(maxIdx) + "'");

  createHistList(set2DHist(name()));
  Log::add(Log::INFO,"Processor '" + name() +
           "' X column index '" + toString(_xcolIdx) +
           "' Y column index '" + toString(_ycolIdx) +
           "' Table " + _table->name() + "' .Condition on postprocessor '" +
           _condition->name() + "'");
}

void pp79::process(const CASSEvent& evt, HistogramBackend &res)
{
  const Histogram2DFloat& table
      (dynamic_cast<const Histogram2DFloat&>(_table->result(evt.id())));
  const HistogramFloatBase::storage_t &tableContents(table.memory());
  Histogram2DFloat &result(dynamic_cast<Histogram2DFloat&>(res));

  QReadLocker lock(&table.lock);

  result.clear();

  const size_t nCols(table.axis()[HistogramBackend::xAxis].size());
  const size_t nRows(table.axis()[HistogramBackend::yAxis].size());

  for (size_t row=0; row < nRows; ++row)
    result.fill(tableContents[row*nCols + _xcolIdx],
                tableContents[row*nCols + _ycolIdx]);

  result.nbrOfFills()=1;
}



