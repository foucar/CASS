// Copyright (C) 2010 Lutz Foucar

/** @file operations.h file contains postprocessors that will operate
 *                     on histograms of other postprocessors
 * @todo add pp that will maks out given areas of a histogram
 * @todo add pp that has input and condition output is copy of input
 * @author Lutz Foucar
 */

#ifndef _OPERATIONS_H_
#define _OPERATIONS_H_

#include "backend.h"
#include "cass_event.h"
#include "histogram.h"
#include <time.h>

namespace cass
{


  /** Compare histogram for less than constant.
   *
   * @cassttng PostProcessor/\%name\%/{HistOne} \n
   *           the postprocessor name that contain the first histogram. Default
   *           is 0.
   * @cassttng PostProcessor/\%name\%/{Value} \n
   *           Value to compare the histograms value to. Default is 0.
   *
   * @author Lutz Foucar
   */
  class pp1 : public PostprocessorBackend
  {
  public:
    /** constructor */
    pp1(PostProcessors& hist, const PostProcessors::key_t&);

    /** process event */
    virtual void process(const CASSEvent&);

    /** load the settings of this pp */
    virtual void loadSettings(size_t);

  protected:
    /** pp containing the histogram */
    PostprocessorBackend *_one;

    /** constant value to compare to */
    float _value;
  };







  /** Compare histogram for greater than constant.
   *
   * @cassttng PostProcessor/\%name\%/{HistOne} \n
   *           the postprocessor name that contain the first histogram. Default
   *           is 0.
   * @cassttng PostProcessor/\%name\%/{Value} \n
   *           Value to compare the histograms value to. Default is 0.
   *
   * @author Lutz Foucar
   */
  class pp2 : public PostprocessorBackend
  {
  public:
    /** constructor */
    pp2(PostProcessors& hist, const PostProcessors::key_t&);

    /** process event */
    virtual void process(const CASSEvent&);

    /** load the settings of this pp */
    virtual void loadSettings(size_t);


  protected:
    /** pp containing histogram */
    PostprocessorBackend *_one;

    /** constant value to compare to */
    float _value;
  };









  /** Compare histogram for equal to constant.
   *
   * @cassttng PostProcessor/\%name\%/{HistOne} \n
   *           the postprocessor name that contain the first histogram. Default
   *           is 0.
   * @cassttng PostProcessor/\%name\%/{Value} \n
   *           Value to compare the histograms value to. Default is 0.
   *
   * @author Lutz Foucar
   */
  class pp3 : public PostprocessorBackend
  {
  public:
    /** constructor */
    pp3(PostProcessors& hist, const PostProcessors::key_t&);

    /** process event */
    virtual void process(const CASSEvent&);

    /** load the settings of this pp */
    virtual void loadSettings(size_t);

  protected:
    /** pp containing histogram */
    PostprocessorBackend *_one;

    /** constant value to compare to */
    float _value;
  };










  /** Apply boolean NOT to 0D Histogram.
   *
   * @cassttng PostProcessor/\%name\%/{HistOne} \n
   *           the postprocessor name that contain the first histogram. Default
   *           is 0.
   *
   * @author Lutz Foucar
   */
  class pp4 : public PostprocessorBackend
  {
  public:
    /** constructor */
    pp4(PostProcessors& hist, const PostProcessors::key_t&);

    /** process event */
    virtual void process(const CASSEvent&);

    /** load the settings of this pp */
    virtual void loadSettings(size_t);

  protected:
    /** pp containing histogram */
    PostprocessorBackend *_one;
  };







  /** Boolean AND of two 0d pp.
   *
   * @cassttng PostProcessor/\%name\%/{HistOne|HistTwo} \n
   *           the postprocessor id's that contain the first histogram and second
   *           histogram for the boolean AND-ing. Default is 0 for both. This
   *           will result in an exception. Since pp 0 is not implemented.
   *
   * @author Lutz Foucar
   */
  class pp5 : public PostprocessorBackend
  {
  public:
    /** constructor */
    pp5(PostProcessors& hist, const PostProcessors::key_t&);

