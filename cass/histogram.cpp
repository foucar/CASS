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

#include "cass.h"


using namespace cass;

void AxisProperty::setSize(size_t size, bool fix)
{
  _size=size;
  if (fix)
  {
    _low=0;
    _up=size-1;
  }
}

void Histogram1DFloat::append(const storage_t::value_type &value)
{
  _memory.push_back(value);
  _axis[xAxis].setSize(_memory.size(),true);

}

void Histogram1DFloat::clearline()
{
  _memory.clear();
  _axis[xAxis].setSize(0,true);
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
