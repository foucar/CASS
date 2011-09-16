//Copyright (C) 2011 Lutz Foucar

/**
 * @file pixeldetector_mask.h contains definition of the mask of a pixeldetector
 *
 * @author Lutz Foucar
 */

#ifndef _PIXEL_DETECTOR_HPP_
#define _PIXEL_DETECTOR_HPP_

/** a mask element
 *
 *
 *  each Mask element need the following "attributes": shape, xsize, ysize,
 *  xcenter, ycenter, shape and orientation shapes:=circ(==circle),ellipse,triangle(isosceles),square and  the orientation!!!\n
 *  the orientation is used only in the case of a triangular shape
 *
 *  xsize,ysize and center are in pixel units.
 *
 * The orientation is used only in the case of a triangular shape.\n
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
  std::string _type; //!< the shape of the simpleROI

  /** size of shape along x-axis */
  uint32_t xsize;   //!< the size(s) along the x axis
  uint32_t ysize;   //!< the size(s) along the y axis
  uint32_t xcentre; //!< the centre(s) along the x axis
  uint32_t ycentre; //!< the centre(s) along the y axis

  int32_t orientation;  //!< the orientation of the triangle
  ~ROIsimple()      {}
  //The following is a "wish", that for the moment is not fullfilled (or needed)!
  //I want something like void ROIload(detROI_t *_detROI);
  //void load(cass::PixelDetector::detROI_t&);
  //void save(cass::PixelDetector::detROI_t *_detROI);
};




/** The mask
 *
 * a region of interest is created from a list of simple shapes.
 * @see cass::ROIsimple
 *
 * this function will define/create
 * - a ROI Mask (the values are:\n
 *          =1 pixel is to be used,\n
 *          =2 pixel is declared BAD,\n
 *          =0 pixel is masked\n
 * - a ROI Iterator, which is a list of indices of the frame that are not
 *   masked as uniteresting\n
 * - a ROI Mask Converter, which is the transformed (in the input original
 *   shape) ROI mask for each detector\n
 * - a ROI Iterator Converter, which is the transformed (in the input
 *   original shape) ROI index-pointer-mask for each detector.\n
 * All of these entities are vectors of unsigned integers.
 *
 * I have decided that I do not need to shrink the ROI if I am rebinning, \n
 * the rebinned frame is calculated from the uncorrected one, via the ROI Mask anyway
 *
 * @todo add examples how to iterate over the frame (in principle pnccd_analysis.cpp is full thereof)
 * @todo refine documentation
 * @author Nicola Coppola
 */
class Mask
{
public:
  /** default constructor */
  Mask(/*const std::string detectorname*/) {}

  /** an region of interest entity */
  typedef detROI_ detROI_t;
  /** the ROI mask for each detector */
  typedef std::vector<uint16_t> ROImask_t;
  /** the ROI index-pointer-mask for each detector */
  typedef std::vector<uint32_t> ROIiterator_t;
  /** the transformed (in the input original shape) ROI mask for each detector */
  typedef std::vector<uint16_t> ROImask_converter_t;
  /** the transformed (in the input original shape) ROI index-pointer-mask for each detector */
  typedef std::vector<uint32_t> ROIiterator_converter_t;
  /** creating the roi entities.
   * This functor will calculate the requested entities.
   *
   * @param detectorname a unique name for which detector we create the mask
   *        we need this info, to be able to extract the right info from cass.ini
   *        (for the moment this is not needed, as the way that would allow to use it
   *         is clearly NOT nice)
   */
  /** the vector with the ROI(s) "inside" */
  detROI_t                  _detROI;
  ROImask_t                 _ROImask;
  ROIiterator_t             _ROIiterator;
  ROIiterator_t             _ROIiterator_pp;
  ROImask_converter_t       _ROImask_converter;
  ROIiterator_converter_t   _ROIiterator_converter;
  //static detROI_t create(const std::string detectorname)const;
  //static ROImask_t create(const std::string detectorname)const;
  //static ROIiterator_t create(const std::string detectorname)const;
  /** this function creates the region of interest */
  /*static*/ void load(/*detROI_**/);
  void save();
};

#endif
