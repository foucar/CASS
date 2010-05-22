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

    /** id of first histogram */
    PostProcessors::id_t _idOne;

    /** id of second histogram */
    PostProcessors::id_t _idTwo;

    /** resulting histgram */
    HistogramFloatBase *_result;
  };






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
   * Implements postprocessors id's 800
   *
   * @author Lutz Foucar
   */
  class pp800 : public PostprocessorBackend
  {
  public:
    /** constructor */
    pp800(PostProcessors& hist, PostProcessors::id_t id);

    /** Free _image space */
    virtual ~pp800();

    /** copy image from CASS event to histogram storage */
    virtual void operator()(const CASSEvent&);

    virtual void loadSettings(size_t);

    /** the two histograms that the user wants to substract */
    virtual std::list<PostProcessors::id_t> dependencies();

  protected:
    /** id of first histogram */
    PostProcessors::id_t _idOne;

    /** id of second histogram */
    PostProcessors::id_t _idTwo;

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
   * Implements postprocessors id's 801
   *
   * @author Lutz Foucar
   */
  class pp801 : public PostprocessorBackend
  {
  public:
    /** constructor */
    pp801(PostProcessors& hist, PostProcessors::id_t id);

    /** Free _image space */
    virtual ~pp801();

    /** copy image from CASS event to histogram storage */
    virtual void operator()(const CASSEvent&);

    virtual void loadSettings(size_t);

    /** the two histograms that the user wants to substract */
    virtual std::list<PostProcessors::id_t> dependencies();

  protected:
    /** how many pixels to bin in horizontal and vertical direction */
    PostProcessors::id_t _idOne;

    /** how many pixels to bin in horizontal and vertical direction */
    PostProcessors::id_t _idTwo;

    /** resulting histgram */
    Histogram0DFloat *_result;
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
   * Implements postprocessors id's 802
   *
   * @author Lutz Foucar
   */
  class pp802 : public PostprocessorBackend
  {
  public:
    /** constructor */
    pp802(PostProcessors& hist, PostProcessors::id_t id);

    /** Free _image space */
    virtual ~pp802();

    /** copy image from CASS event to histogram storage */
    virtual void operator()(const CASSEvent&);

    virtual void loadSettings(size_t);

    /** the two histograms that the user wants to substract */
    virtual std::list<PostProcessors::id_t> dependencies();

  protected:
    /** id of first histogram */
    PostProcessors::id_t _idOne;

    /** id of second histogram */
    PostProcessors::id_t _idTwo;

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
   * Implements postprocessors id's 803
   *
   * @author Lutz Foucar
   */
  class pp803 : public PostprocessorBackend
  {
  public:
    /** constructor */
    pp803(PostProcessors& hist, PostProcessors::id_t id);

    /** Free _image space */
    virtual ~pp803();

    /** copy image from CASS event to histogram storage */
    virtual void operator()(const CASSEvent&);

    virtual void loadSettings(size_t);

    /** the two histograms that the user wants to substract */
    virtual std::list<PostProcessors::id_t> dependencies();

  protected:
    /** id of first histogram */
    PostProcessors::id_t _idOne;

    /** if of second histogram */
    PostProcessors::id_t _idTwo;

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
   * Implements postprocessors id's 804
   *
   * @author Lutz Foucar
   */
  class pp804 : public PostprocessorBackend
  {
  public:
    /** constructor */
    pp804(PostProcessors& hist, PostProcessors::id_t id);

    /** Free _image space */
    virtual ~pp804();

    /** copy image from CASS event to histogram storage */
    virtual void operator()(const CASSEvent&);

    virtual void loadSettings(size_t);

    /** the two histograms that the user wants to substract */
    virtual std::list<PostProcessors::id_t> dependencies();

  protected:
    /** id of the histogram we multiply with a constant */
    PostProcessors::id_t _idHist;

    /** the factor we mulitply the histogram with */
    float _factor;

    /** resulting histgram */
    HistogramFloatBase *_result;
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
   * Implements postprocessors id's 805
   *
   * @author Lutz Foucar
   */
  class pp805 : public PostprocessorBackend
  {
  public:
    /** constructor */
    pp805(PostProcessors& hist, PostProcessors::id_t id);

    /** Free _image space */
    virtual ~pp805();

    /** copy image from CASS event to histogram storage */
    virtual void operator()(const CASSEvent&);

    /** load the settings of the pp */
    virtual void loadSettings(size_t);

    /** the two histograms that the user wants to substract */
    virtual std::list<PostProcessors::id_t> dependencies();

  protected:
    /** id of the 1d histogram we want to have the integral of */
    PostProcessors::id_t _idHist;

    /** range we want to have the integral over in histogram bins */
    std::pair<float,float> _area;

    /** resulting histgram */
    Histogram0DFloat *_result;
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
   * Implements postprocessors id's 806
   *
   * @author Lutz Foucar
   */
  class pp806 : public PostprocessorBackend
  {
  public:
    /** constructor */
    pp806(PostProcessors& hist, PostProcessors::id_t id);

    /** Free _image space */
    virtual ~pp806();

    /** copy image from CASS event to histogram storage */
    virtual void operator()(const CASSEvent&);

    /** load the settings of the pp */
    virtual void loadSettings(size_t);

    /** the two histograms that the user wants to substract */
    virtual std::list<PostProcessors::id_t> dependencies();

  protected:
    /** the id of the 2d hist we want to project */
    PostProcessors::id_t _idHist;

    /** range we want to project */
    std::pair<float,float> _range;

    /** axis we want to project on */
    size_t _axis;

    /** resulting histgram */
    Histogram1DFloat *_projec;
  };


  /** Radial Average/Projection of 2d Histogram.
   *
   * @cassttng PostProcessor/p\%id\%/{HistId} \n
   *           Postprocessor id with 2D-Histogram that we create project.
   *           Default is 0.
   * @cassttng PostProcessor/p\%id\%/{XCentre|YCentre} \n
   *           Xcoordinate and Y coordinate of the centre. Default is
   *           512,512
   * @cassttng PostProcessor/p\%id\%/{LowerBound|UpperBound} \n
   *           Upper and lower bound of the area to project. Default is
   *           0 ... radius
   * @cassttng PostProcessor/p\%id\%/{Radius} \n
   *           Default is 1024.
   *           Possible choises are:
   *           - 0 to Infinite
   *           in case that the value is too large for the histogram this
   *           value will be automatically reduced
   *
   * Implements postprocessors id's 807
   *
   * @author Nicola Coppola
   */
  class pp807 : public PostprocessorBackend
  {
  public:
    /** constructor */
    pp807(PostProcessors& hist, PostProcessors::id_t id);

    /** Free _image space */
    virtual ~pp807();

    /** copy image from CASS event to histogram storage */
    virtual void operator()(const CASSEvent&);

    /** load the settings of the pp */
    virtual void loadSettings(size_t);

    /** the two histograms that the user wants to substract */
    virtual std::list<PostProcessors::id_t> dependencies();

  protected:
    /** the id of the 2d hist we want to project */
    PostProcessors::id_t _idHist;

    /** distance's range we want to see displayed */
    std::pair<float,float> _range;

    /** axis we want to project on */
    size_t _axis;

    /** centre's coordinates we use to calculate the radial average */
    std::pair<float,float> _centre;

    /** max Radius we want to use */
    size_t _radius;

    /** resulting histgram */
    Histogram1DFloat *_projec;
  };

  /** Radar Plot of 2d Histogram.
   *
   * @cassttng PostProcessor/p\%id\%/{HistId} \n
   *           Postprocessor id with 2D-Histogram that we create project.
   *           Default is 0.
   * @cassttng PostProcessor/p\%id\%/{LowerBound|UpperBound} \n
   *           Upper and lower bound of the area to project. Default is
   *           0 ... radius
   * @cassttng PostProcessor/p\%id\%/{Radii} \n
   *           Default is 0,1024.
   *           Possible choices are:
   *           - 0 to Infinite
   *           in case that the value is too large for the histogram this
   *           value will be automatically reduced
   *
   * Implements postprocessors id's 808
   *
   * @author Nicola Coppola
   */
  class pp808 : public PostprocessorBackend
  {
  public:
    /** constructor */
    pp808(PostProcessors& hist, PostProcessors::id_t id);

    /** Free _image space */
    virtual ~pp808();

    /** copy image from CASS event to histogram storage */
    virtual void operator()(const CASSEvent&);

    /** load the settings of the pp */
    virtual void loadSettings(size_t);

    /** the two histograms that the user wants to substract */
    virtual std::list<PostProcessors::id_t> dependencies();

  protected:
    /** the id of the 2d hist we want to project */
    PostProcessors::id_t _idHist;

    /** inner outter radii we want to use displayed */
    std::pair<float,float> _range;
    std::pair<float,float> _radii;

    /** axis we want to project on */
    //size_t _axis;

    /** centre's coordinates we use to calculate the radar plot */
    std::pair<float,float> _centre;

    /** resulting histgram */
    Histogram1DFloat *_projec;
  };


}

#endif
