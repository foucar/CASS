//Copyright (C) 2011 Lutz Foucar

/**
 * @file pixeldetector_mask.h contains definition of the mask of a pixeldetector
 *
 * @author Lutz Foucar
 */

#ifndef _PIXELDETECTOR_MASK_HPP_
#define _PIXELDETECTOR_MASK_HPP_

#include <vector>

namespace cass
{
class CASSSettings;

namespace pixeldetector
{
class CommonData;

/** a mask element
 *
 * each Mask element need the following "attributes": shape, xsize, ysize,
 * xcenter, ycenter, shape and orientation shapes:=circ(==circle),ellipse,
 * triangle(isosceles),square and  the orientation!!!
 *
 * The orientation is used only in the case of a triangular shape
 *
 * xsize,ysize and center are in pixel units.
 *
 * The orientation is used only in the case of a triangular shape.
 *
 * @verbatim
 /\           ----         |\           /|
/  \  ==+1    \  /  ==-1   | \  ==+2   / | == -2
----           \/          | /         \ |
                           |/           \|
   @endverbatim
 * if I rotate the plane by -pi/2: -2=>+1 1=>+2 -1=>-2  +2=>-1.
 * Please remember to use the rotated frame wrt standard-natural frame
 * orientation!!
 *
 * @todo refine the documentation by using casssettng
 *
 * @author Nicola Coppola
 */
struct MaskElement
{
  /** shape type */
  std::string type;

  /** size of shape along x-axis */
  uint32_t xsize;

  /** the size(s) along the y axis */
  uint32_t ysize;

  /** the centre(s) along the x axis */
  uint32_t xcentre;

  /** the centre(s) along the y axis */
  uint32_t ycentre;

  /** the orientation of the triangle */
  int32_t orientation;
};




/** create the mask
 *
 * a mask is created from a list of mask elements loaded form the ini file. See
 * MaskElement for a detailed description of the individual mask elements
 *
 * @author Nicola Coppola
 * @author Lutz Foucar
 */
void createCASSMask(CommonData &data, CASSSettings &s);

}//end namespace pixeldetector
}//end namespace cass

#endif
