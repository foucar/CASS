// Copyright (C) 2013 Lutz Foucar

/**
 * @file table_operations.h  contains processors that will operate
 *                           on table like histograms of other processors.
 *
 * @author Lutz Foucar
 */

#ifndef _TABLE_OPERATIONS_H_
#define _TABLE_OPERATIONS_H_

#include <tr1/functional>

#include "processor.h"
#include "result.hpp"

namespace cass
{

/** get specific column from table like histogram
 *
 * @PPList "72": get specific column from table like histogram
 *
 * Will copy all contents of the input tables user specified column into a
 * 1d histogram.
 *
 * @see Processor for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng Processor/\%name\%/{TableName} \n
 *           name of processor that contains the table like histogram
 *           subset from. Default is "".
 * @cassttng Processor/\%name\%/{ColumnIndex} \n
 *           The index of the column in the table that one wants to have
 *           extracted. Please refer to the Processor description of the
 *           Processor that  contains the table to find out what column
 *           indizes are available.
 *           Default is "0".
 *
 * @author Lutz Foucar
 */
class pp72 : public Processor
{
public:
  /** constructor */
  pp72(const name_t&);

  /** process event */
  virtual void process(const CASSEvent&, result_t&);

  /** load the settings of the pp */
  virtual void loadSettings(size_t);

protected:
  /** pp containing input table */
  shared_pointer _table;

  /** index of the column that needs to be extracted */
  size_t _colIdx;
};




/** get all rows with condition on a column
 *
 * @PPList "73": get all rows with condition on a column
 *
 * Will copy all rows of the input table where a user specified column value
 * satisfies the set condition. Therefore the value has to be greater or equal
 * to the lower bound and smaller than the upper bound.
 *
 * @see Processor for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng Processor/\%name\%/{TableName} \n
 *           name of processor that contains the table like histogram
 *           subset from. Default is "".
 * @cassttng Processor/\%name\%/{ColumnIndex} \n
 *           The index of the column in the table that one wants to use the
 *           condition on. Please refer to the Processor description of the
 *           Processor that  contains the table to find out what column
 *           indizes are available.
 *           Default is "0".
 * @cassttng Processor/\%name\%/{UpperBound|LowerBound} \n
 *           Upper- and Lower Bound of the boundaries that the column value will
 *           be checked for.
 *
 * @author Lutz Foucar
 */
class pp73 : public Processor
{
public:
  /** constructor */
  pp73(const name_t&);

  /** process event */
  virtual void process(const CASSEvent&, result_t&);

  /** load the settings of the pp */
  virtual void loadSettings(size_t);

protected:
  /** pp containing input table */
  shared_pointer _table;

  /** index of the column that will be checked for */
  size_t _colIdx;

  /** the boundaries for the condition */
  std::pair<float,float> _bounds;
};






/** retrieve a specific value of a specific row
 *
 * @PPList "74": retrieve a specific value of a specific row
 *
 *
 * @see Processor for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng Processor/\%name\%/{TableName} \n
 *           name of processor that contains the table like histogram
 *           subset from. Default is "".
 * @cassttng Processor/\%name\%/{RowIndex} \n
 *           The index of the row in the table that contains the requested value.
 *           Default is "0"
 * @cassttng Processor/\%name\%/{ColumnIndex} \n
 *           The index of the column in the table that contains the requested
 *           value. Please refer to the Processor description of the
 *           Processor that contains the table to find out what column
 *           indizes are available.
 *           Default is "0".
 *
 * @author Lutz Foucar
 */
class pp74 : public Processor
{
public:
  /** constructor */
  pp74(const name_t&);

  /** process event */
  virtual void process(const CASSEvent&, result_t&);

  /** load the settings of the pp */
  virtual void loadSettings(size_t);

protected:
  /** pp containing input table */
  shared_pointer _table;

  /** index of the column */
  size_t _colIdx;

  /** the index of the row */
  size_t _rowIdx;
};




/** generate a 2d Histogram from values of 2 columns of a table
 *
 * @PPList "79": generate a 2d Histogram from values of 2 columns of a table
 *
 * @see Processor for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng Processor/\%name\%/{TableName} \n
 *           name of processor that contains the table like histogram
 *           subset from. Default is "".
 * @cassttng Processor/\%name\%/{XColumnIndex|YColumnIndex} \n
 *           The index of the column in the table that one wants to have
 *           extracted and put on the x- and y-axis. Please refer to the
 *           Processor description of the Processor that contains the
 *           table to find out what column indizes are available.
 *           Default is "0".
 * @cassttng Processor/\%name\%/{WeightColumnIndex} \n
 *           Optional index of the column that will be used for the weights
 *           when histogramming the x and y values. If negative number is given
 *           1 will be used as weight. Default is -1
 * @cassttng Processor/\%name\%/{XNbrBins|XLow|XUp|YNbrBins|YLow|YUp}\n
 *           properties of the 2d result
 *
 * @author Lutz Foucar
 */
class pp79 : public Processor
{
public:
  /** constructor */
  pp79(const name_t&);

  /** process event */
  virtual void process(const CASSEvent&, result_t&);

  /** load the settings of the pp */
  virtual void loadSettings(size_t);

protected:
  /** define the function to return the weight value */
  typedef std::tr1::function<result_t::value_t(result_t::const_iterator)> func_t;

  /** returns the weight value from a table
   *
   * @param the weight
   * @param tableIt the iterator that points to the row of the table to exract
   *                the weight from
   */
  func_t::result_type weightFromTable(func_t::argument_type tableIt);

  /** returns a 1
   *
   * @param 1
   * @param unused unused parameter
   */
  func_t::result_type constantWeight(func_t::argument_type unused);


protected:
  /** pp containing input table */
  shared_pointer _table;

  /** index of the column with the x-values that needs to be extracted */
  size_t _xcolIdx;

  /** index of the column with the y-values that needs to be extracted */
  size_t _ycolIdx;

  /** index of the column with the weights that needs to be exracted */
  int _weightcolIdx;

  /** the function to return the weight */
  func_t _getWeight;
};




}//end namespace cass
#endif
