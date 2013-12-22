// Copyright (C) 2013 Lutz Foucar

/**
 * @file tow_d_viewer_data.h contains the wrappe of the data for the 2d viewer
 *
 * @author Lutz Foucar
 */

#ifndef _TWODVIEWERDATA_
#define _TWODVIEWERDATA_

#include <vector>

#include <qwt_raster_data.h>

namespace jocassview
{

/** the 2d data wrapper
 *
 * @author Lutz Foucar
 */
class TwoDViewerData : public QwtRasterData
{
public:
  /** define the shape of the data */\
  typedef std::pair<size_t,size_t> shape_t;

  /** default constructor */
  TwoDViewerData();

  /** constructor with user parameters
   *
   * @param data the vector containing the data
   * @param shape the Shape of the data to be displayed
   * @param xrange the range in user coordinates in x
   * @param yrange the range in user coordinates in y
   * @param zrange the range of values to be displayed
   * @param boundingRect the boundingRect to be used
   */
  TwoDViewerData(const std::vector<float> &data, const shape_t &shape,
                 const QwtDoubleInterval &xrange, const QwtDoubleInterval &yrange,
                 const QwtDoubleInterval &zrange, const QwtDoubleRect &boundingRect);


  /** set the data that should be wrapped
   *
   * When setting the data, the z range will be defaulty set to the min and max
   * value of the data.
   *
   * @param data the vector containing the linearized data to be displayed
   * @param shape the Shape of the data to be displayed
   * @param xrange the range in user coordinates in x
   * @param yrange the range in user coordinates in y
   */
  void setData(const std::vector<float> &data, const shape_t &shape,
               const QwtDoubleInterval &xrange, const QwtDoubleInterval &yrange);

  /** set the range that should be displayed
   *
   * @param min the minimum value to be displayed
   * @param max the maximum value to be displayed
   */
  void setZRange(const QwtDoubleInterval & zrange);

  /** return the value of the data at point x,y
   *
   * @param x the x coordinate of the requested data
   * @param x the x coordinate of the requested data
   */
  virtual double value(double x, double y) const;

  /** return the z range to be displayed
   *
   * @return the z range to be displayed
   */
  virtual QwtDoubleInterval range() const;


  /** return the z range of the data
   *
   * @return the z range of the data
   */
  QwtDoubleInterval zRange() const;

  /** return a pointer to a copy of this
   *
   * @return a pointer to a copy of this
   */
  virtual QwtRasterData * copy() const;

private:
  /** the range in along x (fast) dimension */
  QwtDoubleInterval _xRange;

  /** the range in along y (slow) dimension */
  QwtDoubleInterval _yRange;

  /** the value range that should be displayed */
  QwtDoubleInterval _zRange;

  /** vector that contains the linearized array */
  std::vector<float> _data;

  /** the shape of the 2d data */
  shape_t _shape;
};
}//end namespace jocassview
#endif
