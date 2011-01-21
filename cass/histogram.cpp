// Copyright (C) 2010 Jochen Küpper
// Copyright (C) 2010 Lutz Foucar

/**
 * @file histogram.h file contains histogram classes definitions
 *
 * @author Lutz Foucar
 */

#include <algorithm>
#include <cmath>

#include "histogram.h"


namespace cass
{
  std::vector<size_t> AxisProperty::rebinfactors()const
  {
    std::vector<size_t> factors;
    for (size_t i=2; i<_size;++i)
      if(0 == _size % i)
        factors.push_back(i);
    return factors;
  }

  void HistogramFloatBase::operator=(const HistogramFloatBase& rhs)
  {
    _axis = rhs._axis;
    _nbrOfFills = rhs._nbrOfFills;
    _memory = rhs._memory;
  }

  void Histogram1DFloat::resize(size_t nbrXBins, float xLow, float xUp)
  {
    using namespace std;
    QWriteLocker wlock(&lock);
    _memory.clear();
    string xaxisTitle (_axis[HistogramBackend::xAxis].title());
    _axis.clear();
    _memory.resize(nbrXBins+2,0);
    _axis.push_back(AxisProperty(nbrXBins,xLow,xUp,xaxisTitle));
  }

  void Histogram2DFloat::resize(size_t nbrXBins, float xLow, float xUp,
                                size_t nbrYBins, float yLow, float yUp)
  {
    using namespace std;
    QWriteLocker wlock(&lock);
    _memory.clear();
    string xaxisTitle (_axis[HistogramBackend::xAxis].title());
    string yaxisTitle (_axis[HistogramBackend::yAxis].title());
    _axis.clear();
    _memory.resize(nbrXBins*nbrYBins+8,0);
    _axis.push_back(AxisProperty(nbrXBins,xLow,xUp,xaxisTitle));
    _axis.push_back(AxisProperty(nbrYBins,yLow,yUp,yaxisTitle));
  }

  Histogram1DFloat Histogram2DFloat::project(std::pair<float,float> range, Histogram2DFloat::Axis axis) const
  {
    Histogram1DFloat hist(_axis[axis].size(), _axis[axis].lowerLimit(), _axis[axis].upperLimit(), _axis[axis].title());
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
    Histogram1DFloat norms(maxRadius, 0., _axis[xAxis].hist2user(maxRadius));

    const float NbrBins2=pow(static_cast<float>(maxRadius),2);

    //    const int32_t xc(static_cast<int32_t>(center.first));
    //    const int32_t yc(static_cast<int32_t>(center.second));

    const int32_t row_min(static_cast<int32_t>(center.second-maxRadius));
    const int32_t row_max(static_cast<int32_t>(center.second+maxRadius));

    const int32_t col_min(static_cast<int32_t>(center.first-maxRadius));
    const int32_t col_max(static_cast<int32_t>(center.first+maxRadius));

    //here the following are safe as the radius has been safely reduced
    for(int32_t row=row_min; row<row_max; ++row)
      for(int32_t col=col_min; col<col_max; ++col)
      {
        float iradius2= square(row-center.second)+square(col-center.first);
        //only if inside the radius add the bin-values
        if(iradius2<=NbrBins2)
        {
          hist.bin( static_cast<int32_t>( floor(std::sqrt(iradius2)) ) ) += bin(row, col);
          norms.bin(static_cast<int32_t>( floor(std::sqrt(iradius2)) ) ) +=1.;
        }
      }
    //and now normalise the output histogram
    for(size_t ibin=0; ibin<maxRadius; ibin++)
      {
        if(norms.bin(ibin)>0)
          hist.bin(ibin)=hist.bin(ibin)/norms.bin(ibin);
        else hist.bin(ibin)=0.;
      }
    return hist;

    /* @todo make new function with somebody-else version, which is marked out*/
    /* the following works only if the 2*pi*maxRadius<360 otherwise is ABSOLUTELY mathematically INCORRECT */
    //    Histogram1DFloat hist(maxRadius, 0., _axis[xAxis].hist2user(maxRadius));
    //    for(size_t jr = 0;jr<maxRadius; jr++)
    //    {
    //      float val(0);
    //      for(size_t jth = 1; jth<360; jth++)
    //      {
    //        const float radius(jr);
    //        const float angle(2.*M_PI * float(jth) / float(360));
    //        size_t col(size_t(center.first  + radius*sin(angle)));
    //        size_t row(size_t(center.second + radius*cos(angle)));
    //        val += _memory[col + row * _axis[0].nbrBins()];
    //      }
    //      hist.memory()[jr]+=val;
    //    }
    //    return hist;
  }


