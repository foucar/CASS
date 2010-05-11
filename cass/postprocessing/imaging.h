// Copyright (C) 2010 Lutz Foucar

#ifndef _IMAGING_H_
#define _IMAGING_H_

#include "postprocessing/backend.h"
#include "cass_event.h"
#include "cass_acqiris.h"

namespace cass
{
  // forward declaration
  class Histogram0DFloat;
  class Histogram1DFloat;
  class Histogram2DFloat;






  /** Advanced peakfinding on front pnCCD.
   *
   * Take the averaged front pnccd image (104) and make a condition on yag on.
   * Create an image that is the result of the current image substracted by
   * the averaged image. Where the averaged image is weighted with the sum of
   * all pixels in the current image divided by the sum of all pixels in the
   * averaged image. \n
   * Check which pixels in the resulting image are above a given threshold and
   * the the requested image with One. It follows the function:
   * \f$ Pixel_{resulting Image} +=
   *           (Pixel_{current Image) - \apha \times Pixel_{running Average}\f$
   *
   * @cassttng PostProcessor/p\%id\%/{Threshold} \n
   *           The threshold which will quantify whether there is a photon
   *           in the current image.
   *
   * Implements postprocessors id's 160
   *
   * @todo make it get more usersettable parameters so that it can be reused
   * @author Lutz Foucar
   */
  class pp160 : public PostprocessorBackend
  {
  public:
    /** constructor. */
    pp160(PostProcessors& hist, PostProcessors::id_t id);
    /** Free _image space */
    virtual ~pp160();
    /** copy image from CASS event to histogram storage */
    virtual void operator()(const CASSEvent&);
    /** load the settings*/
    virtual void loadSettings(size_t);
    /** the two histograms that the user wants to substract */
    virtual std::list<PostProcessors::id_t> dependencies();

  protected:
    /** the Detector that we make the condition on*/
    ACQIRIS::Detectors _conditionDetector;
    /** threshold for peakfinding */
    float _threshold;
    /** the id of the averaged image */
    PostProcessors::id_t _idAverage;
    /** resulting image */
    Histogram2DFloat *_image;
  };







}



#endif
