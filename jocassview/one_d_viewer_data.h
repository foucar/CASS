// Copyright (C) 2013 Lutz Foucar

/**
 * @file one_d_viewer_data.h contains the wrapper of the data for the 1d viewer
 *
 * @author Lutz Foucar
 */

#ifndef _ONEDVIEWERDATA_
#define _ONEDVIEWERDATA_

#include <vector>

#include <QtCore/QString>

#include <qwt_series_data.h>

#include "data.h"

namespace cass
{
class Histogram1DFloat;
}//end namespace cass

namespace jocassview
{

/** the 1d data wrapper
 *
 * @author Lutz Foucar
 */
class OneDViewerData : public QwtSeriesData<QPointF>, public Data
{
public:
  /** default constructor
   *
   * intializes the _hist pointer with 0
   */
  OneDViewerData();

  /** destructor
   *
   * deletes the _hist pointer
   */
  virtual ~OneDViewerData();

  /** return the size of the data
   *
   * @return the size of the vector that holds the data
   */
  virtual size_t size() const;

  /** return the x value for data point
   *
   * @param i the datapoint whos x value should be returned
   */
  virtual QPointF sample(size_t i) const;

  /** return the bounding rectangle
   *
   * @return the bounding rectangle
   */
  virtual QRectF boundingRect() const;

  /** set the cass data to be wrapped by this
   *
   * @param hist the cass data to be wrapped
   */
  void setData(cass::Histogram1DFloat *hist);

  /** retrieve pointer to the cass data
   *
   * @return pointer to cass data
   */
  cass::Histogram1DFloat* data();

  /** retrieve const pointer to the cass data
   *
   * @return const pointer to cass data
   */
  const cass::Histogram1DFloat* data()const;

  /** set up the bounding rect for when x should be log scale
   *
   * in case of log-scale:
   * find the minimum x value that is bigger than 0 and set Left to be that
   * value.
   *
   * in case of lin-scale:
   * set Left to the lowest value
   *
   * @param log when this parameter is true the scale will be set up for
   *            log-scale otherwise for lin-scale
   */
  void setXRangeForLog(bool log);

  /** set up the bounding rect for when y should be log scale
   *
   * in case of log-scale:
   * find the minimum y value that is bigger than 0 and a valid number, set
   * bottom to that value.
   *
   * in case of lin-scale:
   * set bottom to the lowest value
   *
   * @param log when this parameter is true the scale will be set up for
   *            log-scale otherwise for lin-scale
   */
  void setYRangeForLog(bool log);

public:
  /** the minimum positions in x and y for log scales */
  QPointF _logMinPos;

  /** flag to tell whether x scale will be drawn in log */
  bool _xLog;

  /** flag to tell whether y scale will be drawn in log */
  bool _yLog;

  /** pointer to the cass data */
  cass::Histogram1DFloat *_hist;
};
}//end namespace jocassview
#endif
