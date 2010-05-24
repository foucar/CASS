// Copyright (C) 2010 Jochen Küpper

#include <algorithm>
#include <cmath>

#include "histogram.h"


namespace cass
{
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


  Histogram1DFloat Histogram2DFloat::radial_project(const std::pair<size_t,size_t> &center, size_t maxRadius)const
  {
    Histogram1DFloat hist(maxRadius, 0., _axis[xAxis].hist2user(maxRadius));
    for(size_t jr = 0;jr<maxRadius; jr++)
    {
      float val(0);
      for(size_t jth = 1; jth<360; jth++)
      {
        const float radius(jr);
        const float angle(2.*M_PI * float(jth) / float(360));
        size_t col(size_t(center.first  + radius*sin(angle)));
        size_t row(size_t(center.second + radius*cos(angle)));
        val += _memory[col + row * _axis[0].nbrBins()];
      }
      hist.memory()[jr]+=val;
    }
    return hist;
  }


  Histogram1DFloat Histogram2DFloat::radar_plot(std::pair<size_t,size_t> center,
                                                std::pair<size_t,size_t> range,
                                                size_t nbrBins) const
  {
    Histogram1DFloat hist(nbrBins, 0., 360.);
    for(size_t jr = range.first;jr<range.second; jr++)
    {
      for(size_t jth = 1; jth<360; jth++)
      {
        const float radius(jr);
        const float angle(2.*M_PI * float(jth) / float(360));
        size_t col(size_t(center.first  + radius*sin(angle)));
        size_t row(size_t(center.second + radius*cos(angle)));
        float val = _memory[col + row * _axis[0].nbrBins()];
        hist.fill(jth-0.5,val);
      }
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
