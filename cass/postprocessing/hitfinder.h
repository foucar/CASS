// Copyright (C) 2013 Lutz Foucar

/** @file hitfinder.h contains postprocessors that will extract pixels of
 *                    interrest from 2d histograms.
 * @author Lutz Foucar
 */

#ifndef _HITFINDER_H_
#define _HITFINDER_H_

#include "backend.h"
#include "cass_event.h"
#include "histogram.h"



namespace cass
{


/** get the local background from image.
 *
 * splits up the image into sections of user choosable size. In each of these
 * sections the local background is determined by taking a box of a user
 * choosable size and determining the median of the pixel values inside this
 * box.
 *
 * @see PostprocessorBackend for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng PostProcessor/\%name\%/{HistName} \n
 *           the postprocessor name that contain the 2d histogram. Default
 *           is "blubb".
 * @cassttng PostProcessor/\%name\%/{SectionSizeX|SectionSizeY} \n
 *           Size of the subsection of the image. Default is 1024|512.
 * @cassttng PostProcessor/\%name\%/{BoxSizeX|BoxSizeY} \n
 *           size in x and y of the box that is used for determining the median
 *           background. Default is 10|10.
 *
 * @author Lutz Foucar
 * @author Wolfgang Kabsch
 */
class pp203 : public PostprocessorBackend
{
public:
  /** constructor */
  pp203(PostProcessors& hist, const PostProcessors::key_t&);

  /** process event */
  virtual void process(const CASSEvent&);

  /** load the settings of this pp */
  virtual void loadSettings(size_t);

protected:
  /** change own histograms when one of the ones we depend on has changed histograms */
  virtual void histogramsChanged(const HistogramBackend*);

  /** pp containing 2d histogram */
  PostprocessorBackend *_hist;

  /** the size of the box used for the median filter */
  std::pair<size_t,size_t> _boxSize;

  /** size of a image section */
  std::pair<size_t,size_t> _sectionSize;

private:
  /** set up the histogram */
  void setup(const HistogramBackend &hist);
};

}//end namespace cass

#endif
