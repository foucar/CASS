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

#include "qwt_data.h"
#include "qwt_double_interval.h"

namespace jocassview
{

/** the 1d data wrapper
 *
 * @author Lutz Foucar
 */
class OneDViewerData : public QwtData
{
public:
  /** default constructor */
  OneDViewerData();

  /** copy constructor
   *
   * @param other the other object to copy
   */
  OneDViewerData(const OneDViewerData& other);

  /** return a pointer to a copy of this
   *
   * @return a pointer to a copy of this
   */
  virtual QwtData * copy() const;

  /** return the size of the data
   *
   * @return the size of the vector that holds the data
   */
  virtual size_t size() const;

  /** return the x value for data point
   *
   * @param i the datapoint whos x value should be returned
   */
  virtual double x(size_t i) const;

  /** return the y value for data point
   *
   * @param i the datapoint whos y value should be returned
   */
  virtual double y(size_t i) const;

  /** return the bounding rectangle
   *
   * @return the bounding rectangle
   */
  virtual QwtDoubleRect boundingRect() const;

  /** set the data
   *
   * @param data vector containing the data
   * @param xRange the x Range
   */
  void setData(const std::vector<float> &data, const QwtDoubleInterval &xRange);

public:
  /** vector that contains the linearized array */
  std::vector<float> _data;

  /** the range in x */
  QwtDoubleInterval _xRange;

  /** the range in y */
  QwtDoubleInterval _yRange;

  /** the name of the data */
  QString _name;
};
}//end namespace jocassview
#endif
