// Copyright (C) 2010 Nicola Coppola
// Copyright (C) 2010 Lutz Foucar

#include <stdint.h>
#include <vector>
#include <string>


namespace cass
{

  /** functor creating a region of interest.
   * a region of interest is created from a list of simple shapes. Each simple shape
   * needs the following "attributes": shape, xsize, ysize, xcenter, ycenter
   * there are several shapes types:=circ(==circle),triangle(isosceles),square.
   * The attributes will be loaded from cass.ini.
   * @note Do I need many squares per frame?
   * @warning there is a problem with a triangular shape... the orientation!!!
   *
   * the orientation is used only in the case of a triangular shape.
   *   /\           ----         |\           /|
   *  /  \  ==+1    \  /  ==-1   | \  ==+2   / | == -2
   *  ----           \/          | /         \ |
   *                             |/           \|
   * if I rotate the plane by -pi/2: -2=>+1 1=>+2 -1=>-2  +2=>-1
   *
   * @note please remember to use the rotated frame wrt standard-natural frame
   * orientation!!
   *
   * @note I think also a "double triangle bottle-like shape could be helpful
   * xsize,ysize and center are in pixel units
   * this will create
   * - a ROI Mask
   * - a ROI Iterator, which is a list of indizes of the frame that are not
   *   masked as uniteresting
   * - a ROI Mask Converter, which is the transformed (in the input original
   *   shape) ROI mask for each detector
   * - a ROI Iterator Converter, which is the transformed (in the input
   *   original shape) ROI index-pointer-mask for each detector
   * depending on the input parameter. All of these entities are vectors of unsigned
   * integers.
   * The info how to create the
   * @note Do I need to shrink the ROI if I am rebinning??
   * @todo explain what the different kind of entities are and what they do.
   * @todo decide whether this should be moved to another namespace
   * @example
   * //create a list of "good" indizes in the frame//
   * //for the first pnccd detector//
   * cass::ROI::roi_t roi = cass::ROI::create("pnCCD01",cass::ROI::ROIIterator);
   * @todo add to the example how to iterate over the frame
   * @author Nicola Coppola
   */
  struct ROI
  {
    /** enum for the different entity types*/
    enum entity_t {ROIMask, ROIIterator, ROIMaskConverter, ROIIteratorConverter};
    /** an region of interest entitiy */
    typedef std::vector<uint32_t> roi_t;
    /** creating the roi entity.
     * This functor will create the requested entity.
     * @return the requested entity
     * @param detectorname a unique name for which detector we create the mask
     *        we need this info, to be able to extract the right info from cass.ini
     * @param entity tells the functor what type of entity we want it to return
     */
    static roi_t create(const std::string detectorname, entity_t entity)const;
  };

}
