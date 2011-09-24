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

/** create the mask
 *
 * a mask is created from a list of mask elements loaded form the ini file. The
 * individual elements properties are loaded from the .ini file according to
 * their shape.
 *
 * @cassttng PixelDetectors/\%name\%/CorrectionMaps/Mask/{size}\n
 *           The number of mask elements that are part of the complete mask.
 *           Default is 0.
 * @cassttng PixelDetectors/\%name\%/CorrectionMaps/Mask/\%index\%/{MaskElementType}\n
 *           Name of the mask element. Default is "square". Possible values are:
 *           - "square": a square region of the mask. See
 *                       cass::pixeldetector::addSquare
 *           - "circle" or "circ": a circular region of the mask. See
 *                                 cass::pixeldetector::addCircle
 *           - "ellipse": a ellipsodial region of the mask. See
 *                       cass::pixeldetector::addEllipse
 *           - "triangle": a square region of the mask. See
 *                       cass::pixeldetector::addTriangle
 *
 * @author Nicola Coppola
 * @author Lutz Foucar
 */
void createCASSMask(CommonData &data, CASSSettings &s);

}//end namespace pixeldetector
}//end namespace cass

#endif
