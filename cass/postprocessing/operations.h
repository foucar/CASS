// Copyright (C) 2010 Lutz Foucar

#ifndef _OPERATIONS_H_
#define _OPERATIONS_H_

#include "backend.h"
#include "cass_event.h"




namespace cass
{
  // forward declaration
  class Histogram0DFloat;
  class Histogram1DFloat;
  class Histogram2DFloat;
  class HistogramFloatBase;




  /** Compare two histograms for less.
   *
   * \f$result = hist_{one}  < hist_{two}\f$
   * where \f$hist_{one}\f$ and \f$hist_{two}\f$ are histograms one or two
   * respectivly
   *
   * @cassttng PostProcessor/p\%id\%/{HistOne|HistTwo} \n
   *           the postprocessor id's that contain the first histogram and second
   *           histogram for the less comparison. Default is 0 for both. This
   *           will result in an exception. Since pp 0 is not implemented.
   *
   * @author Lutz Foucar
   */
  class pp7 : public PostprocessorBackend
  {
  public:
    /** constructor */
    pp7(PostProcessors& hist, const PostProcessors::key_t&);

    /** Free _image space */
    virtual ~pp7();

    /** copy image from CASS event to histogram storage */
    virtual void operator()(const CASSEvent&);

    virtual void loadSettings(size_t);

    /** the two histograms that the user wants to substract */
    virtual PostProcessors::active_t dependencies();

  protected:
    /** id of first histogram */
    PostProcessors::key_t _idOne;

    /** id of second histogram */
    PostProcessors::key_t _idTwo;

    /** resulting histgram */
    Histogram0DFloat *_result;
  };







  /** Compare two histograms for equality.
   *
   * \f$result = hist_{one}  == hist_{two}\f$
   * where \f$hist_{one}\f$ and \f$hist_{two}\f$ are histograms one or two
   * respectivly
   *
   * @cassttng PostProcessor/p\%id\%/{HistOne|HistTwo} \n
   *           the postprocessor id's that contain the first histogram and second
   *           histogram for the operation. Default is 0 for both. This
   *           will result in an exception. Since pp 0 is not implemented.
   *
   * @author Lutz Foucar
   */
  class pp8 : public PostprocessorBackend
  {
  public:
    /** constructor */
    pp8(PostProcessors& hist, const PostProcessors::key_t&);

    /** Free _image space */
    virtual ~pp8();

    /** copy image from CASS event to histogram storage */
    virtual void operator()(const CASSEvent&);

    virtual void loadSettings(size_t);

    /** the two histograms that the user wants to substract */
    virtual PostProcessors::active_t dependencies();

  protected:
    /** how many pixels to bin in horizontal and vertical direction */
    PostProcessors::key_t _idOne;

    /** how many pixels to bin in horizontal and vertical direction */
    PostProcessors::key_t _idTwo;

    /** resulting histgram */
    Histogram0DFloat *_result;
  };









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
   * @author Lutz Foucar
   */
  class pp20 : public PostprocessorBackend
  {
  public:
    /** constructor */
    pp20(PostProcessors& hist, const PostProcessors::key_t&);

    /** Free _image space */
    virtual ~pp20();

    /** copy image from CASS event to histogram storage */
    virtual void operator()(const CASSEvent&);

    virtual void loadSettings(size_t);

    /** the two histograms that the user wants to substract */
    virtual PostProcessors::active_t dependencies();

  protected:
    /** factor by which the first histogram will be weighted */
    float _fOne;

    /** factor by which the second histogram will be weighted */
    float _fTwo;

    /** id of first histogram */
    PostProcessors::key_t _idOne;

    /** id of second histogram */
    PostProcessors::key_t _idTwo;

    /** resulting histgram */
    HistogramFloatBase *_result;
  };









  /** Divide two histograms.
   *
   * \f$result = \frac{hist_{one}}{hist_{two}}\f$
   * where \f$hist_{one}\f$ and \f$hist_{two}\f$ are histograms one or two
   * respectivly
   *
   * The resulting histogram will be created using the size and dimension of the
   * first histogram.
   *
   * @cassttng PostProcessor/p\%id\%/{HistOne|HistTwo} \n
   *           the postprocessor id's that contain the first histogram and second
   *           histogram for the operation. Default is 0 for both. This
   *           will result in an exception. Since pp 0 is not implemented.
   *
   * @author Lutz Foucar
   */
  class pp21 : public PostprocessorBackend
  {
  public:
    /** constructor */
    pp21(PostProcessors& hist, const PostProcessors::key_t&);

    /** Free _image space */
    virtual ~pp21();

    /** copy image from CASS event to histogram storage */
    virtual void operator()(const CASSEvent&);

    virtual void loadSettings(size_t);

    /** the two histograms that the user wants to substract */
    virtual PostProcessors::active_t dependencies();

  protected:
    /** id of first histogram */
    PostProcessors::key_t _idOne;

    /** id of second histogram */
    PostProcessors::key_t _idTwo;

    /** resulting histgram */
    HistogramFloatBase *_result;
  };