  Histogram1DFloat Histogram2DFloat::radar_plot(const std::pair<size_t,size_t> &center,
                                                const std::pair<size_t,size_t> &range,
                                                size_t nbrBins) const
  {
    Histogram1DFloat hist(nbrBins, 0., 360.);
//    const float NbrBins2_min=pow(static_cast<float>(range.first),2);
    const float NbrBins2_max=pow(static_cast<float>(range.second),2);
    const float xc(static_cast<float>(center.first)-0.5);
    const float yc(static_cast<float>(center.second)-0.5);

    //here the following are NOT safe as the radius has NOT been reduced

    //HHHHAAAAA I do not know anymore the yAxis...
    // I have therefore lost the capability of drawing a part of the circle

    int32_t row_min(static_cast<int32_t>(yc-NbrBins2_max));
//    row_min=std::max(row_min,static_cast<int32_t>(_axis[yAxis].lowerLimit()));
    row_min=std::max(row_min,0);
//    int32_t row_max(static_cast<int32_t>(yc+NbrBins2_max));
//    row_max=std::min(row_max,static_cast<int32_t>(_axis[yAxis].upperLimit()));

    int32_t col_min(static_cast<int32_t>(xc-NbrBins2_max));
//    col_min=std::max(col_min,static_cast<int32_t>(_axis[xAxis].lowerLimit()));
    col_min=std::max(col_min,0);
//    int32_t col_max(static_cast<int32_t>(xc+NbrBins2_max));
//    col_max=std::min(col_max,static_cast<int32_t>(_axis[xAxis].upperLimit()));

    //the following loop is quite generous...
//    for(int32_t row=row_min; row<row_max; ++row)
//      for(int32_t col=col_min; col<col_max; ++col)
//      {
//        float iradius2= square(static_cast<float>(row)-yc)+square(static_cast<float>(col)-xc);
        //only if inside the corona of range.first,range.second add the bin-values
        //boundary included!!
//      if(iradius2<NbrBins2_max && iradius2>NbrBins2_min)
//      {
//
//        //const float angle(180.*std::atan2(static_cast<double>(row-yc),static_cast<double>(col-xc))/M_PI);
//        const float angle(std::atan2(static_cast<float>(row)-yc,static_cast<float>(col)-xc));
//        /*float cosangle;
//          if ((square(col-xc)+square(row-yc))!=0)
//            cosangle= (col-xc)/sqrt(square(col-xc)+square(row-yc)) ;
//          else
//          cosangle=0.;*/
//        /*
//          std::cout<<"angle vs pos "<< row-yc << " "<< col-xc << " "<< angle << " "
//                      << row << " " << col <<std::endl;
//          */
//        //hist.bin( angle ) += bin(row, col);
//        //hist.fill( cosangle );
//        //hist.fill( angle );
//        hist.fill( angle,bin(row,col) );
//      }
//      }

    /* the following works only if the 2*pi*maxRadius<360 otherwise is ABSOLUTELY mathematically INCORRECT */
    /* and it is clearly not as general as the previously implemented code */
    //    Histogram1DFloat hist(nbrBins, 0., 360.);
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



  Histogram2DFloat Histogram2DFloat::convert2RPhi(const std::pair<size_t,size_t> &center,
                                                  const size_t maxRadius,
                                                  const size_t nbrAngleBins) const
  {
    Histogram2DFloat hist(nbrAngleBins, 0., 360.,
                          maxRadius, 0., _axis[yAxis].hist2user(maxRadius));
    for(size_t jr = 0;jr<maxRadius; jr++)
    {
      for(size_t jth = 1; jth<360; jth++)
      {
        const float radius(jr);
        const float angle(2.*M_PI * float(jth) / float(360));
        size_t col(size_t(center.first  + radius*sin(angle)));
        size_t row(size_t(center.second + radius*cos(angle)));
        float val = _memory[col + row * _axis[0].nbrBins()];
        hist.fill(jth-0.5, _axis[yAxis].hist2user(jr) , val);
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
