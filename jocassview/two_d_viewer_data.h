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

#include "data.h"

namespace cass
{
class Histogram2DFloat;
}//end namespace cass

namespace jocassview
{

/** the 2d data wrapper
 *
 * @author Lutz Foucar
 */
class TwoDViewerData : public QwtRasterData, public Data
{
public:
  /** define the shape of the data */
  typedef std::pair<size_t,size_t> shape_t;

  /** default constructor
   *
   * intialize the histogram pointer to 0
   */
  TwoDViewerData();

  /** destructor
   *
   * delete the histogram data pointed to
   */
  virtual ~TwoDViewerData();

  /** set the cass data to be wrapped by this
   *
   * takes over ownership of the data pointed to and deletes it when another
   * pointer is passed to this.
   *
   * @param hist the histogram that contains the data
   */
  void setResult(cass::HistogramBackend *hist);

  /** retrieve the pointer to the data
   *
   * @return pointer to the data
   */
  cass::HistogramBackend* result();

  /** return the min max values of the values in the data
   *
   * @return the interval of min to max values in the data
   * @param log when true it will return the minimal positive value
   */
  QwtInterval origZInterval(bool log)const;

  /** return the value of the data at point x,y
   *
   * @param x the x coordinate of the requested data
   * @param y the y coordinate of the requested data
   */
  virtual double value(double x, double y) const;

private:
  /** the cass data container */
  cass::Histogram2DFloat *_hist;
};
}//end namespace jocassview
#endif
