// Copyright (C) 2013 Lutz Foucar

/**
 * @file table_operations.h  contains postprocessors that will operate
 *                           on table like histograms of other postprocessors.
 *
 * @author Lutz Foucar
 */

#ifndef _TABLE_OPERATIONS_H_
#define _TABLE_OPERATIONS_H_

#include "backend.h"
#include "histogram.h"

namespace cass
{

/** get specific column from table like histogram
 *
 * Will copy all contents of the input tables user specified column into a
 * 1d histogram.
 *
 * @see PostprocessorBackend for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng PostProcessor/\%name\%/{TableName} \n
 *           name of postprocessor that contains the table like histogram
 *           subset from. Default is "".
 * @cassttng PostProcessor/\%name\%/{ColumnIndex} \n
 *           The index of the column in the table that one wants to have
 *           extracted. Please refer to the PostProcessor description of the
 *           PostProcessor that  contains the table to find out what column
 *           indizes are available.
 *           Default is "0".
 *
 * @author Lutz Foucar
 */
class pp72 : public PostprocessorBackend
{
public:
  /** constructor */
  pp72(PostProcessors& hist, const PostProcessors::key_t&);

  /** process event */
  virtual void process(const CASSEvent&);

  /** load the settings of the pp */
  virtual void loadSettings(size_t);

protected:
  /** pp containing input table */
  PostprocessorBackend *_table;

  /** offset of first bin in input in Histogram coordinates */
  size_t _colIdx;
};

}//end namespace cass
#endif
