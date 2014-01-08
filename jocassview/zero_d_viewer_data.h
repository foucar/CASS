// Copyright (C) 2014 Lutz Foucar

/**
 * @file zero_d_viewer_data.h contains the wrapper of the data for the 0d viewer
 *
 * @author Lutz Foucar
 */

#ifndef _ZERODVIEWERDATA_
#define _ZERODVIEWERDATA_

#include "data.h"

namespace cass
{
class Histogram0DFloat;
}//end namespace cass

class QLabel;

namespace jocassview
{

/** wrapper for the 0d data
 *
 * @author Lutz Foucar
 */
class ZeroDViewerData : public Data
{
public:
  /** constructor
   *
   * initializes the pointer to 0
   *
   * @param valuedisplay The label that displays the value.
   */
  ZeroDViewerData(QLabel *valuedisplay);

  /** virtual destructor
   *
   * destory the 0d histogram
   */
  virtual ~ZeroDViewerData();

  /** set the result
   *
   * take ownership of the data pointed to and destroy anthing that we have been
   * managing so far.
   *
   * @param result the pointer to the data that contains the value we manage
   */
  virtual void setResult(cass::HistogramBackend *result);

  /** retrieve the result
   *
   * @return pointer to the result
   */
  virtual cass::HistogramBackend* result();

private:
  /** the cass data container */
  cass::Histogram0DFloat *_hist;

  /** pointer to the lable that displays the value */
  QLabel *_value;
};
}//end namespace jocassview
#endif
