// Copyright (C) 2010 Lutz Foucar

#ifndef _OPERATIONS_H_
#define _OPERATIONS_H_

#include "postprocessing/backend.h"
#include "cass_event.h"




namespace cass
{
  // forward declaration
  class Histogram0DFloat;
  class Histogram1DFloat;
  class Histogram2DFloat;

  /** Difference between choosable histograms.
   *
   * This histogram will create a histogram which is the result of substracting
   * histogram in pp with id one from histogram in pp with id two. The resulting
   * histogram will follow the formular:
   * \f$Result = One * f_{one}  - Two * f_{two}\f$
   * where \f$f_{one}\f$ and \f$f_{two}\f$ are factors that one can weight the
   * first and second part with.
   *
   * The resulting histogram will be created using the size and dimension of the
   * first histogram.
   *
   * @cassttng PostProcessor/p\%id\%/{HistOne|HistTwo} \n
   *           the postprocessor id's that contain the first histogram and second
   *           histogram for the substraction. Default is 0 for both. This
   *           will result in an exception. Since pp 0 is not implemented.
   * @cassttng PostProcessor/p\%id\%/{FactorOne|FactorTwo} \n
   *           The factors that will weight the substraction. The default is 1.
   *
   * Implements postprocessors id's 106, 107
   *
   * @author Jochen Kuepper
   * @author Lutz Foucar
   */
  class pp106 : public PostprocessorBackend
  {
  public:

    pp106(PostProcessors& hist, PostProcessors::id_t id);

    /** Free _image space */
    virtual ~pp106();

    /** copy image from CASS event to histogram storage */
    virtual void operator()(const CASSEvent&);

    virtual void loadSettings(size_t);

    /** the two histograms that the user wants to substract */
    virtual std::list<PostProcessors::id_t> dependencies();

  protected:

    /** factor by which the first histogram will be weighted */
    float _fOne;

    /** factor by which the second histogram will be weighted */
    float _fTwo;

    /** how many pixels to bin in horizontal and vertical direction */
    PostProcessors::id_t _idOne;

    /** how many pixels to bin in horizontal and vertical direction */
    PostProcessors::id_t _idTwo;

    /** resulting image */
    Histogram2DFloat *_image;
  };

}

#endif