  /** Multiply two histograms.
   *
   * \f$result = hist_{one} \times hist_{two}\f$
   * where \f$hist_{one}\f$ and \f$hist_{two}\f$ are histograms one or two
   * respectivly
   *
   * The resulting histogram will be created using the size and dimension of the
   * first histogram.
   *
   * @cassttng PostProcessor/p\%id\%/{HistOne|HistTwo} \n
   *           the postprocessor id's that contain the first histogram and second
   *           histogram for the operation. Default is 0 for both. This
   *           will result in an exception. Since pp 0 is not implemented.
   *
   * @author Lutz Foucar
   */
  class pp22 : public PostprocessorBackend
  {
  public:
    /** constructor */
    pp22(PostProcessors& hist, const PostProcessors::key_t&);

    /** Free _image space */
    virtual ~pp22();

    /** copy image from CASS event to histogram storage */
    virtual void operator()(const CASSEvent&);

    virtual void loadSettings(size_t);

    /** the two histograms that the user wants to substract */
    virtual PostProcessors::active_t dependencies();

  protected:
    /** id of first histogram */
    PostProcessors::key_t _idOne;

    /** if of second histogram */
    PostProcessors::key_t _idTwo;

    /** resulting histgram */
    HistogramFloatBase *_result;
  };









  /** Multiply histogram with constant.
   *
   * @cassttng PostProcessor/p\%id\%/{HistId} \n
   *           Postprocessor id with histogram that should be multiplied. Default is 0.
   * @cassttng PostProcessor/p\%id\%/{Factor} \n
   *           Factor with which histogram should be multiplied. Default is 1.
   *
   * @author Lutz Foucar
   */
  class pp23 : public PostprocessorBackend
  {
  public:
    /** constructor */
    pp23(PostProcessors& hist, const PostProcessors::key_t&);

    /** Free _image space */
    virtual ~pp23();

    /** copy image from CASS event to histogram storage */
    virtual void operator()(const CASSEvent&);

    virtual void loadSettings(size_t);

    /** the two histograms that the user wants to substract */
    virtual PostProcessors::active_t dependencies();

  protected:
    /** id of the histogram we multiply with a constant */
    PostProcessors::key_t _idHist;

    /** the factor we mulitply the histogram with */
    float _factor;

    /** resulting histgram */
    HistogramFloatBase *_result;
  };












  /** Projection of 2d Histogram.
   *
   * @cassttng PostProcessor/p\%id\%/{HistId} \n
   *           Postprocessor id with 2D-Histogram that we create project.
   *           Default is 0.
   * @cassttng PostProcessor/p\%id\%/{LowerBound|UpperBound} \n
   *           Upper and lower bound of the area to project. Default is
   *           -1e6 ... 1e6
   * @cassttng PostProcessor/p\%id\%/{Axis} \n
   *           The axis we want to project to. Default is xAxis.
   *           Possible choises are:
   *           - 0:xAxis
   *           - 1:yAxis
   *
   * @author Lutz Foucar
   */
  class pp50 : public PostprocessorBackend
  {
  public:
    /** constructor */
    pp50(PostProcessors& hist, const PostProcessors::key_t&);

    /** Free _image space */
    virtual ~pp50();

    /** copy image from CASS event to histogram storage */
    virtual void operator()(const CASSEvent&);

    /** load the settings of the pp */
    virtual void loadSettings(size_t);

    /** the two histograms that the user wants to substract */
    virtual PostProcessors::active_t dependencies();

  protected:
    /** the id of the 2d hist we want to project */
    PostProcessors::key_t _idHist;

    /** range we want to project */
    std::pair<float,float> _range;

    /** axis we want to project on */
    size_t _axis;

    /** resulting histgram */
    Histogram1DFloat *_projec;
  };










  /** Integral of 1d Histogram.
   *
   * \f$result = hist_{one} \times f\f$
   * where \f$hist_{one}\f$ is the histogram to be multiplied by f
   *
   * The resulting histogram will be created using the size and dimension of the
   * first histogram.
   *
   * @cassttng PostProcessor/p\%id\%/{HistId} \n
   *           Postprocessor id with 1D-Histogram that we create the intgral from Default is 0.
   * @cassttng PostProcessor/p\%id\%/{LowerBound|UpperBound} \n
   *           Upper and lower bound of the area to integrate. Default is -1e6 ... 1e6
   *
   * @author Lutz Foucar
   */
  class pp51 : public PostprocessorBackend
  {
  public:
    /** constructor */
    pp51(PostProcessors& hist, const PostProcessors::key_t&);

    /** Free _image space */
    virtual ~pp51();

    /** copy image from CASS event to histogram storage */
    virtual void operator()(const CASSEvent&);

    /** load the settings of the pp */
    virtual void loadSettings(size_t);

    /** the two histograms that the user wants to substract */
    virtual PostProcessors::active_t dependencies();

  protected:
    /** id of the 1d histogram we want to have the integral of */
    PostProcessors::key_t _idHist;

    /** range we want to have the integral over in histogram bins */
    std::pair<float,float> _area;

    /** resulting histgram */
    Histogram0DFloat *_result;
  };

}

#endif