    /** process event */
    virtual void process(const CASSEvent&);

    /** load the settings of this pp */
    virtual void loadSettings(size_t);

  protected:
    /** pp containing first histogram */
    PostprocessorBackend *_one;

    /** pp containing second histogram */
    PostprocessorBackend *_two;
  };







  /** Boolean OR of two 0d pp.
   *
   * @cassttng PostProcessor/\%name\%/{HistOne|HistTwo} \n
   *           the postprocessor id's that contain the first histogram and second
   *           histogram for the boolean AND-ing. Default is 0 for both. This
   *           will result in an exception. Since pp 0 is not implemented.
   *
   * @author Lutz Foucar
   */
  class pp6 : public PostprocessorBackend
  {
  public:
    /** constructor */
    pp6(PostProcessors& hist, const PostProcessors::key_t&);

    /** process event */
    virtual void process(const CASSEvent&);

    /** load the settings of this pp */
    virtual void loadSettings(size_t);

  protected:
    /** pp containing first histogram */
    PostprocessorBackend *_one;

    /** pp containing second histogram */
    PostprocessorBackend *_two;
  };








  /** Compare two histograms for less.
   *
   * \f$result = hist_{one}  < hist_{two}\f$
   * where \f$hist_{one}\f$ and \f$hist_{two}\f$ are histograms one or two
   * respectivly
   *
   * @cassttng PostProcessor/\%name\%/{HistOne|HistTwo} \n
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

    /** process event */
    virtual void process(const CASSEvent&);

    /** load the settings of this pp */
    virtual void loadSettings(size_t);

  protected:
    /** id of first histogram */
    PostProcessors::key_t _idOne;

