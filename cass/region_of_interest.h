// Copyright (C) 2010 Nicola Coppola
// Copyright (C) 2010 Lutz Foucar

#include <stdint.h>
#include <vector>
#include <string>


namespace cass
{

  /** functor creating a region of interest.
   * a region of interest is created from a list of simple shapes.
   * @see cass::ROIShape
   * @note Do I need many squares per frame?
   *
   * this function will create
   * - a ROI Mask
   * - a ROI Iterator, which is a list of indizes of the frame that are not
   *   masked as uniteresting
   * - a ROI Mask Converter, which is the transformed (in the input original
   *   shape) ROI mask for each detector
   * - a ROI Iterator Converter, which is the transformed (in the input
   *   original shape) ROI index-pointer-mask for each detector
   * depending on the input parameter. All of these entities are vectors of unsigned
   * integers.
   *
   * Example usage:
   * @code
   * //create a list of "good indizes in the frame//
   * //for the first pnccd detector//
   * cass::ROI::roi_t roi = cass::ROI::create("pnCCD01",cass::ROI::ROIIterator);
   * @endcode
   *
   * @note Do I need to shrink the ROI if I am rebinning??
   * @todo explain what the different kind of entities are and what they do.
   * @todo decide whether this should be moved to another namespace, maybe
   *       it should be just a function instead of a struct? (lutz)
   * @todo add examples how to iterate over the frame
   * @todo cleanup the documentation make it more clear
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
