// Copyright (C) 2010 Stephan Kassemeyer

#ifndef _HITRATE_H_
#define _HITRATE_H_

#include "backend.h"
#include "cass_event.h"
#include "cass_acqiris.h"

namespace cass
{

  // forward declaration
  class Histogram0DFloat;
  class Histogram1DFloat;
  class Histogram2DFloat;

  /** Single particle hit.
   *
   * detect Single Particle hits.
   *
   * @cassttng PostProcessor/\%name\%/{HistName}\n
   *           Postprocessor name containing the histogram in which hits should be detected.
   * @cassttng PostProcessor/\%name\%/{threshold}\n
   *           Threshold for outlier detection based discrimination (mahalanobis distance to dataset) default = 1.0 .
   * @cassttng PostProcessor/\%name\%/{xstart}\n
   *           ROI for calculations. First pixel = 0 (default).
   * @cassttng PostProcessor/\%name\%/{ystart}\n
   *           ROI for calculations. First pixel = 0 (default).
   * @cassttng PostProcessor/\%name\%/{xend}\n
   *           ROI for calculations. Last pixel = -1 (default).
   * @cassttng PostProcessor/\%name\%/{yend}\n
   *           ROI for calculations. Last pixel = -1 (default).
   *
   * @author Stephan Kassemeyer
   */

  class pp589 : public PostprocessorBackend
  {
  public:
    /** constructor */
    pp589(PostProcessors&, const PostProcessors::key_t&);

    /** destructor */
    virtual ~pp589();

   /** check if image specified in settings contains a single particle hit*/
    virtual void operator()(const CASSEvent&);

    /** load the settings for this pp */
    virtual void loadSettings(size_t);

    /** the dependencies on input histogram */
    virtual PostProcessors::active_t dependencies();

  protected:
    /** xstart - ROI for calculations*/
    int _xstart;
    /** ystart - ROI for calculations*/
    int _ystart;
    /** xend - ROI for calculations*/
    int _xend;
    /** yend - ROI for calculations*/
    int _yend;

    /** Threshold for outlier detection based discrimination (mahalanobis distance to dataset). */
    float _threshold;

    /** the histogram to work on */
    PostProcessors::key_t _idHist;

    /** result */
    Histogram0DFloat *_result;

    /** storage for integralimage (2d integral of entire image) */
    Histogram2DFloat* _integralimg;

    /** storage for row sum (1d integral of image) */
    Histogram2DFloat* _rowsum;
  };


}
#endif