    /** id of second histogram */
    PostProcessors::key_t _idTwo;
  };







  /** Compare two histograms for equality.
   *
   * \f$result = hist_{one}  == hist_{two}\f$
   * where \f$hist_{one}\f$ and \f$hist_{two}\f$ are histograms one or two
   * respectivly
   *
   * @cassttng PostProcessor/\%name\%/{HistOne|HistTwo} \n
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

    /** process event */
    virtual void process(const CASSEvent&);

    /** load the settings of this pp */
    virtual void loadSettings(size_t);

  protected:
    /** how many pixels to bin in horizontal and vertical direction */
    PostProcessors::key_t _idOne;

    /** how many pixels to bin in horizontal and vertical direction */
    PostProcessors::key_t _idTwo;
  };








  /** Check whether histogram is in range.
   *
   * @cassttng PostProcessor/\%name\%/{HistOne} \n
   *           the postprocessor name that contain the first histogram. Default
   *           is 0.
   * @cassttng PostProcessor/\%name\%/{UpperLimit|LowerLimit} \n
   *           Upper and Lower limit of the range to check. Default is 0,0.
   *
   * @author Lutz Foucar
   */
  class pp9 : public PostprocessorBackend
  {
  public:
    /** constructor */
    pp9(PostProcessors& hist, const PostProcessors::key_t&);

    /** process event */
    virtual void process(const CASSEvent&);

    /** load the settings of this pp */
    virtual void loadSettings(size_t);

  protected:
    /** id of first histogram */
    PostProcessors::key_t _idOne;

    /** the requested range that the histogram should be in */
    std::pair<float,float> _range;
  };






  /** Constant true postprocessor.
   *
   * @author Lutz Foucar
   */
  class pp10 : public PostprocessorBackend
  {
  public:
    /** constructor */
    pp10(PostProcessors& pp, const PostProcessors::key_t &key)
      :PostprocessorBackend(pp,key)
    {
      _result = (new Histogram0DFloat(true));
      createHistList(1);
    }
    /** don't do anything to the histogram */
    virtual void process(const CASSEvent&){}
  };




  /** Constant false postprocessor.
   *
   * @author Lutz Foucar
   */
  class pp11 : public PostprocessorBackend
  {
  public:
    /** constructor */
    pp11(PostProcessors& pp, const PostProcessors::key_t &key)
      :PostprocessorBackend(pp,key)
    {
      _result = (new Histogram0DFloat(false));
      createHistList(1);
    }
    /** don't do anything to the histogram */
    virtual void process(const CASSEvent&){}
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
   * @cassttng PostProcessor/\%name\%/{HistOne|HistTwo} \n
   *           the postprocessor id's that contain the first histogram and second
   *           histogram for the substraction. Default is 0 for both. This
   *           will result in an exception. Since pp 0 is not implemented.
   * @cassttng PostProcessor/\%name\%/{FactorOne|FactorTwo} \n
   *           The factors that will weight the substraction. The default is 1.
   *
   * @author Lutz Foucar
   */
  class pp20 : public PostprocessorBackend
  {
  public:
    /** constructor */
    pp20(PostProcessors& hist, const PostProcessors::key_t&);

    /** process event */
    virtual void process(const CASSEvent&);

    /** load the settings of this pp */
    virtual void loadSettings(size_t);

  protected:
    /** factor by which the first histogram will be weighted */
    float _fOne;

    /** factor by which the second histogram will be weighted */
    float _fTwo;

    /** id of first histogram */
    PostProcessors::key_t _idOne;

    /** id of second histogram */
    PostProcessors::key_t _idTwo;
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
   * @cassttng PostProcessor/\%name\%/{HistOne|HistTwo} \n
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

    /** process event */
    virtual void process(const CASSEvent&);

    /** load the settings of this pp */
    virtual void loadSettings(size_t);

  protected:
    /** id of first histogram */
    PostProcessors::key_t _idOne;

    /** id of second histogram */
    PostProcessors::key_t _idTwo;
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
   * @cassttng PostProcessor/\%name\%/{HistOne|HistTwo} \n
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

    /** process event */
    virtual void process(const CASSEvent&);

    /** load the settings of this pp */
    virtual void loadSettings(size_t);

  protected:
    /** id of first histogram */
    PostProcessors::key_t _idOne;

    /** if of second histogram */
    PostProcessors::key_t _idTwo;
  };









  /** Multiply histogram with constant.
   *
   * @cassttng PostProcessor/\%name\%/{HistName} \n
   *           Postprocessor id with histogram that should be multiplied. Default is 0.
   * @cassttng PostProcessor/\%name\%/{Factor} \n
   *           Factor with which histogram should be multiplied. Default is 1.
   *
   * @author Lutz Foucar
   */
  class pp23 : public PostprocessorBackend
  {
  public:
    /** constructor */
    pp23(PostProcessors& hist, const PostProcessors::key_t&);

    /** process event */
    virtual void process(const CASSEvent&);

    /** load the settings of this pp */
    virtual void loadSettings(size_t);

  protected:
    /** id of the histogram we multiply with a constant */
    PostProcessors::key_t _idHist;

    /** the factor we mulitply the histogram with */
    float _factor;
  };









  /** Substract constant from histogram.
   *
   * @cassttng PostProcessor/\%name\%/{HistName} \n
   *           Postprocessor id with histogram that should be multiplied. Default is 0.
   * @cassttng PostProcessor/\%name\%/{Factor} \n
   *           Factor with which histogram should be substracted. Default is 1.
   *
   * @author Lutz Foucar
   */
  class pp24 : public PostprocessorBackend
  {
  public:
    /** constructor */
    pp24(PostProcessors& hist, const PostProcessors::key_t&);

    /** process event */
    virtual void process(const CASSEvent&);

    /** load the settings of this pp */
    virtual void loadSettings(size_t);

  protected:
    /** id of the histogram we multiply with a constant */
    PostProcessors::key_t _idHist;

    /** the factor we substract the histogram with */
    float _factor;
  };








  /** Threshold histogram.
   *
   * @cassttng PostProcessor/\%name\%/{HistName} \n
   *           Postprocessor id with histogram that should be thresholded. Default is 0.
   * @cassttng PostProcessor/\%name\%/{Threshold} \n
   *           Factor with which threshold value. Default is 0.
   *
   * @author Thomas White
   */
  class pp25 : public PostprocessorBackend
  {
  public:
    /** constructor */
    pp25(PostProcessors& hist, const PostProcessors::key_t&);

    /** process event */
    virtual void process(const CASSEvent&);

    /** load the settings of this pp */
    virtual void loadSettings(size_t);

  protected:
    /** id of the histogram we multiply with a constant */
    PostProcessors::key_t _idHist;

    /** the threshold */
    float _threshold;
  };







  /** Projection of 2d Histogram.
   *
   * @cassttng PostProcessor/\%name\%/{HistName} \n
   *           Postprocessor id with 2D-Histogram that we create project.
   *           Default is 0.
   * @cassttng PostProcessor/\%name\%/{LowerBound|UpperBound} \n
   *           Upper and lower bound of the area to project. Default is
   *           -1e6 ... 1e6
   * @cassttng PostProcessor/\%name\%/{Axis} \n
   *           The axis we want to project to. Default is xAxis.
   *           Possible choises are:
   *           - 0:xAxis
   *           - 1:yAxis
   * @cassttng PostProcessor/\%name\%/{Normalize} \n
   *           Normalize the projection, so that maximum value is always 1.
   *           Default is false.
   *
   * @author Lutz Foucar
   */
  class pp50 : public PostprocessorBackend
  {
  public:
    /** constructor */
    pp50(PostProcessors& hist, const PostProcessors::key_t&);

    /** process event */
    virtual void process(const CASSEvent&);

    /** load the settings of the pp */
    virtual void loadSettings(size_t);

  protected:
    /** pp containing the 2d hist we want to project */
    PostprocessorBackend *_pHist;

    /** range we want to project */
    std::pair<float,float> _range;

    /** flag whether we should normalize the values */
    bool _normalize;

    /** axis we want to project on */
    size_t _axis;
  };










  /** Integral of 1d Histogram.
   *
   * \f$result = hist_{one} \times f\f$
   * where \f$hist_{one}\f$ is the histogram to be multiplied by f
   *
   * The resulting histogram will be created using the size and dimension of the
   * first histogram.
   *
   * @cassttng PostProcessor/\%name\%/{HistName} \n
   *           Postprocessor id with 1D-Histogram that we create the intgral from Default is 0.
   * @cassttng PostProcessor/\%name\%/{LowerBound|UpperBound} \n
   *           Upper and lower bound of the area to integrate. Default is -1e6 ... 1e6
   *
   * @author Lutz Foucar
   */
  class pp51 : public PostprocessorBackend
  {
  public:
    /** constructor */
    pp51(PostProcessors& hist, const PostProcessors::key_t&);

    /** process event */
    virtual void process(const CASSEvent&);

    /** load the settings of the pp */
    virtual void loadSettings(size_t);

  protected:
    /** id of the 1d histogram we want to have the integral of */
    PostProcessors::key_t _idHist;

    /** range we want to have the integral over in histogram bins */
    std::pair<float,float> _area;
  };











  /** Radial Average/Projection of 2d Histogram.
   *
   * @cassttng PostProcessor/\%name\%/{HistName} \n
   *           Postprocessor id with 2D-Histogram that we create project.
   *           Default is 0.
   * @cassttng PostProcessor/\%name\%/{XCenter|YCenter} \n
   *           Xcoordinate and Y coordinate of the centre. Default is 512,512
   *
   * @todo improve and generalize radial projection to account for different distance of detector to beam line
   * @todo add treatment of possible asymmetric positions of detectors to beam line
   * @todo add possibility to have circle partially outside the physical detector dimensions
   *
   * @author Nicola Coppola
   * @author Lutz Foucar
   */
  class pp52 : public PostprocessorBackend
  {
  public:
    /** constructor */
    pp52(PostProcessors& hist, const PostProcessors::key_t&);

    /** process event */
    virtual void process(const CASSEvent&);

    /** load the settings of the pp */
    virtual void loadSettings(size_t);

  protected:
    /** the id of the 2d hist we want to project */
    PostProcessors::key_t _idHist;

    /** center coordinates we use to calculate the radial average in histogram coordinates */
    std::pair<size_t,size_t> _center;

    /** the maximum size of the radius, calculated from the two d histogram */
    size_t _radius;
  };











  /** Radar Plot of 2d Histogram.
   *
   * @cassttng PostProcessor/\%name\%/{HistName} \n
   *           Postprocessor id with 2D-Histogram that we create project.
   *           Default is 0.
   * @cassttng PostProcessor/\%name\%/{MinRadius|MaxRadius} \n
   *           Minimum and Maximum Radius to include in the polar plot. Default
   *           is 0 ... 512
   * @cassttng PostProcessor/\%name\%/{NbrBins} \n
   *           Number of Bins where the 360 degrees will be put in. Default is 360
   * @cassttng PostProcessor/\%name\%/{XCenter|YCenter} \n
   *           X and Y Center of the images polar plot. Default is 512,512
   *
   * @author Nicola Coppola
   * @author Lutz Foucar
   */
  class pp53 : public PostprocessorBackend
  {
  public:
    /** constructor */
    pp53(PostProcessors& hist, const PostProcessors::key_t&);

    /** process event */
    virtual void process(const CASSEvent&);

    /** load the settings of the pp */
    virtual void loadSettings(size_t);

  protected:
    /** the id of the 2d hist we want to project */
    PostProcessors::key_t _idHist;

    /** range of radii that we use for the angular distribution */
    std::pair<size_t,size_t> _range;

    /** centre's coordinates we use to calculate the radar plot */
    std::pair<size_t,size_t> _center;

    /** the number of bins in the resulting histogram, range is fixed */
    size_t _nbrBins;
  };















  /** Radius \f$ \phi \f$ Representation of 2D Histogram.
   *
   * @cassttng PostProcessor/\%name\%/{HistName} \n
   *           Postprocessor id with 2D-Histogram we convert.
   *           Default is 0.
   * @cassttng PostProcessor/\%name\%/{NbrAngleBins} \n
   *           Number of Bins that the 360 degrees will be put in. Default is 360
   * @cassttng PostProcessor/\%name\%/{XCenter|YCenter} \n
   *           X and Y Center of the images polar plot. Default is 512,512
   *
   * @author Lutz Foucar
   */
  class pp54 : public PostprocessorBackend
  {
  public:
    /** constructor */
    pp54(PostProcessors& hist, const PostProcessors::key_t&);

    /** process event */
    virtual void process(const CASSEvent&);

    /** load the settings of the pp */
    virtual void loadSettings(size_t);

  protected:
    /** the id of the 2d hist we want to project */
    PostProcessors::key_t _idHist;

    /** centre's coordinates we use to calculate the radar plot */
    std::pair<size_t,size_t> _center;

    /** the number of bins in the resulting histogram, range is fixed */
    size_t _nbrBins;

    /** the maximum size of the radius, calculated from the two d histogram */
    size_t _radius;
  };








  /** 0D histogramming.
   *
   * create a 1d histogram from 0d values
   *
   * @cassttng PostProcessor/\%name\%/{XNbrBins|XLow|XUp}\n
   *           properties of the 1d histogram
   * @cassttng PostProcessor/\%name\%/{HistName} \n
   *           Postprocessor name containing the 0D value to histogram
   * @cassttng PostProcessor/\%name\%/{ConditionName} \n
   *           0D Postprocessor name that we check before we histogram
   *
   * @author Lutz Foucar
   */
  class pp60 : public PostprocessorBackend
  {
  public:
    /** constructor */
    pp60(PostProcessors& hist, const PostProcessors::key_t&);

    /** process event */
    virtual void process(const CASSEvent&);

    /** load the settings */
    virtual void loadSettings(size_t);

  protected:
    /** alpha for the running average */
    float _alpha;

    /** the 0D histogram to work on */
    PostProcessors::key_t _idHist;
  };










  /** Histogram averaging.
   *
   * Running or cummulative average of a histogram.
   *
   * @cassttng PostProcessor/\%name\%/{average}\n
   *           how many images should be averaged. When value is 0 its a cummulative
   *           average. Default is 1.
   * @cassttng PostProcessor/\%name\%/{HistName} \n
   *           Postprocessor name containing the histogram that we average.
   * @cassttng PostProcessor/\%name\%/{ConditionName} \n
   *           0D Postprocessor name that we check before we start averaging
   *
   * @author Lutz Foucar
   */
  class pp61 : public PostprocessorBackend
  {
  public:
    /** constructor */
    pp61(PostProcessors& hist, const PostProcessors::key_t&);

    /** process event */
    virtual void process(const CASSEvent&);

    /** load the settings */
    virtual void loadSettings(size_t);

  protected:
    /** alpha for the running average */
    float _alpha;

    /** the histogram to work on */
    PostProcessors::key_t _idHist;
  };





  /** Histogram summing.
   *
   * Sums up histograms.
   *
   * @cassttng PostProcessor/\%name\%/{HistName} \n
   *           Postprocessor name containing the histogram that we average.
   * @cassttng PostProcessor/\%name\%/{ConditionName} \n
   *           0D Postprocessor name that we check before we start averaging
   *
   * @author Lutz Foucar
   */
  class pp62 : public PostprocessorBackend
  {
  public:
    /** constructor */
    pp62(PostProcessors& hist, const PostProcessors::key_t&);

    /** process event */
    virtual void process(const CASSEvent&);

    /** load the settings */
    virtual void loadSettings(size_t);

  protected:
    /** the histogram to work on */
    PostProcessors::key_t _idHist;
  };






  /** time average of 0d Histogram.
   *
   * Makes an running average of a given Histogram over a given time period.
   *
   * @cassttng PostProcessor/\%name\%/{HistName} \n
   *           Postprocessor id with 0D-Histogram that we create project.
   *           Default is 0.
   * @cassttng PostProcessor/\%name\%/{MinTime|MaxTime} \n
   *           Minimum and Maximum Time to plot in the histogram. Default
   *           is 0 ... 300 (WARNING: for the moment this setting is not active)
   * @cassttng PostProcessor/\%name\%/{NbrSamples} \n
   *           Number of values that are used per second to calculate the average. 
   *           Default is 5
   *
   * @author Nicola Coppola
   */
  class pp63 : public PostprocessorBackend
  {
  public:
    /** constructor */
    pp63(PostProcessors& hist, const PostProcessors::key_t&);

    /** process event */
    virtual void process(const CASSEvent&);

    /** load the settings of the pp */
    virtual void loadSettings(size_t);

  protected:
    /** the histogram to work on */
    PostProcessors::key_t _idHist;

    /** range of time that we use for the angular distribution */
    std::pair<size_t,size_t> _timerange;

    /** the number of bins in the resulting histogram, range is fixed */
    size_t _nbrSamples;

    /** resulting time average histgram */
    HistogramFloatBase *_time_avg;

    /** the number of samples seen up to now and used in the point */
    size_t _num_seen_evt;

    /** time when the first samples was used in the point in time */
    time_t _when_first_evt;
    uint32_t _first_fiducials;
  };





  /** record 0d Histogram into 1d Histogram.
   *
   * appends values from 0d histogram at end of 1d histogram and shifts the old values to the left.
   *
   * @cassttng PostProcessor/\%name\%/{HistName} \n
   *           Postprocessor id with 0D-Histogram that we create project.
   *           Default is 0.
   * @cassttng PostProcessor/\%name\%/{Size} \n
   *           Number of values that are stored
   *           Default is 10000
   *
   * @author Stephan Kassemeyer
   */
  class pp70 : public PostprocessorBackend
  {
  public:
    /** constructor */
    pp70(PostProcessors& hist, const PostProcessors::key_t&);

    /** process event */
    virtual void process(const CASSEvent&);

    /** load the settings of the pp */
    virtual void loadSettings(size_t);

  protected:
    /** the histogram to work on */
    PostProcessors::key_t _idHist;

    /** the number of bins in the resulting histogram, range is fixed */
    size_t _size;
  };



}

#endif



// Local Variables:
// coding: utf-8
// mode: C++
// c-file-style: "gnu"
// c-file-offsets: ((c . 0) (innamespace . 0))
// fill-column: 100
// End:
