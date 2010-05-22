// Copyright (C) 2010 Jochen Küpper

#include <QtGui/QColor>
#include <algorithm>
#include <cmath>

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

  Histogram1DFloat Histogram2DFloat::radial_project(std::pair<float,float> centre,
                                                    std::pair<float,float> range, float radius) const
  {

    size_t NbrBins=static_cast<size_t>(ceil(radius));
    const float NbrBins2=pow(NbrBins,2);
    Histogram1DFloat hist(NbrBins, 0., radius);
    Histogram1DFloat norms(NbrBins, 0., radius);

    const int32_t xc(static_cast<int32_t>(centre.first));
    const int32_t yc(static_cast<int32_t>(centre.second));

    const int32_t row_min(static_cast<int32_t>(yc-radius));
    const int32_t row_max(static_cast<int32_t>(yc+radius));

    const int32_t col_min(static_cast<int32_t>(xc-radius));
    const int32_t col_max(static_cast<int32_t>(xc+radius));

    //here the following are safe as the radius has been safely reduced
    for(int32_t row=row_min; row<row_max; ++row)
      for(int32_t col=col_min; col<col_max; ++col)
      {
        float iradius2= pow(row-yc,2)+pow(col-xc,2);
        //only if inside the radius add the bin-values
        if(iradius2<=NbrBins2)
        {
          /*          std::cout<<"rad vs pos "<< row-yc << " "<< col-xc << " "<< floor(std::sqrt(iradius2)) << " " 
                      << row << " " << col << " " << bin(row, col)<<std::endl;*/
          hist.bin( static_cast<int32_t>( floor(std::sqrt(iradius2)) ) ) += bin(row, col);
          //hist.bin( static_cast<int32_t>( floor(std::sqrt(iradius2)) ) ) += 1.1;
          norms.bin(static_cast<int32_t>( floor(std::sqrt(iradius2)) ) ) +=1.;
        }
      }
    //and now normalise the output histogram
    for(size_t ibin=0; ibin<NbrBins; ibin++)
      if(norms.bin(ibin)>0) hist.bin(ibin)=hist.bin(ibin)/norms.bin(ibin);
    return hist;
  }

  Histogram1DFloat Histogram2DFloat::radar_plot(std::pair<float,float> centre,
                                                std::pair<float,float> range) const
  {
    size_t NbrBins=720;
    const float NbrBins2_min=pow(range.first,2);
    const float NbrBins2_max=pow(range.second,2);
    //size_t NbrBins=100;
    //Histogram1DFloat hist(NbrBins, -180.f, 180.f);
    Histogram1DFloat hist(NbrBins, -M_PI, M_PI);
    //Histogram1DFloat hist(NbrBins, -1.005f, 1.005f);
    //Histogram1DFloat norms(NbrBins, 0., radius);

    const float xc(centre.first-0.5);
    const float yc(centre.second-0.5);

    //here the following are NOT safe as the radius has NOT been reduced

    int32_t row_min(static_cast<int32_t>(yc-range.second));
    row_min=std::max(row_min,static_cast<int32_t>(_axis[yAxis].lowerLimit()));
    int32_t row_max(static_cast<int32_t>(yc+range.second));
    row_max=std::min(row_max,static_cast<int32_t>(_axis[yAxis].upperLimit()));

    int32_t col_min(static_cast<int32_t>(xc-range.second));
    col_min=std::max(col_min,static_cast<int32_t>(_axis[xAxis].lowerLimit()));
    int32_t col_max(static_cast<int32_t>(xc+range.second));
    col_max=std::min(col_max,static_cast<int32_t>(_axis[xAxis].upperLimit()));

    /*std::cout<<"0 4 " 
             <<row_min << " "<<row_max<<" "
             <<col_min << " "<<col_max<<std::endl;*/
    //std::cout<<"0 5 " <<std::endl;

    //the following loop is quite generous...
    for(int32_t row=row_min; row<row_max; ++row)
      for(int32_t col=col_min; col<col_max; ++col)
      {
        float iradius2= square(static_cast<float>(row)-yc)+square(static_cast<float>(col)-xc);
        //only if inside the corona of range.first,range.second add the bin-values
        //boundary included!!
        if(iradius2<NbrBins2_max && iradius2>NbrBins2_min)
        {

          //const float angle(180.*std::atan2(static_cast<double>(row-yc),static_cast<double>(col-xc))/M_PI);
          const float angle(std::atan2(static_cast<double>(row)-yc,static_cast<double>(col)-xc));
          /*float cosangle;
          if ((square(col-xc)+square(row-yc))!=0)
            cosangle= (col-xc)/sqrt(square(col-xc)+square(row-yc)) ;
          else
          cosangle=0.;*/
          /*
          std::cout<<"angle vs pos "<< row-yc << " "<< col-xc << " "<< angle << " " 
                      << row << " " << col <<std::endl;
          */
          //hist.bin( angle ) += bin(row, col);
          //hist.fill( cosangle );
          hist.fill( angle );
          //hist.fill( angle,bin(row,col) );
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
