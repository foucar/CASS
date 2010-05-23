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


  Histogram1DFloat Histogram2DFloat::radial_project(std::pair<float,float> center,
                                                    float radius) const
  {

//    size_t NbrBins=static_cast<size_t>(ceil(radius));
//    const float NbrBins2=pow(NbrBins,2);
//    Histogram1DFloat hist(NbrBins, 0., radius);
//    Histogram1DFloat norms(NbrBins, 0., radius);
//
//    const int32_t xc(static_cast<int32_t>(centre.first));
//    const int32_t yc(static_cast<int32_t>(centre.second));
//
//    const int32_t row_min(static_cast<int32_t>(yc-radius));
//    const int32_t row_max(static_cast<int32_t>(yc+radius));
//
//    const int32_t col_min(static_cast<int32_t>(xc-radius));
//    const int32_t col_max(static_cast<int32_t>(xc+radius));
//
//    //here the following are safe as the radius has been safely reduced
//    for(int32_t row=row_min; row<row_max; ++row)
//      for(int32_t col=col_min; col<col_max; ++col)
//      {
//        float iradius2= square(row-yc)+square(col-xc);
//        //only if inside the radius add the bin-values
//        if(iradius2<=NbrBins2)
//        {
//          hist.bin( static_cast<int32_t>( floor(std::sqrt(iradius2)) ) ) += bin(row, col);
//          norms.bin(static_cast<int32_t>( floor(std::sqrt(iradius2)) ) ) +=1.;
//        }
//      }
//    //and now normalise the output histogram
//    for(size_t ibin=0; ibin<NbrBins; ibin++)
//      if(norms.bin(ibin)>0)
//        hist.bin(ibin)=hist.bin(ibin)/norms.bin(ibin);
//    return hist;

    Histogram1DFloat hist(static_cast<size_t>(radius), 0.f, radius);
    /** @note make this have the right cols and rows when binning is used */
    for(size_t jr = 0;jr<static_cast<size_t>(radius); jr++)
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

  Histogram1DFloat Histogram2DFloat::radar_plot(std::pair<float,float> center,
                                                std::pair<float,float> range) const
  {
    Histogram1DFloat hist(360, 0.f, 360.f);
    /** @note make this have the right cols and rows when binning is used */
    for(size_t jr = static_cast<size_t>(range.first);jr<static_cast<size_t>(range.second); jr++)
    {
      for(size_t jth = 1; jth<360; jth++)
      {
        const float radius(jr);
        const float angle(2.*M_PI * float(jth) / float(360));
        size_t col(size_t(center.first  + radius*sin(angle)));
        size_t row(size_t(center.second + radius*cos(angle)));
        float val = _memory[col + row * _axis[0].nbrBins()];
        hist.memory()[jth-1]+=val;
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
