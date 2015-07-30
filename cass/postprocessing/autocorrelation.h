// Copyright (C) 2013 Lutz Foucar

/**
 * @file autocorrelation.h containing the class to calculate the
 *                         autocorrelation of a 2d histogram
 *
 * @author Lutz Foucar
 */

#ifndef _AUTOCORRELATION_H_
#define _AUTOCORRELATION_H_

#include "processor.h"
#include "histogram.h"

namespace cass
{

/** calculate the autocorrelation of an image in radial coordinates
 *
 * details
 *
 * @PPList "310": calculate the autocorrelation of an image in radial coordinates
 *
 * @cassttng Processor/\%name\%/{HistName} \n
 *           processor name containing the histogram whos autocorrelation
 *           should be calculated. The radius should be along the y-axis and the
 *           phi should be along the x-axis.
 *
 * @author Aliakbar Jafarpour
 * @author Stephan Kassemeyer
 * @author Lutz Foucar
 */
class pp310 : public Processor
{
public:
  /** constructor */
  pp310(const name_t &);

  /** process the event */
  virtual void process(const CASSEvent&, HistogramBackend &result);

  /** load the settings of this pp */
  virtual void loadSettings(size_t);

protected:
  /** pp containing histogram to calculate the autocorrelation for */
  shared_pointer _hist;
};


/** calculate the autocorrelation of an image
 *
 * details
 *
 * @PPList "311": calculate the autocorrelation of an image
 *
 * @cassttng Processor/\%name\%/{CenterX|CenterY} \n
 * @cassttng Processor/\%name\%/{MaximumRadius} \n
 * @cassttng Processor/\%name\%/{HistName} \n
 *           processor name containing the histogram whos autocorrelation
 *           should be calculated.
 *
 * @author Stephan Kassemeyer
 * @author Lutz Foucar
 */
class pp311 : public Processor
{
public:
  /** constructor */
  pp311(const name_t &);

  /** process the event */
  virtual void process(const CASSEvent&, HistogramBackend &result);

  /** load the settings of this pp */
  virtual void loadSettings(size_t);

protected:
  /** pp containing histogram to calculate the autocorrelation for */
  shared_pointer _hist;

private:
  /** define a ring that knows its position in the original image and the value
   *  at that position
   */
  typedef std::vector<std::pair<int,HistogramFloatBase::storage_t::value_type> > ring_t;

  /** retrieve the length of the ring for a given radius
   *
   * @return the amount of pixels that belong to the radius
   * @param rad the radius that one wants to find the pixels for
   */
  int getCircleLength(const int rad);

  /** fill the ring with the pixels that belong to a certain radius around a
   *  given center
   *
   * @note the ring that is passed to this function should already contain
   *       enough elements as it will not be refilled.
   *
   * @param image the original image
   * @param rad the radius for which the ring should be found
   * @param x0 the center in x
   * @param y0 the center in y
   * @param nxx the number of columns of the original image
   * @param ring the ring that will contain the result
   */
  void fillRing(const HistogramFloatBase::storage_t &image,
                const int rad, const int x0, const int y0, const int nxx,
                ring_t &ring);

  /** the used center of the image */
  std::pair<int,int> _center;

  /** the used maximum radius*/
  int _maxrad;

};
} //end namespace cass
#endif
