// Copyright (C) 2010 Jochen Küpper
// Copyright (C) 2010-2013 Lutz Foucar

/**
 * @file cass/histogram.cpp file contains histogram classes definitions
 *
 * @author Lutz Foucar
 */

#include <algorithm>
#include <cmath>
#include <stdexcept>

#include "histogram.h"


using namespace cass;

std::vector<size_t> AxisProperty::rebinfactors()const
{
  std::vector<size_t> factors;
  for (size_t i=2; i<_size;++i)
    if(0 == _size % i)
      factors.push_back(i);
  return factors;
}

void AxisProperty::setSize(size_t size, bool fix)
{
  _size=size;
  if (fix)
  {
    _low=0;
    _up=size-1;
  }
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

void Histogram1DFloat::append(const storage_t::value_type &value)
{
  QWriteLocker wlock(&lock);
  _memory.push_back(value);
  _axis[xAxis].setSize(_memory.size(),true);

}

void Histogram1DFloat::clearline()
{
  QWriteLocker wlock(&lock);
  _memory.clear();
  _axis[xAxis].setSize(0,true);
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
  size_t columns(_axis[0].size()), rows(_axis[1].size());
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

void Histogram2DFloat::appendRows(const HistogramFloatBase::storage_t &rows)
{
  using namespace std;
  if (rows.size() % _axis[xAxis].size())
    throw runtime_error("Histogram2DFloat::addRow: The rowsize '"+
                        toString(rows.size()) + "' that should be added to the " +
                        "table is not a modulo of the rowsize of the table '" +
                        toString(_axis[xAxis].size()));
  _axis[yAxis].setSize(_axis[yAxis].size() + (rows.size()/_axis[xAxis].size()),true);
  _memory.insert(_memory.end(),rows.begin(),rows.end());
}

void Histogram2DFloat::clearTable()
{
  _axis[yAxis].setSize(0,true);
  _memory.clear();
}

#ifdef JPEG_CONVERSION

/* libjpeg memory managment stuff */
#define JPEG_BLOCK_SIZE 16384   // buffer resize step

struct jpeg_vector_destination{
  struct jpeg_destination_mgr jdst;
  std::vector<JOCTET>* buffer;
};

void my_init_destination(j_compress_ptr cinfo)
{
  std::vector<JOCTET>& buffer = *(((jpeg_vector_destination*)(cinfo->dest))->buffer);
  buffer.resize(JPEG_BLOCK_SIZE);
  cinfo->dest->next_output_byte = &buffer[0];
  cinfo->dest->free_in_buffer = buffer.size();
}

boolean my_empty_output_buffer(j_compress_ptr cinfo)
{
  std::vector<JOCTET>& buffer = *(((jpeg_vector_destination*)(cinfo->dest))->buffer);
  size_t oldsize = buffer.size();
  buffer.resize(oldsize + JPEG_BLOCK_SIZE);
  cinfo->dest->next_output_byte = &buffer[oldsize];
  cinfo->dest->free_in_buffer = buffer.size() - oldsize;
  return true;
}

void my_term_destination(j_compress_ptr cinfo)
{
  std::vector<JOCTET>& buffer = *(((jpeg_vector_destination*)(cinfo->dest))->buffer);
  buffer.resize(buffer.size() - cinfo->dest->free_in_buffer);
}



/* render 1d histogram into jpeg image
 * @author Stephan kassemeyer
 */
std::vector<JOCTET>* Histogram1DFloat::jpegImage() const {
  int nxpix = 1024;
  int nypix = 1024;
  float plotvalue = 100;
  Histogram2DFloat outhist(nxpix, 0, nxpix, nypix, 0, nypix, "X","Y");
  float xx_low = _axis[0].lowerLimit();
  float xx_up = _axis[0].upperLimit();
  float yy_low = min();
  float yy_up = max();
  float dxx = xx_up-xx_low;
  float dyy = yy_up-yy_low;

  for (int xx_pix=0;xx_pix<nxpix;++xx_pix) {
    float xx = xx_pix*dxx/nxpix + xx_low;
    float yy = (*this)(xx);
    int yy_pix = (yy-yy_low)/dyy * nypix;
    std::cout << xx << "," << yy << "    -    " << xx_pix << "," << yy_pix << " : " << plotvalue << std::endl;
    std::cout << _axis[0].bin(xx) << std::endl;
    if (xx_pix >=0 && xx_pix<nxpix && yy_pix>=0 && yy_pix<nypix) {
      outhist.fill(xx_pix,yy_pix,plotvalue);
    }
    //    for (int ii=0; ii<size(); ++ii) {
    //      float xx = _axis[0].position(ii);
    //      float yy = bin(ii);
    //      int xx_pix = (xx-xx_low)/dxx * nxpix;
    //      int yy_pix = (yy-yy_low)/dyy * nypix;
    //      if (xx_pix >=0 && xx_pix<nxpix && yy_pix>=0 && yy_pix<nypix) {
    //        //outhist.fill(xx_pix,yy_pix,plotvalue);
    //        outhist.fill(xx_pix,xx_pix,plotvalue);
    //        std::cout << xx << "," << yy << "    -    " << xx_pix << "," << yy_pix << " : " << plotvalue << std::endl;
    //      }
  }
  return outhist.jpegImage();
}



/* render 2d histogram into jpeg image
 * @author Stephan kassemeyer
 */
std::vector<JOCTET>* Histogram2DFloat::jpegImage() const {
  struct jpeg_compress_struct cinfo;
  struct jpeg_error_mgr       jerr;

  jpeg_create_compress(&cinfo);

  // storage:
  std::vector<JOCTET>* buffer = new std::vector<JOCTET>;
  struct jpeg_vector_destination vector_destination;
  cinfo.dest = (jpeg_destination_mgr*)&vector_destination;
  vector_destination.buffer = buffer;
  vector_destination.jdst.init_destination = &my_init_destination;
  vector_destination.jdst.empty_output_buffer = &my_empty_output_buffer;
  vector_destination.jdst.term_destination = &my_term_destination;

  cinfo.err = jpeg_std_error(&jerr);

  cinfo.image_width      = _axis[0].size();
  cinfo.image_height     = _axis[1].size();
  cinfo.input_components = 1;
  cinfo.in_color_space   = JCS_GRAYSCALE;

  jpeg_set_defaults(&cinfo);
  /*set the quality [0..100]  */
  jpeg_set_quality (&cinfo, 75, true);
  jpeg_start_compress(&cinfo, true);

  std::vector<JSAMPLE> jpeg_sampling_memory(_memory.size());
  value2pixel converter(min(), max());
  std::transform(_memory.begin(), _memory.end()-8, jpeg_sampling_memory.begin(), converter);


  JSAMPROW row_pointer;          /* pointer to a single row */
  //    while (cinfo.next_scanline < cinfo.image_height) {
  //        row_pointer = (JSAMPROW) &jpeg_sampling_memory[cinfo.next_scanline*cinfo.image_width];
  //        jpeg_write_scanlines(&cinfo, &row_pointer, 1);
  //    }
  while (cinfo.next_scanline < cinfo.image_width) {
    row_pointer = (JSAMPROW) &jpeg_sampling_memory[cinfo.next_scanline*cinfo.image_height];
    jpeg_write_scanlines(&cinfo, &row_pointer, 1);
  }
  jpeg_finish_compress(&cinfo);
  jpeg_destroy_compress(&cinfo);

  int superflous(vector_destination.jdst.free_in_buffer);
  buffer->resize( buffer->size() - superflous );

  return buffer;
}
#endif //JPEG_CONVERSION

