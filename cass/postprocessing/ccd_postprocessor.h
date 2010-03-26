// Copyright (C) 2010 lmf
// Copyright (C) 2010 Jochen Küpper

#ifndef _CCD_POSTPROCESSOR_H_
#define _CCD_POSTPROCESSOR_H_

#include "postprocessor_backend.h"

namespace cass
{
  //forward declaration
  class Histogram2DFloat;

  class PostprocessorPnccdLastImage : public PostprocessorBackend
  {
  public:

    PostprocessorPnccdLastImage(PostProcessors::histograms_t&, PostProcessors::id_t);

    /** Free _image spcae */
    virtual ~PostprocessorPnccdLastImage();

    /** copy image from CASS event to histogram storage */
    virtual void operator()(const CASSEvent&);


  protected:

    size_t _detector;

    Histogram2DFloat *_image;
  };



  class PostprocessorPnccdBinnedRunningAverage : public PostprocessorBackend
  {
  public:

    PostprocessorPnccdBinnedRunningAverage(PostProcessors::histograms_t& hist, PostProcessors::id_t id);

    /** Free _image spcae */
    virtual ~PostprocessorPnccdBinnedRunningAverage();

    /** copy image from CASS event to histogram storage */
    virtual void operator()(const CASSEvent&);

    virtual void loadSettings(size_t);

  protected:

    /** how many pixels to bin in vertical and horizontal direction */
    std::pair<unsigned, unsigned> _binning;

    /** pnCCD detector to work on */
    size_t _detector;

    /** current image */
    Histogram2DFloat *_image;
  };

}

#endif

