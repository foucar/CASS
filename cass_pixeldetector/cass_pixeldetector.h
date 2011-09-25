//Copyright (C) 2011 Lutz Foucar

/**
 * @file cass_pixeldetector.h contains global definitions for all classes
 *
 * @author Lutz Foucar
 */


#ifndef _CASS_PIXELDETECTOR_H_
#define _CASS_PIXELDETECTOR_H_

#include <stdint.h>
#include <vector>
#include <string>
#include <sstream>

namespace cass
{
namespace pixeldetector
{

/** the type of a pixel of a ccd image*/
typedef float pixel_t;

/** a frame is a vector of pixels */
typedef std::vector<pixel_t> frame_t;


/** convert any type to a string
 *
 * should be used for converting numbers to strings. This function was inspired
 * by a function found at Sep, 24th 2011 here:
 * http://notfaq.wordpress.com/2006/08/30/c-convert-int-to-string/
 *
 * @tparam Type the type of the number
 * @param t the number to convert to string
 *
 * @author Lutz Foucar
 */
template <typename Type>
std::string toString (const Type& t)
{
  std::stringstream ss;
  ss << t;
  return ss.str();
}

/** Pixel definition
 *
 * Defines a pixel within a pixel detector.
 *
 * @author Lutz Foucar
 */
struct Pixel
{
  /** constructor
     *
     * @param X the x coordinate of the pixel
     * @param Y the y coordinate of the pixel
     * @param Z the value of the pixel
     */
  Pixel(uint16_t X, uint16_t Y, pixel_t Z)
    :x(X),y(Y),z(Z),used(false)
  {}

  /** default constructor.*/
  Pixel()
    :x(0),y(0),z(0),used(0)
  {}

  /** x coordinate of the pixel */
  uint16_t x;

  /** x coordinate of the pixel */
  uint16_t y;  //!< y coordinate of the pixel

  /** x coordinate of the pixel */
  pixel_t z;  //!< the pixel value

  /** value to mark pixel any analysis of the pixels */
  uint32_t used;
};

/** A Hit on a pixel detector.
 *
 * This class defines a hit on a pixel detector that might consist of more
 * one pixel.
 *
 * @author Lutz Foucar
 */
struct Hit
{
  /** default constructor.*/
  Hit()
    :x(0),y(0),z(0),nbrPixels(0)
  {}

  /** the x coordinate of hit */
  float x;

  /** the x coordinate of hit */
  float y;

  /** the value of the hit */
  uint64_t z;

  /** number of pixels that this hit consists of */
  size_t nbrPixels;
};

}//end namespace pixeldetector
}//end namepace cass
#endif
