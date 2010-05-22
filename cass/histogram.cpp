// Copyright (C) 2010 Jochen Küpper

#include <QtGui/QColor>
#include <algorithm>

#include "histogram.h"


namespace cass
{
  /** Convert Histogram2DFloat::value_t to uint8_t
 *
 * @author Jochen Küpper
 */
  class value2pixel
  {
  public:

    value2pixel(Histogram2DFloat::value_t min, Histogram2DFloat::value_t max)
      : _min(min), _max(max)
    {};

    uint8_t operator()(Histogram2DFloat::value_t val) {
      return uint8_t((val - _min) / (_max - _min) * 0xff);
    };

  protected:

    Histogram2DFloat::value_t _min, _max;
  };




  QImage Histogram2DFloat::qimage()
  {
    QImage qi(shape().first, shape().second, QImage::Format_Indexed8);
    qi.setColorCount(256);
    for(unsigned i=0; i<256; ++i)
      qi.setColor(i, QColor(i, i, i).rgb());
    qi.fill(0);
    uint8_t *data(qi.bits());
    //    value2pixel converter(0,1);
    value2pixel converter(min(), max());
    lock.lockForRead();
    // Subtract 8 to get the size of the buffer excluding over/underflow flags
    std::transform(_memory.begin(), _memory.end()-8, data, converter);
    lock.unlock();
    return qi;
  }

  void HistogramFloatBase::operator=(const HistogramFloatBase& rhs)
  {
    _axis = rhs._axis;
    _nbrOfFills = rhs._nbrOfFills;
    _memory = rhs._memory;
  }

  Histogram1DFloat Histogram2DFloat::project(std::pair<float,float> range, Histogram2DFloat::Axis axis) const
  {
    Histogram1DFloat hist(_axis[axis].size(), _axis[axis].lowerLimit(), _axis[axis].upperLimit());
    size_t columns(_axis[1].size()), rows(_axis[0].size());
    switch(axis)
    {
    case xAxis: // reduce along rows (integrate rows)
      {
        size_t low(_axis[yAxis].bin(std::min(range.first,range.second)));
        size_t up (_axis[yAxis].bin(std::max(range.first,range.second)));
        for(size_t row=low; row<up; ++row)
          for(size_t col=0; col<columns; ++col)
            hist.bin(col) += bin(row, col);
      }
      break;
    case yAxis: // reduce along columns (integrate rows)
      {
        size_t low(_axis[xAxis].bin(std::min(range.first,range.second)));
        size_t up (_axis[xAxis].bin(std::max(range.first,range.second)));
        for(size_t row=0; row<rows; ++row)
          for(size_t col=low; col<up; ++col)
            hist.bin(row) += bin(row, col);
      }
      break;
    case zAxis:
      throw std::out_of_range("Cannot reduce 2D histogram along 3rd (z) axis!");
    }
    return hist;
  }


} // end namespace cass



// Local Variables:
// coding: utf-8
// mode: C++
// c-file-style: "gnu"
// c-file-offsets: ((c . 0) (innamespace . 0))
// fill-column: 100
// End:
