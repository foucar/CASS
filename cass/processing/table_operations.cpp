// Copyright (C) 2013 Lutz Foucar

/**
 * @file table_operations.cpp contains processors that will operate
 *                            on table like histograms of other processors.
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

  size_t maxIdx(_table->result().axis(result_t::xAxis).nBins);
  if (_colIdx >= maxIdx)
    throw runtime_error("pp72::loadSettings(): '" + name() + "' The requested " +
                        "column index '" + toString(_colIdx) + " 'exeeds the " +
                        "maximum possible index value '" + toString(maxIdx) + "'");

  createHistList(result_t::shared_pointer(new result_t(0)));
  Log::add(Log::INFO,"Processor '" + name() +
           "' retrieves column with index '" + toString(_colIdx) +
           "' from table " + _table->name() + "' .Condition on processor '" +
           _condition->name() + "'");
}

void pp72::process(const CASSEvent& evt, result_t &result)
{
  const result_t& table(_table->result(evt.id()));
  QReadLocker lock(&table.lock);

  result.reset();

  const size_t nCols(table.shape().first);
  const size_t nRows(table.shape().second);

  for (size_t row=0; row < nRows; ++row)
    result.append(table[row*nCols + _colIdx]);

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

  size_t tableSize(_table->result().axis(result_t::xAxis).nBins);
  if (_colIdx >= tableSize)
    throw runtime_error("pp73::loadSettings(): '" + name() + "' The requested " +
                        "column index '" + toString(_colIdx) + " 'exeeds the " +
                        "maximum possible index value '" + toString(tableSize) + "'");

  createHistList(result_t::shared_pointer(new result_t(tableSize,0)));
  Log::add(Log::INFO,"Processor '" + name() +
           "' retrieves subset of table in '" + _table->name() + "'. UpperBound '" +
           toString(_bounds.first) + "' LowerBound '" + toString(_bounds.second) +
           "' on values in column with index '" + toString(_colIdx) +
           "'. Condition on processor '" + _condition->name() + "'");
}

void pp73::process(const CASSEvent& evt, result_t &result)
{
  const result_t& table(_table->result(evt.id()));
  QReadLocker lock(&table.lock);

  result_t::const_iterator tableIt(table.begin());

  result.resetTable();

  const size_t nCols(table.shape().first);
  const size_t nRows(table.shape().second);

  result_t::storage_t rows;
  for (size_t rowIdx=0; rowIdx < nRows; ++rowIdx)
  {
    if(_bounds.first <= tableIt[_colIdx] && tableIt[_colIdx] < _bounds.second)
      rows.insert(rows.end(),tableIt,tableIt+nCols);
    tableIt += nCols;
  }
  result.appendRows(rows);

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

  size_t tableSize(_table->result().shape().first);
  if (_colIdx >= tableSize)
    throw runtime_error("pp73::loadSettings(): '" + name() + "' The requested " +
                        "column index '" + toString(_colIdx) + " 'exeeds the " +
                        "maximum possible index value '" + toString(tableSize) + "'");

  createHistList(result_t::shared_pointer(new result_t()));

  Log::add(Log::INFO,"Processor '" + name() +
           "' retrieves the value of row '" + toString(_rowIdx) +
           "' and column '" + toString(_colIdx) + "' from table '" +
           _table->name() + "'. Condition on processor '" +
           _condition->name() + "'");
}

void pp74::process(const CASSEvent& evt, result_t &result)
{
  const result_t& table(_table->result(evt.id()));
  QReadLocker lock(&table.lock);

  const size_t nCols(table.shape().first);
  const size_t nRows(table.shape().second);

  if (_rowIdx >= nRows)
    throw invalid_argument("pp74::process(): '" + name() + "' The requested row index '" +
                           toString(_rowIdx) + "' is too big for a table with '" +
                           toString(nRows) + "' rows.");

  result.setValue(table[_rowIdx * nCols + _colIdx]);
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
  _weightcolIdx = s.value("WeightColumnIndex",-1).toInt();

  size_t maxIdx(_table->result().shape().first);
  if (_xcolIdx >= maxIdx)
    throw runtime_error("pp79::loadSettings(): '" + name() + "' The requested " +
                        "x column index '" + toString(_xcolIdx) + " 'exeeds the " +
                        "maximum possible index value '" + toString(maxIdx) + "'");
  if (_ycolIdx >= maxIdx)
    throw runtime_error("pp79::loadSettings(): '" + name() + "' The requested " +
                        "y column index '" + toString(_ycolIdx) + " 'exeeds the " +
                        "maximum possible index value '" + toString(maxIdx) + "'");
  if (!(_weightcolIdx < 0) && _weightcolIdx >= static_cast<int>(maxIdx))
    throw runtime_error("pp79::loadSettings(): '" + name() + "' The requested " +
                        "weight column index '" + toString(_weightcolIdx) + " 'exeeds the " +
                        "maximum possible index value '" + toString(maxIdx) + "'");
  if (_weightcolIdx < 0)
    _getWeight = bind(&pp79::constantWeight, this, tr1::placeholders::_1);
  else
  {
    _getWeight = bind(&pp79::weightFromTable, this, tr1::placeholders::_1);
  }

  createHistList(set2DHist(name()));
  Log::add(Log::INFO,"Processor '" + name() +
           "' X column index '" + toString(_xcolIdx) +
           "' Y column index '" + toString(_ycolIdx) +
           "' Table " + _table->name() + "' .Condition on processor '" +
           _condition->name() + "'");
}

pp79::func_t::result_type pp79::weightFromTable(func_t::argument_type tableIt)
{
  return tableIt[_weightcolIdx];
}

pp79::func_t::result_type pp79::constantWeight(func_t::argument_type)
{
  return abs(_weightcolIdx);
}

void pp79::process(const CASSEvent& evt, result_t &result)
{
  const result_t& table(_table->result(evt.id()));
  QReadLocker lock(&table.lock);

  const size_t nCols(table.shape().first);
  const size_t nRows(table.shape().second);

  result_t::const_iterator tableIt(table.begin());
  for (size_t row=0; row < nRows; ++row)
  {
    const int pixCol(tableIt[_xcolIdx]);
    const int pixRow(tableIt[_ycolIdx]);
    result.histogram(make_pair(pixCol,pixRow),_getWeight(tableIt));
    tableIt += nCols;
  }
}



