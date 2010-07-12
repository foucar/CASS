// Copyright (C) 2010 Lutz Foucar

/** @file operations.h file contains postprocessors that will operate
 *                     on histograms of other postprocessors
 * @todo add pp that has input and condition output is copy of input
 * @author Lutz Foucar
 */

#ifndef _OPERATIONS_H_
#define _OPERATIONS_H_

#include "backend.h"
#include "cass_event.h"
#include "histogram.h"
#include <time.h>
#include <deque>

namespace cass
{


  /** Apply boolean NOT to 0D Histogram.
   *
   * @see PostprocessorBackend for a list of all commonly available cass.ini
   *      settings.
   *
   * @cassttng PostProcessor/\%name\%/{HistName} \n
   *           the postprocessor name that contain the first histogram. Default
   *           is "".
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








  /** Check whether histogram is in range.
   *
   * @see PostprocessorBackend for a list of all commonly available cass.ini
   *      settings.
   *
   * @cassttng PostProcessor/\%name\%/{HistName} \n
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
    /** pp containing first histogram */
    PostprocessorBackend *_one;

    /** the requested range that the histogram should be in */
    std::pair<float,float> _range;
  };








  /** Constant BOOLEAN postprocessor.
   *
   * implements pp id 10 and 11. It will not show up in the pp list and not be
   * written to file.
   *
   * @author Lutz Foucar
   */
  class pp10 : public PostprocessorBackend
  {
  public:
    /** constructor */
    pp10(PostProcessors& pp, const PostProcessors::key_t &key, bool boolean)
      :PostprocessorBackend(pp,key)
    {
      _result = (new Histogram0DFloat(boolean));
      createHistList(1);
      _hide =true;
      _write =false;
    }
    /** don't do anything to the histogram */
    virtual void process(const CASSEvent&){}
  };







  /** Threshold histogram.
   *
   * @see PostprocessorBackend for a list of all commonly available cass.ini
   *      settings.
   *
   * @cassttng PostProcessor/\%name\%/{HistName} \n
   *           Postprocessor name with histogram that should be thresholded. Default is 0.
   * @cassttng PostProcessor/\%name\%/{Threshold} \n
   *           Factor with which threshold value. Default is 0.
   *
   * @author Thomas White
   */
  class pp40 : public PostprocessorBackend
  {
  public:
    /** constructor */
    pp40(PostProcessors& hist, const PostProcessors::key_t&);

    /** process event */
    virtual void process(const CASSEvent&);

    /** load the settings of this pp */
    virtual void loadSettings(size_t);

  protected:
    /** pp containing input histogram */
    PostprocessorBackend *_one;

    /** the threshold */
    float _threshold;
  };







  /** Projection of 2d Histogram.
   *
   * @see PostprocessorBackend for a list of all commonly available cass.ini
   *      settings.
   *
   * @cassttng PostProcessor/\%name\%/{HistName} \n
   *           Postprocessor name with 2D-Histogram that we create project.
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
   * @see PostprocessorBackend for a list of all commonly available cass.ini
   *      settings.
   *
   * @cassttng PostProcessor/\%name\%/{HistName} \n
   *           Postprocessor name with 1D-Histogram that we create the intgral from Default is 0.
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
    /** pp containing the 1d hist we want to integrate */
    PostprocessorBackend *_pHist;

    /** range we want to have the integral over in histogram bins */
    std::pair<float,float> _area;
  };











  /** Radial Average/Projection of 2d Histogram.
   *
   * @see PostprocessorBackend for a list of all commonly available cass.ini
   *      settings.
   *
   * @cassttng PostProcessor/\%name\%/{HistName} \n
   *           Postprocessor name with 2D-Histogram that we create project.
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
    /** pp containing the 2d hist we want to project */
    PostprocessorBackend *_pHist;

    /** center coordinates we use to calculate the radial average in histogram coordinates */
    std::pair<size_t,size_t> _center;

    /** the maximum size of the radius, calculated from the two d histogram */
    size_t _radius;
  };











  /** Radar Plot of 2d Histogram.
   *
   * @see PostprocessorBackend for a list of all commonly available cass.ini
   *      settings.
   *
   * @cassttng PostProcessor/\%name\%/{HistName} \n
   *           Postprocessor name with 2D-Histogram that we create project.
   *           Default is 0.
   * @cassttng PostProcessor/\%name\%/{MinRadius|MaxRadius} \n
   *           Minimum and Maximum Radius to include in the polar plot. Default
   *           is 0 ... 512
   * @cassttng PostProcessor/\%name\%/{NbrBins} \n
   *           Number of Bins where the 360 degrees will be put in. Default is 360
   * @cassttng PostProcessor/\%name\%/{XCenter|YCenter} \n
   *           X and Y Center of the images polar plot. Default is 512,512
   *
   * @todo improve and generalize radial projection to account for different distance of detector to beam line
   * @todo add treatment of possible asymmetric positions of detectors to beam line
   * @todo add possibility to have circle partially outside the physical detector dimensions
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
    /** pp containing the 2d hist we want to project */
    PostprocessorBackend *_pHist;

    /** range of radii that we use for the angular distribution */
    std::pair<size_t,size_t> _range;

    /** centre's coordinates we use to calculate the radar plot */
    std::pair<size_t,size_t> _center;

    /** the number of bins in the resulting histogram, range is fixed */
    size_t _nbrBins;
  };















  /** Radius \f$ \phi \f$ Representation of 2D Histogram.
   *
   * @see PostprocessorBackend for a list of all commonly available cass.ini
   *      settings.
   *
   * @cassttng PostProcessor/\%name\%/{HistName} \n
   *           Postprocessor name with 2D-Histogram we convert.
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
    /** pp containing the 2d hist we want to project */
    PostprocessorBackend *_pHist;

    /** centre's coordinates we use to calculate the radar plot */
    std::pair<size_t,size_t> _center;

    /** the number of bins in the resulting histogram, range is fixed */
    size_t _nbrBins;

    /** the maximum size of the radius, calculated from the two d histogram */
    size_t _radius;
  };








  /** 0D to 1D histogramming.
   *
   * histogram 0d values into a 1D Histogram
   *
   * @see PostprocessorBackend for a list of all commonly available cass.ini
   *      settings.
   *
   * @cassttng PostProcessor/\%name\%/{XNbrBins|XLow|XUp}\n
   *           properties of the 1d histogram
   * @cassttng PostProcessor/\%name\%/{HistName} \n
   *           Postprocessor name containing the 0D value to histogram
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
    /** pp containing 0D histogram to work on */
    PostprocessorBackend *_pHist;
  };






  /** Histogram averaging.
   *
   * Running or cummulative average of a histogram.
   *
   * @see PostprocessorBackend for a list of all commonly available cass.ini
   *      settings.
   *
   * @cassttng PostProcessor/\%name\%/{NbrOfAverages}\n
   *           how many images should be averaged. When value is 0 its a cummulative
   *           average. Default is 1.
   * @cassttng PostProcessor/\%name\%/{HistName} \n
   *           Postprocessor name containing the histogram that we average.
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

    /** pp containing histogram to work on */
    PostprocessorBackend *_pHist;
  };





  /** Histogram summing.
   *
   * Sums up histograms.
   *
   * @see PostprocessorBackend for a list of all commonly available cass.ini
   *      settings.
   *
   * @cassttng PostProcessor/\%name\%/{HistName} \n
   *           Postprocessor name containing the histogram that we average.
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
    /** pp containing histogram to work on */
    PostprocessorBackend *_pHist;
  };






  /** time average of 0d Histogram.
   *
   * Makes an running average of a given Histogram over a given time period.
   *
   * @see PostprocessorBackend for a list of all commonly available cass.ini
   *      settings.
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
    /** pp containing histogram to work on */
    PostprocessorBackend *_pHist;

    /** range of time that we use for the angular distribution */
    std::pair<size_t,size_t> _timerange;

    /** the number of bins in the resulting histogram, range is fixed */
    size_t _nbrSamples;

    /** the number of samples seen up to now and used in the point */
    size_t _num_seen_evt;

    //@{
    /** time when the first samples was used in the point in time */
    time_t _when_first_evt;
    uint32_t _first_fiducials;
    //@}
  };





  /** record 0d Histogram into 1d Histogram.
   *
   * appends values from 0d histogram at end of 1d histogram and shifts the old values to the left.
   *
   * @see PostprocessorBackend for a list of all commonly available cass.ini
   *      settings.
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
  class pp64 : public PostprocessorBackend
  {
  public:
    /** constructor */
    pp64(PostProcessors& hist, const PostProcessors::key_t&);

    /** process event */
    virtual void process(const CASSEvent&);

    /** load the settings of the pp */
    virtual void loadSettings(size_t);

  protected:
    /** pp containing input histogram */
    PostprocessorBackend *_one;

    /** pp containing input histogram */
    PostprocessorBackend *_two;

    /** the number of bins in the resulting histogram, range is fixed */
    size_t _size;
  };






  /** 0D to 2D histogramming.
   *
   * histograms two 0d values into one 2D Histogram
   *
   * @see PostprocessorBackend for a list of all commonly available cass.ini
   *      settings.
   *
   * @cassttng PostProcessor/\%name\%/{XNbrBins|XLow|XUp|YNbrBins|YLow|YUp}\n
   *           properties of the 2d histogram
   * @cassttng PostProcessor/\%name\%/{HistOne|HistTwo} \n
   *           Postprocessor names containing the 0D values to histogram.
   *
   * @author Lutz Foucar
   */
  class pp65 : public PostprocessorBackend
  {
  public:
    /** constructor */
    pp65(PostProcessors& hist, const PostProcessors::key_t&);

    /** process event */
    virtual void process(const CASSEvent&);

    /** load the settings */
    virtual void loadSettings(size_t);

  protected:
    /** pp containing first 0D histogram to work on */
    PostprocessorBackend *_one;

    /** pp containing second 0D histogram to work on */
    PostprocessorBackend *_two;
  };







  /** Subset Histogram
   *
   * Will copy a subset of another histogram and return it in a new histogram.
   *
   * @see PostprocessorBackend for a list of all commonly available cass.ini
   *      settings.
   *
   * @cassttng PostProcessor/\%name\%/{HistName} \n
   *           name of postprocessor that contains the histogram you want a
   *           subset from. Default is "".
   *
   * @author Lutz Foucar
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
    /** pp containing input histogram */
    PostprocessorBackend *_pHist;

    /** nbr of bins in y (1 for 1D Hist) */
    size_t _nyBins;

    /** xlow of input in Histogram coordinates */
    size_t _inputXLow;
  };









  /** return number of fills of a given histogram
   *
   * @see PostprocessorBackend for a list of all commonly available cass.ini
   *      settings.
   *
   * @cassttng PostProcessor/\%name\%/{HistName} \n
   *           histogram name for which we count fills. Default is 0.
   *
   * @author Stephan Kassemeyer
   */
  class pp80 : public PostprocessorBackend
  {
  public:
    /** constructor */
    pp80(PostProcessors& hist, const PostProcessors::key_t&);

    /** copy image from CASS event to histogram storage */
    virtual void process(const CASSEvent&);

    /** load the settings of the pp */
    virtual void loadSettings(size_t);

  protected:
    /** pp containing input histogram */
    PostprocessorBackend *_pHist;
  };







  /** return radial average of a given 2D histogram
   *
   * @see PostprocessorBackend for a list of all commonly available cass.ini
   *      settings.
   *
   * @cassttng PostProcessor/\%name\%/{HistName} \n
   *           histogram name for which we count fills. Default is 0.
   * @cassttng PostProcessor/\%name\%/{ConditionName} \n
   *           condition
   * @cassttng PostProcessor/\%name\%/{maximalRadius} \n
   * @cassttng PostProcessor/\%name\%/{centerX} \n
   * @cassttng PostProcessor/\%name\%/{centerY} \n
   * @author Stephan Kassemeyer
   */
  class pp401 : public PostprocessorBackend
  {
  public:
    /** constructor */
    pp401(PostProcessors& hist, const PostProcessors::key_t&);

    /** copy image from CASS event to histogram storage */
    virtual void process(const CASSEvent&);

    /** load the settings of the pp */
    virtual void loadSettings(size_t);

  protected:
    /** pp containing input histogram */
    PostprocessorBackend *_pHist;

    /** setting: maximal radius */
    int _maxRad;

    /** setting: center x*/
    int _cx;

    /** setting: center y*/
    int _cy;

    /** tmp hist: number of values filled in for each rad, for averaging*/
    Histogram1DFloat* _count;


  };





  /** calculate median of last values. If input histogram is > 0d, its values get
   *  summed up prior to median calculation.
   *
   * @see PostprocessorBackend for a list of all commonly available cass.ini
   *      settings.
   *
   * @cassttng PostProcessor/\%name\%/{HistName} \n
   *           the postprocessor name that contain the first histogram. Default
   *           is 0.
   * @cassttng PostProcessor/\%name\%/{medianSize} \n
   *           how many last values should be included in median calculation.
   *           default is 100.
   *
   * @todo make more general: operate on bins. now operates on sum.
   * @author Stephan Kassemeyer.
   */
  class pp66 : public PostprocessorBackend
  {
  public:
    /** constructor */
    pp66(PostProcessors& hist, const PostProcessors::key_t&);

    /** process event */
    virtual void process(const CASSEvent&);

    /** load the settings of this pp */
    virtual void loadSettings(size_t);

  protected:
    /** pp containing first histogram */
    PostprocessorBackend *_one;

    /** last N items to be used for median calculation */
    unsigned int _medianSize;

    /** storage of last values for median calculation */
    std::deque<float> *_medianStorage;
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
