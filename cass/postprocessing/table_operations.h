// Copyright (C) 2013 Lutz Foucar

/**
 * @file table_operations.h  contains postprocessors that will operate
 *                           on table like histograms of other postprocessors.
 *
 * @author Lutz Foucar
 */

#ifndef _TABLE_OPERATIONS_H_
#define _TABLE_OPERATIONS_H_

#include "processor.h"
#include "histogram.h"

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
 *           name of postprocessor that contains the table like histogram
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
  virtual void process(const CASSEvent&, HistogramBackend &);

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
 *           name of postprocessor that contains the table like histogram
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
  virtual void process(const CASSEvent&, HistogramBackend &);

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
 *           name of postprocessor that contains the table like histogram
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
  virtual void process(const CASSEvent&, HistogramBackend &);

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
 *           name of postprocessor that contains the table like histogram
 *           subset from. Default is "".
 * @cassttng Processor/\%name\%/{XColumnIndex|YColumnIndex} \n
 *           The index of the column in the table that one wants to have
 *           extracted and put on the x- and y-axis. Please refer to the
 *           Processor description of the Processor that contains the
 *           table to find out what column indizes are available.
 *           Default is "0".
 * @cassttng Processor/\%name\%/{XNbrBins|XLow|XUp|YNbrBins|YLow|YUp}\n
 *           properties of the 2d histogram
 *
 * @author Lutz Foucar
 */
class pp79 : public Processor
{
public:
  /** constructor */
  pp79(const name_t&);

  /** process event */
  virtual void process(const CASSEvent&, HistogramBackend &);

  /** load the settings of the pp */
  virtual void loadSettings(size_t);

protected:
  /** pp containing input table */
  shared_pointer _table;

  /** index of the column that needs to be extracted */
  size_t _xcolIdx;

  /** index of the column that needs to be extracted */
  size_t _ycolIdx;
};




}//end namespace cass
#endif
