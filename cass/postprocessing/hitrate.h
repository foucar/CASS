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
   *           Threshold for outlier detection based discrimination (mahalanobis distance to dataset).
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
    /** Threshold for outlier detection based discrimination (mahalanobis distance to dataset). */
    float _threshold;

    /** the histogram to work on */
    PostProcessors::key_t _idHist;

    /** result */
    Histogram0DFloat *_result;
  };


}
#endif
