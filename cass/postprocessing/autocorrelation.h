// Copyright (C) 2013 Lutz Foucar

/**
 * @file autocorrelation.h containing the class to calculate the
 *                         autocorrelation of a 2d histogram
 *
 * @author Lutz Foucar
 */

#ifndef _AUTOCORRELATION_H_
#define _AUTOCORRELATION_H_

#include "backend.h"

namespace cass
{

/** calculate the autocorrelation of an image in radial coordinates
 *
 * details
 *
 * @cassttng PostProcessor/\%name\%/{HistName} \n
 *           Postprocessor name containing the histogram whos autocorrelation
 *           should be calculated. The radius should be along the y-axis and the
 *           phi should be along the x-axis.
 *
 * @author Aliakbar Jafarpour
 * @author Stephan Kassemeyer
 * @author Lutz Foucar
 */
class pp310 : public PostprocessorBackend
{
public:
  /** constructor
   *
   * @param pp reference to the postprocessor manager
   * @param key the name of this PostProce
   */
  pp310(PostProcessors &pp, const PostProcessors::key_t &key);

  /** process the event */
  virtual void process(const CASSEvent&);

  /** load the settings of this pp */
  virtual void loadSettings(size_t);

protected:
  /** pp containing histogram to calculate the autocorrelation for */
  PostprocessorBackend *_hist;
};
} //end namespace cass
#endif
