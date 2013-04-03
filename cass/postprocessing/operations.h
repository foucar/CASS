// Copyright (C) 2010 -2013 Lutz Foucar

/**
 * @file operations.h file contains postprocessors that will operate
 *                     on histograms of other postprocessors
 *
 * @todo add pp creating a running/moving standart deviation (just lke average)
 *
 * @author Lutz Foucar
 */

#ifndef _OPERATIONS_H_
#define _OPERATIONS_H_

#include <tr1/functional>

#include "backend.h"
#include "cass_event.h"
#include "histogram.h"
#include <time.h>

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
      _write_summary=false;
    }

    /** don't do anything to the histogram */
    virtual void process(const CASSEvent&){}
  };







  /** Constant Value postprocessor.
   *
   * It will not show up in the pp list and not be written to file. And contains
   * a user choosable value
   *
   * @cassttng PostProcessor/\%name\%/{Value} \n
   *           The value of the postprocessors 0d histogram Default is 0.
   *
   * @author Lutz Foucar
   */
  class pp12 : public PostprocessorBackend
  {
  public:
    /** constructor */
    pp12(PostProcessors& pp, const PostProcessors::key_t &key);

    /** don't do anything to the histogram */
    virtual void process(const CASSEvent&){}

    /** load the settings of this pp */
    virtual void loadSettings(size_t);
  };







  /** Check whether value has changed.
   *
   * check whether a value has changed with respekt to the previous event.
   *
   * @cassttng PostProcessor/\%name\%/{HistName} \n
   *           the postprocessor name that contain the first histogram. Default
   *           is 0.
   *
   * @author Lutz Foucar
   */
  class pp15 : public PostprocessorBackend
  {
  public:
    /** constructor */
    pp15(PostProcessors& hist, const PostProcessors::key_t&);

    /** process event */
    virtual void process(const CASSEvent&);

    /** load the settings of this pp */
    virtual void loadSettings(size_t);

  protected:
    /** pp containing the value to check */
    PostprocessorBackend *_hist;

    /** the value of the previous event */
    float _previousVal;
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

    /** change own histograms when one of the ones we depend on has changed histograms */
    virtual void histogramsChanged(const HistogramBackend*);

  protected:
    /** pp containing input histogram */
    PostprocessorBackend *_one;

    /** the threshold */
    float _threshold;
  };












  /** Threshold histogram.
   *
   * does the same as PostProcessor 40, but does it with another image. Checks
   * wether the pixel value of the threshold image is bigger than the pxiel
   * value of the image, if so the result will be the the image pixel value and
   * 0 otherwise.
   *
   * @see PostprocessorBackend for a list of all commonly available cass.ini
   *      settings.
   *
   * @cassttng PostProcessor/\%name\%/{HistName} \n
   *           Postprocessor name with histogram that should be thresholded. Default is 0.
   * @cassttng PostProcessor/\%name\%/{ThresholdHist} \n
   *           Image with which the hist will be thresholded.  Default is 0.
   *
   * @author Thomas White
   * @author Lutz Foucar
   */
  class pp41 : public PostprocessorBackend
  {
  public:
    /** constructor */
    pp41(PostProcessors& hist, const PostProcessors::key_t&);

    /** process event */
    virtual void process(const CASSEvent&);

    /** load the settings of this pp */
    virtual void loadSettings(size_t);

    /** change own histograms when one of the ones we depend on has changed histograms */
    virtual void histogramsChanged(const HistogramBackend*);

  protected:
    /** pp containing input histogram */
    PostprocessorBackend *_one;

    /** pp containing threshold histogram */
    PostprocessorBackend *_threshold;
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

    /** change the histogram, when told the the dependand histogram has changed */
    virtual void histogramsChanged(const HistogramBackend*);

  protected:
    /** setup the parameters for finding the full width at half maximum */
    void setupParameters(const HistogramBackend &hist);

    /** pp containing the 2d hist we want to project */
    PostprocessorBackend *_pHist;

    /** range we want to project */
    std::pair<float,float> _userRange;

    /** range we want to project */
    std::pair<float,float> _range;

    /** flag whether we should normalize the values */
    bool _normalize;

    /** axis we want to project on */
    HistogramBackend::Axis _axis;

    /** axis we want to put the range on */
    HistogramBackend::Axis _otherAxis;
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







  /** store previous histogram of other PostProcessor
   *
   * Stores a previous version of another histogram.
   *
   * @see PostprocessorBackend for a list of all commonly available cass.ini
   *      settings.
   *
   * @cassttng PostProcessor/\%name\%/{HistName} \n
   *           Postprocessor name containing the histogram that we average.
   *
   * @author Lutz Foucar
   */
  class pp56 : public PostprocessorBackend
  {
  public:
    /** constructor */
    pp56(PostProcessors& hist, const PostProcessors::key_t&);

    /** process event */
    virtual void process(const CASSEvent&);

    /** load the settings */
    virtual void loadSettings(size_t);

    /** change the histogram, when told the the dependand histogram has changed */
    virtual void histogramsChanged(const HistogramBackend*);

  protected:
    /** alpha for the running average */
    HistogramFloatBase::storage_t _storage;

    /** pp containing histogram to work on */
    PostprocessorBackend *_pHist;
  };















  /** 0D,1D or 2D to 1D histogramming.
   *
   * histograms all values of 0D, 1D or 2D into a 1D Histogram. This histogram
   * holds only the histogrammed values of one event. Use PostProcessors 61 or
   * 62 to average or sum up this histogram, respectively.
   *
   * @see PostprocessorBackend for a list of all commonly available cass.ini
   *      settings.
   *
   * @cassttng PostProcessor/\%name\%/{XNbrBins|XLow|XUp}\n
   *           properties of the resulting 1D histogram
   * @cassttng PostProcessor/\%name\%/{HistName} \n
   *           Postprocessor name containing the values to histogram
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

    /** change the histogram, when told the the dependand histogram has changed */
    virtual void histogramsChanged(const HistogramBackend*);

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
   *           Postprocessor name containing the histogram that we sum up.
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

    /** change the histogram, when told the the dependand histogram has changed */
    virtual void histogramsChanged(const HistogramBackend*);

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
    PostprocessorBackend *_hist;

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








  /** 1D to 2D histogramming.
   *
   * histograms two 1d histograms into one 2D Histogram
   *
   * @see PostprocessorBackend for a list of all commonly available cass.ini
   *      settings.
   *
   * @cassttng PostProcessor/\%name\%/{HistOne|HistTwo} \n
   *           Postprocessor names containing the 1D histograms to histogram.
   *
   * @author Lutz Foucar
   */
  class pp66 : public PostprocessorBackend
  {
  public:
    /** constructor */
    pp66(PostProcessors& hist, const PostProcessors::key_t&);

    /** process event */
    virtual void process(const CASSEvent&);

    /** load the settings */
    virtual void loadSettings(size_t);

    /** change the histogram, when told the the dependand histogram has changed */
    virtual void histogramsChanged(const HistogramBackend*);

  protected:
    /** pp containing first 0D histogram to work on */
    PostprocessorBackend *_one;

    /** pp containing second 0D histogram to work on */
    PostprocessorBackend *_two;
  };







  /** 0D to 1D histogramming.
   *
   * histograms two 0d values into one 1D Histogram where the first Histogram
   * defines the x axis bin and the second is the weight.
   *
   * @see PostprocessorBackend for a list of all commonly available cass.ini
   *      settings.
   *
   * @cassttng PostProcessor/\%name\%/{XNbrBins|XLow|XUp}\n
   *           properties of the resulting 1d histogram
   * @cassttng PostProcessor/\%name\%/{HistOne|HistTwo} \n
   *           Postprocessor names containing the 0D values to histogram.
   *
   * @author Lutz Foucar
   */
  class pp67 : public PostprocessorBackend
  {
  public:
    /** constructor */
    pp67(PostProcessors& hist, const PostProcessors::key_t&);

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






  /** 0D and 1D to 2D histogramming.
   *
   * histograms a 0d and 1D Histogram to a 2d Histogram where the first 1d
   * Histogram defines the x axis and the second 0d histogram gives the y axis.
   * One only has to define the y axis since the x axis will be taken from the
   * 1d histogram
   *
   * @see PostprocessorBackend for a list of all commonly available cass.ini
   *      settings.
   *
   * @cassttng PostProcessor/\%name\%/{YNbrBins|YLow|YUp}\n
   *           properties of the y axis of the resulting 2d histogram
   * @cassttng PostProcessor/\%name\%/{HistOne}\n
   *           postprocessr containing the 1d histogram.
   * @cassttng PostProcessor/\%name\%/{HistTwo} \n
   *           Postprocessor containing the 0D values for the y axis
   *
   * @author Lutz Foucar
   */
  class pp68 : public PostprocessorBackend
  {
  public:
    /** constructor */
    pp68(PostProcessors& hist, const PostProcessors::key_t&);

    /** process event */
    virtual void process(const CASSEvent&);

    /** change the histogram, when told the the dependand histogram has changed */
    virtual void histogramsChanged(const HistogramBackend*);

    /** set up the histogram */
    void setup(const Histogram1DFloat &one);

    /** load the settings */
    virtual void loadSettings(size_t);

  protected:
    /** pp containing first 0D histogram to work on */
    PostprocessorBackend *_one;

    /** pp containing second 0D histogram to work on */
    PostprocessorBackend *_two;
  };






  /** 0D to 1D scatter plot.
   *
   * sets two 0d values into one 1D Histogram where the first Histogram
   * defines the x axis bin and the second is the weight.
   *
   * @see PostprocessorBackend for a list of all commonly available cass.ini
   *      settings.
   *
   * @cassttng PostProcessor/\%name\%/{XNbrBins|XLow|XUp}\n
   *           properties of the resulting 1d histogram
   * @cassttng PostProcessor/\%name\%/{HistOne|HistTwo} \n
   *           Postprocessor names containing the 0D values to histogram.
   *
   * @author Lutz Foucar
   */
  class pp69 : public PostprocessorBackend
  {
  public:
    /** constructor */
    pp69(PostProcessors& hist, const PostProcessors::key_t&);

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
   * @cassttng PostProcessor/\%name\%/{XLow|Xup} \n
   *           For 1d and 2d histogram the lower and upper range on the x-axis that one wants
   *           to include in the subset histogram. Default is 0|1
   * @cassttng PostProcessor/\%name\%/{YLow|Yup} \n
   *           In case you want to subset a 2d histogram these are the lower and upper range
   *           on the y-axis that one wants to include in the subset histogram. Default is 0|1
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

    /** change the histogram, when told the the dependand histogram has changed */
    virtual void histogramsChanged(const HistogramBackend*);

    /** load the settings of the pp */
    virtual void loadSettings(size_t);

  protected:
    /** pp containing input histogram */
    PostprocessorBackend *_pHist;

    /** offset of first bin in input in Histogram coordinates */
    size_t _inputOffset;

    /** the user requested x-axis limits */
    std::pair<float,float> _userXRange;

    /** the user requested x-axis limits */
    std::pair<float,float> _userYRange;
  };






  /** return the maximum value of a histogram
   *
   * @see PostprocessorBackend for a list of all commonly available cass.ini
   *      settings.
   *
   * @cassttng PostProcessor/\%name\%/{RetrieveType} \n
   *           Type of function used to retrieve the requested value in the
   *           Histogram. Default is "max". Possible values are:
   *           - "max": return the maximum value in the histogram
   *           - "min": return the minimum value in the histogram
   * @cassttng PostProcessor/\%name\%/{HistName} \n
   *           histogram name to find the maximum value in.
   *
   * @author Lutz Foucar
   */
  class pp71 : public PostprocessorBackend
  {
  public:
    /** constructor */
    pp71(PostProcessors& hist, const PostProcessors::key_t&);

    /** process event */
    virtual void process(const CASSEvent&);

    /** load the settings of the pp */
    virtual void loadSettings(size_t);

  protected:
    /** pp containing input histogram */
    PostprocessorBackend *_pHist;

    /** the type of function used to retrive the wanted element */
    std::tr1::function<HistogramFloatBase::storage_t::const_iterator(HistogramFloatBase::storage_t::const_iterator,HistogramFloatBase::storage_t::const_iterator)> _func;
  };











  /** clear Histogram
   *
   * Will clear a specific histogram when the condition is true.
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
  class pp75 : public PostprocessorBackend
  {
  public:
    /** constructor */
    pp75(PostProcessors& hist, const PostProcessors::key_t&);

    /** process event */
    virtual void process(const CASSEvent&);

    /** load the settings of the pp */
    virtual void loadSettings(size_t);

  protected:
    /** pp containing input histogram */
    PostprocessorBackend *_hist;
  };










  /** Quit Program
   *
   * Will quit the program, when called. Make sure that it is only called when
   * you want it to be called by setting the "ConditionName" to something
   * meaningful. Defaultly "ConditionName" is set to "DefaultTrueHist" which
   * will let the program quit immediately
   *
   * @see PostprocessorBackend for a list of all commonly available cass.ini
   *      settings.
   *
   * @author Lutz Foucar
   */
  class pp76 : public PostprocessorBackend
  {
  public:
    /** constructor */
    pp76(PostProcessors& hist, const PostProcessors::key_t&);

    /** process event */
    virtual void process(const CASSEvent&);

    /** load the settings of the pp */
    virtual void loadSettings(size_t);
  };







  /** Checks for id on list
   *
   * Checks if the id of the current event is on a user provided list. The
   * user provided list of id should be an ascii file where the ids are in lines.
   *
   * @see PostprocessorBackend for a list of all commonly available cass.ini
   *      settings.
   *
   * @cassttng PostProcessor/\%name\%/{List} \n
   *           Path and name of the file containing the list of id that should
   *           be checked. Default is "".
   *
   * @author Lutz Foucar
   */
  class pp77 : public PostprocessorBackend
  {
  public:
    /** constructor */
    pp77(PostProcessors& hist, const PostProcessors::key_t&);

    /** process event */
    virtual void process(const CASSEvent&);

    /** load the settings of the pp */
    virtual void loadSettings(size_t);

  protected:
    /** the list of ids */
    std::vector<uint64_t> _list;
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

    /** process event */
    virtual void process(const CASSEvent&);

    /** load the settings of the pp */
    virtual void loadSettings(size_t);

  protected:
    /** pp containing input histogram */
    PostprocessorBackend *_pHist;
  };













  /** return the bin with the highest value of an 1D histogram
   *
   * @see PostprocessorBackend for a list of all commonly available cass.ini
   *      settings.
   *
   * @cassttng PostProcessor/\%name\%/{HistName} \n
   *           histogram name for which we count fills. Default is 0.
   *
   * @author Stephan Kassemeyer
   */
  class pp81 : public PostprocessorBackend
  {
  public:
    /** constructor */
    pp81(PostProcessors& hist, const PostProcessors::key_t&);

    /** process event */
    virtual void process(const CASSEvent&);

    /** load the settings of the pp */
    virtual void loadSettings(size_t);

  protected:
    /** pp containing input histogram */
    PostprocessorBackend *_pHist;
  };
















  /** return the mean value of all bins of incomming histogram
   *
   * @see PostprocessorBackend for a list of all commonly available cass.ini
   *      settings.
   *
   * @cassttng PostProcessor/\%name\%/{HistName} \n
   *           histogram name for which the mean is calculated.
   *           Default is blubb
   *
   * @author Lutz Foucar
   */
  class pp82 : public PostprocessorBackend
  {
  public:
    /** constructor */
    pp82(PostProcessors& hist, const PostProcessors::key_t&);

    /** process event */
    virtual void process(const CASSEvent&);

    /** load the settings of the pp */
    virtual void loadSettings(size_t);

  protected:
    /** pp containing input histogram */
    PostprocessorBackend *_pHist;
  };
















  /** return the standart deviation of all bins of incomming histogram
   *
   * @see PostprocessorBackend for a list of all commonly available cass.ini
   *      settings.
   *
   * @cassttng PostProcessor/\%name\%/{HistName} \n
   *           histogram name for which the standart deviation is calculated.
   *           Default is blubb.
   *
   * @author Lutz Foucar
   */
  class pp83 : public PostprocessorBackend
  {
  public:
    /** constructor */
    pp83(PostProcessors& hist, const PostProcessors::key_t&);

    /** process event */
    virtual void process(const CASSEvent&);

    /** load the settings of the pp */
    virtual void loadSettings(size_t);

  protected:
    /** pp containing input histogram */
    PostprocessorBackend *_pHist;
  };













  /** return the sum of all bin elements of incomming histogram
   *
   * @see PostprocessorBackend for a list of all commonly available cass.ini
   *      settings.
   *
   * @cassttng PostProcessor/\%name\%/{HistName} \n
   *           histogram name for the sum of all bins is calculated.
   *           Default is blubb
   *
   * @author Lutz Foucar
   */
  class pp84 : public PostprocessorBackend
  {
  public:
    /** constructor */
    pp84(PostProcessors& hist, const PostProcessors::key_t&);

    /** process event */
    virtual void process(const CASSEvent&);

    /** load the settings of the pp */
    virtual void loadSettings(size_t);

  protected:
    /** pp containing input histogram */
    PostprocessorBackend *_pHist;
  };












  /** return full width at half maximum in given range of 1D histgoram
   *
   * @see PostprocessorBackend for a list of all commonly available cass.ini
   *      settings.
   *
   * @cassttng PostProcessor/\%name\%/{HistName} \n
   *           Name of PostProcessor that contains the histogram that we want to
   *           analyze and find the FWHM. Default is 0.
   * @cassttng PostProcessor/\%name\%/{XLow|XUp} \n
   *           Lower and upper limit of the range that we look for the width at half maximum.
   *           Default is 0|1.
   *
   * @author Lutz Foucar
   */
  class pp85 : public PostprocessorBackend
  {
  public:
    /** constructor */
    pp85(PostProcessors& hist, const PostProcessors::key_t&);

    /** process event */
    virtual void process(const CASSEvent&);

    /** load the settings of the pp */
    virtual void loadSettings(size_t);

    /** change the histogram, when told the the dependand histogram has changed */
    virtual void histogramsChanged(const HistogramBackend*);

  protected:
    /** setup the parameters for finding the full width at half maximum */
    void setupParameters(const HistogramBackend &hist);

    /** pp containing input histogram */
    PostprocessorBackend *_pHist;

    /** the user requested x-axis limits */
    std::pair<float,float> _userXRange;

    /** the requested x-axis limits in histogram coordinates */
    std::pair<size_t,size_t> _xRange;
  };









  /** find step in 1d hist
   *
   * finds the x-position of a step in a 1d hist. It does this by defining a
   * baseline from the user selected range. It then searches for the highest
   * point in the range that should contain the step. Now it looks for the first
   * x position where the y value is Fraction * (highestPoint + baselline).
   *
   * @see PostprocessorBackend for a list of all commonly available cass.ini
   *      settings.
   *
   * @cassttng PostProcessor/\%name\%/{HistName} \n
   *           Histogram name of the 1d Histogram that we look for the step in.
   *          Default is 0.
   * @cassttng PostProcessor/\%name\%/{XLow|XUp} \n
   *           Lower and upper limit of the range that we look for the step.
   *           Default is 0|1.
   * @cassttng PostProcessor/\%name\%/{BaselineLow|BaselineUp} \n
   *           Lower and upper limit of the range that we use to calculate the
   *           Baseline.
   *           Default is 0|1.
   * @cassttng PostProcessor/\%name\%/{Fraction} \n
   *           The Fraction between the baseline and the highest value that
   *           should be taken when searching for the right point. Default is
   *           0.5
   *
   * @author Lutz Foucar
   */
  class pp86 : public PostprocessorBackend
  {
  public:
    /** constructor */
    pp86(PostProcessors& hist, const PostProcessors::key_t&);

    /** process event */
    virtual void process(const CASSEvent&);

    /** load the settings of the pp */
    virtual void loadSettings(size_t);

    /** change the parameters, when told the the dependand histogram has changed */
    virtual void histogramsChanged(const HistogramBackend*);

  protected:
    /** setup the parameters for finding the step */
    void setupParameters(const HistogramBackend &hist);

    /** pp containing input histogram */
    PostprocessorBackend *_pHist;

    /** the user requested x-axis limits for the step finding*/
    std::pair<float,float> _userXRangeStep;

    /** the requested x-axis limits for find the step in histogram coordinates */
    std::pair<size_t,size_t> _xRangeStep;

    /** the user requested x-axis limits for the baseline finding*/
    std::pair<float,float> _userXRangeBaseline;

    /** the requested x-axis limits for find the baseline in histogram coordinates */
    std::pair<size_t,size_t> _xRangeBaseline;

    /** user fraction of the height between low and up */
    float _userFraction;
  };






  /** find center of Mass of 1d hist in a user given range
   *
   * calculates the center of Mass in the user given range.
   *
   * @see PostprocessorBackend for a list of all commonly available cass.ini
   *      settings.
   *
   * @cassttng PostProcessor/\%name\%/{HistName} \n
   *           Histogram name of the 1d Histogram that we look for the step in.
   *           Default is 0.
   * @cassttng PostProcessor/\%name\%/{XLow|XUp} \n
   *           Lower and upper limit of the range that we look for the step.
   *           Default is 0|1.
   *
   * @author Lutz Foucar
   */
  class pp87 : public PostprocessorBackend
  {
  public:
    /** constructor */
    pp87(PostProcessors& hist, const PostProcessors::key_t&);

    /** process event */
    virtual void process(const CASSEvent&);

    /** load the settings of the pp */
    virtual void loadSettings(size_t);

    /** change the parameters, when told the the dependand histogram has changed */
    virtual void histogramsChanged(const HistogramBackend*);

  protected:
    /** setup the parameters for finding the step */
    void setupParameters(const HistogramBackend &hist);

    /** pp containing input histogram */
    PostprocessorBackend *_pHist;

    /** the user requested x-axis limits */
    std::pair<float,float> _userXRange;

    /** the requested x-axis limits in histogram coordinates */
    std::pair<size_t,size_t> _xRange;
  };





  /** return axis parameter of an histogram
   *
   * @see PostprocessorBackend for a list of all commonly available cass.ini
   *      settings.
   *
   * @cassttng PostProcessor/\%name\%/{HistName} \n
   *           histogram name for which we count fills. Default is 0.
   * @cassttng PostProcessor/\%name\%/{AxisParameter} \n
   *           The parameter of the axis one is interested in.
   *           Default is "XNbrBins". Possible values are:
   *           - "XNbrBins": The number of Bins in X
   *           - "XLow": The lower bound of the x-axis
   *           - "XUp": The upper bound of the x-axis
   *           - "YNbrBins": The number of Bins in Y
   *           - "YLow": The lower bound of the y-axis
   *           - "YUp": The upper bound of the y-axis
   *
   * @author Lutz Foucar
   */
  class pp88 : public PostprocessorBackend
  {
  public:
    /** constructor */
    pp88(PostProcessors& hist, const PostProcessors::key_t&);

    /** process event */
    virtual void process(const CASSEvent&);

    /** load the settings of the pp */
    virtual void loadSettings(size_t);

  protected:
    /** pp containing input histogram */
    PostprocessorBackend *_pHist;

    /** the id of the axis */
    int _axisId;

    /** function to retrieve the parameter from the axis */
    std::tr1::function<float(const AxisProperty&)> _func;
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
