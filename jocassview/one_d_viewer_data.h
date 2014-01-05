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

namespace jocassview
{

/** the 1d data wrapper
 *
 * @author Lutz Foucar
 */
class OneDViewerData : public QwtSeriesData<QPointF>
{
public:
  /** default constructor */
  OneDViewerData();

  /** copy constructor
   *
   * @param other the other object to copy
   */
  OneDViewerData(const OneDViewerData& other);

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

  /** set the data
   *
   * @param data vector containing the data
   * @param xRange the x Range
   */
  void setData(const std::vector<float> &data, const QwtInterval &xRange);

public:
  /** vector that contains the linearized array */
  std::vector<float> _data;

  /** the range in x */
  QwtInterval _xRange;

  /** the range in y */
  QwtInterval _yRange;

  /** the name of the data */
  QString _name;
};
}//end namespace jocassview
#endif
