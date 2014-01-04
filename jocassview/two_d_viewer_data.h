// Copyright (C) 2013 Lutz Foucar

/**
 * @file two_d_viewer_data.h contains the wrappe of the data for the 2d viewer
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
  /** define the shape of the data */
  typedef std::pair<size_t,size_t> shape_t;

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
               const QwtInterval &xrange, const QwtInterval &yrange);

  /** return the min max values of the values in the data
   *
   * @return the interval of min to max values in the data
   */
  QwtInterval origZInterval()const;

  /** return the value of the data at point x,y
   *
   * @param x the x coordinate of the requested data
   * @param x the x coordinate of the requested data
   */
  virtual double value(double x, double y) const;

private:
  /** vector that contains the linearized array */
  std::vector<float> _data;

  /** the shape of the 2d data */
  shape_t _shape;
};
}//end namespace jocassview
#endif
