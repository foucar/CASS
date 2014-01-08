// Copyright (C) 2014 Lutz Foucar

/**
 * @file data.h contains the base class for add viewer data
 *
 * @author Lutz Foucar
 */

#ifndef _JOCASSVIEWDATA_
#define _JOCASSVIEWDATA_

namespace jocassview
{
/** base class for all data wrappers
 *
 * @author Lutz Foucar
 */
class Data
{
public:
  /** virtual destrutor */
  virtual ~Data();

  /** fill the data with the result
   *
   * @param result the result to fill into this data container
   */
  virtual void setResult(cass::HistogramBackend* result) = 0;
};
}//end namespace jocassview
