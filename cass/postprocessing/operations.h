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
#include <time.h>

#include "processor.h"
#include "cass_event.h"
#include "histogram.h"
#include "statistics_calculator.hpp"

namespace cass
{



/** Operation on 2 Histograms
 *
 * @PPList "1":Operation on 2 Histograms value by value
 *
 * @see PostProcessor for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng PostProcessor/\%name\%/{HistOne} \n
 *           the postprocessor name that contain the first histogram.
 *           Needs to be of the same dimension and size as the second.
 * @cassttng PostProcessor/\%name\%/{HistTwo} \n
 *           the postprocessor name that contain the second histogram.
 *           Needs to be of the same dimension and size as the first
 * @cassttng PostProcessor/\%name\%/{Operation} \n
 *           Default is "+". Possible values are:
 *           - "+": Use add as operation
 *           - "-": Use minus as operation
 *           - "/": Use divide as operation
 *           - "*": Use multiply as operation
 *           - "AND": Use logical AND as operation
 *           - "OR": Use logical OR as operation
 *           - ">": Use greater as operation
 *           - ">=": Use greater or equal as operation
 *           - "<": Use less than as operation
 *           - "<=": Use less or equal as operation
 *           - "==": Use equal to as operation
 *           - "!=": Use not equal to as operation
 *
 * @author Lutz Foucar
 */
class pp1 : public PostProcessor
{
public:
  /** constructor */
  pp1(const name_t & name);

  /** load the settings of this pp */
  virtual void loadSettings(size_t);

  /** process event */
  virtual void process(const CASSEvent& evt, HistogramBackend &res);

protected:
  /** pp containing the first histogram */
  shared_pointer _one;

  /** pp containing the second histogram */
  shared_pointer _two;

  /** the operand */
  std::tr1::function<float(float, float)> _op;
};






/** Operation on histogram with value
 *
 * @PPList "2":Operation on histogram with value
 *
 * @see PostProcessor for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng PostProcessor/\%name\%/{HistName} \n
 *           the postprocessor name that contain the first histogram. Needs to
 *           be implemented, because default is "", which is invalid.
 * @cassttng PostProcessor/\%name\%/{Value} \n
 *           Value for the operation. Default is 1.
 * @cassttng PostProcessor/\%name\%/{ValueName} \n
 *           0D PostProcessor containing value for the operation. If this is not
 *           the default value of DonnotUse, the value from the PostProcssor is
 *           used. Otherwise the Value is used. Default is "DonnotUse"
 * @cassttng PostProcessor/\%name\%/{Operation} \n
 *           Default is "+". Possible values are:
 *           - "+": Use add as operation
 *           - "-": Use minus as operation
 *           - "/": Use divide as operation
 *           - "*": Use multiply as operation
 *           - "AND": Use logical AND as operation
 *           - "OR": Use logical OR as operation
 *           - ">": Use greater as operation
 *           - ">=": Use greater or equal as operation
 *           - "<": Use less than as operation
 *           - "<=": Use less or equal as operation
 *           - "==": Use equal to as operation
 *           - "!=": Use not equal to as operation
 * @cassttng PostProcessor/\%name\%/{ValuePos} \n
 *           Chooses where in the operation the Value or the value taken from
 *           the 0D PostProcessor will be. Default is "first". Possible values
 *           are:
 *           - first: Value will be first operand
 *           - second: Value will be second operand
 *
 * @author Lutz Foucar
 */
class pp2 : public PostProcessor
{

public:
  /** constructor */
  pp2(const name_t &name);

  /** load the settings of this pp */
  virtual void loadSettings(size_t);

  /** process event */
  virtual void process(const CASSEvent& evt, HistogramBackend &res);

protected:
  /** define the unary operation */
  typedef std::tr1::function<float(float)> unaryoperation_t;

  /** define the binary operation */
  typedef std::tr1::function<float(float,float)> binaryoperation_t;

  /** define how to get the value */
  typedef std::tr1::function<float(const CASSEvent::id_t &)> valueRetrieval_t;

  /** define how to get the right parameter position */
  typedef std::tr1::function<unaryoperation_t(float)> setParamPos_t;

  /** bind the value to the first parameter of the binaryoperation
   *
   * @return function call where value is bound to the fist parameter
   */
  unaryoperation_t ValAtFirst(float val);

  /** bind the value to the second parameter of the binaryoperation
   *
   * @return function call where value is bound to the second parameter
   */
  unaryoperation_t ValAtSecond(float val);

  /** retrieve value from PostProcessor
   *
   * @returns value stored in _valuePP
   * @param id id of the event for which the value should be returned
   */
  float valueFromPP(const CASSEvent::id_t &id);

  /** retrieve value constant
   *
   * @returns _value
   * @param evt ignored
   */
  float valueFromConst(const CASSEvent::id_t &evt);

protected:
  /** pp containing input histogram */
  shared_pointer _hist;

  /** pp containing 0D value histogram */
  shared_pointer _valuePP;

  /** the value for the unary operation */
  float _value;

  /** the operand */
  binaryoperation_t _op;

  /** function to set the value to the requested parameter position */
  setParamPos_t _setParamPos;

  /** function to retrieve the value */
  valueRetrieval_t _retrieveValue;

};







/** Apply boolean NOT to 0D Histogram
 *
 * @PPList "4": Apply boolean NOT to 0D Histogram
 *
 * @see PostProcessor for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng PostProcessor/\%name\%/{HistName} \n
 *           the postprocessor name that contain the first histogram. Default
 *           is "".
 *
 * @author Lutz Foucar
 */
class pp4 : public PostProcessor
{
public:
  /** constructor */
  pp4(const name_t &name);

  /** process event */
  virtual void process(const CASSEvent&, HistogramBackend &);

  /** load the settings of this pp */
  virtual void loadSettings(size_t);

protected:
  /** pp containing histogram */
  shared_pointer _one;
};








/** Check whether histogram is in range.
 *
 * @PPList "9": Check whether sum value of histogram is in range
 *
 * @see PostProcessor for a list of all commonly available cass.ini
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
class pp9 : public PostProcessor
{
public:
  /** constructor */
  pp9(const name_t &name);

  /** process event */
  virtual void process(const CASSEvent&, HistogramBackend &res);

  /** load the settings of this pp */
  virtual void loadSettings(size_t);

protected:
  /** pp containing first histogram */
  shared_pointer _one;

  /** the requested range that the histogram should be in */
  std::pair<float,float> _range;
};










/** Constant Value postprocessor.
 *
 * @PPList "12": Constant Value
 *
 * @cassttng PostProcessor/\%name\%/{Value} \n
 *           The value of the postprocessors 0d histogram Default is 0.
 * @cassttng PostProcessor/\%name\%/{ValueType} \n
 *           The type of constant that will we returned. Default is '0D'.
 *           Possible values are:
 *           - '0D': A 0d histogram  will be returned
 *           - '1D': A 1d array histogram will be returned
 *           - '2D': A 2d image histogram will be returned
 * @cassttng PostProcessor/\%name\%/{XNbrBins|XLow|XUp|YNbrBins|YLow|YUp}\n
 *           Optional settings, needed when selected 1D or 2D as type
 *
 * @author Lutz Foucar
 */
class pp12 : public PostProcessor
{
public:
  /** constructor */
  pp12(const name_t &);

  /** overwrite default behaviour and just return the constant */
  virtual const HistogramBackend& result(const CASSEvent::id_t)
  {
    return *_res;
  }

  /** overwrite default behaviour don't do anything */
  virtual void processEvent(const CASSEvent&){}

  /** overwrite default behaviour don't do anything */
  virtual void releaseEvent(const CASSEvent &){}

  /** load the settings of this pp */
  virtual void loadSettings(size_t);

private:
  /** the constant result */
  std::tr1::shared_ptr<HistogramFloatBase> _res;
};







/** Check whether value has changed.
 *
 * @PPList "15": Check whether value of 0d histogram has changed
 *
 * check whether a value has changed with respekt to the previous event.
 *
 * @cassttng PostProcessor/\%name\%/{HistName} \n
 *           The PostProcessor name that contain the 0D Histogram that should be
 *           monitored.
 * @cassttng PostProcessor/\%name\%/{Difference} \n
 *           The maximum allowed difference between the previous and the current
 *           value of the 0D Histogram. Default is 0 which results in a value
 *           given by std::numeric_limits<float>::epsilon().
 *
 * @author Lutz Foucar
 */
class pp15 : public PostProcessor
{
public:
  /** constructor */
  pp15(const name_t &name);

  /** process event */
  virtual void process(const CASSEvent&, HistogramBackend &);

  /** load the settings of this pp */
  virtual void loadSettings(size_t);

protected:
  /** pp containing the value to check */
  shared_pointer _hist;

  /** the value of the previous event */
  float _previousVal;

  /** the maximum difference to previous val that is accepted */
  float _difference;

  /** mutex for locking the previous value variable */
  QMutex _mutex;
};







/** Threshold histogram.
 *
 * @PPList "40": Threshold histogram
 *
 * @see PostProcessor for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng PostProcessor/\%name\%/{HistName} \n
 *           Postprocessor name with histogram that should be thresholded. Default is 0.
 * @cassttng PostProcessor/\%name\%/{Threshold} \n
 *           Factor with which threshold value. Default is 0.
 *
 * @author Thomas White
 */
class pp40 : public PostProcessor
{
public:
  /** constructor */
  pp40(const name_t &name);

  /** process event */
  virtual void process(const CASSEvent&, HistogramBackend &);

  /** load the settings of this pp */
  virtual void loadSettings(size_t);

protected:
  /** pp containing input histogram */
  shared_pointer _one;

  /** the threshold */
  float _threshold;
};












/** Threshold histogram with another histogram
 *
 * @PPList "41": Threshold histogram with another histogram
 *
 * set the bin of a histogram to a user requested value when the value of the
 * threshold is within a user requested range.
 *
 * @see PostProcessor for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng PostProcessor/\%name\%/{HistName} \n
 *           Postprocessor name with histogram that should be thresholded.
 *           Default is 0.
 * @cassttng PostProcessor/\%name\%/{ThresholdName} \n
 *           Histogram with which the histogram will be thresholded.
 *           Default is 0.
 * @cassttng PostProcessor/\%name\%/{UserVal} \n
 *           The value that will be set when the value of the ThresholdHist is
 *           within the boundaries. Default is 0
 * @cassttng PostProcessor/\%name\%/{LowerBound|UpperBound} \n
 *           The boundaries within which the value of the ThresholdHist has to
 *           be in order to set the value of the histogram to UserVal.
 *           Default is 0.5|1.5
 *
 * @author Lutz Foucar
 */
class pp41 : public PostProcessor
{
public:
  /** constructor */
  pp41(const name_t &name);

  /** process event */
  virtual void process(const CASSEvent&, HistogramBackend &);

  /** load the settings of this pp */
  virtual void loadSettings(size_t);

protected:
  /** check if value is in range and return another value
   *
   * @return user set value if value is in range
   * @param val the value that will be returned if checkval is not in range
   * @param checkval the value to check if it is in range
   */
  float checkrange(float val, float checkval);

protected:
  /** pp containing input histogram */
  shared_pointer _one;

  /** pp containing threshold histogram */
  shared_pointer _threshold;

  /** the value that will be set */
  float _userVal;

  /** the lower boundary of the range */
  float _lowerBound;

  /** the upper boundary of the range */
  float _upperBound;
};







/** Projection of 2d Histogram.
 *
 * @PPList "50": Project 2D histogram onto a axis
 *
 * @see PostProcessor for a list of all commonly available cass.ini
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
 *
 * @author Lutz Foucar
 */
class pp50 : public PostProcessor
{
public:
  /** constructor */
  pp50(const name_t &name);

  /** process event */
  virtual void process(const CASSEvent&, HistogramBackend &);

  /** load the settings of the pp */
  virtual void loadSettings(size_t);

protected:
  /** pp containing the 2d hist we want to project */
  shared_pointer _pHist;

  /** range we want to project */
  std::pair<float,float> _userRange;

  /** range we want to project */
  std::pair<float,float> _range;

  /** axis we want to project on */
  HistogramBackend::Axis _axis;

  /** axis we want to put the range on */
  HistogramBackend::Axis _otherAxis;
};










/** Integral of 1d Histogram.
 *
 * @PPList "51": Integral of 1D histogram
 *
 * @see PostProcessor for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng PostProcessor/\%name\%/{HistName} \n
 *           Postprocessor name with 1D-Histogram that we create the intgral from Default is 0.
 * @cassttng PostProcessor/\%name\%/{LowerBound|UpperBound} \n
 *           Upper and lower bound of the area to integrate. Default is -1e6 ... 1e6
 *
 * @author Lutz Foucar
 */
class pp51 : public PostProcessor
{
public:
  /** constructor */
  pp51(const name_t &name);

  /** process event */
  virtual void process(const CASSEvent&, HistogramBackend &);

  /** load the settings of the pp */
  virtual void loadSettings(size_t);

protected:
  /** pp containing the 1d hist we want to integrate */
  shared_pointer _pHist;

  /** range we want to have the integral over in histogram bins */
  std::pair<float,float> _area;
};










/** store previous histogram of other PostProcessor
 *
 * @PPList "56": Contains the Histogram of the previous event
 *
 * Stores a previous version of another histogram.
 *
 * @see PostProcessor for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng PostProcessor/\%name\%/{HistName} \n
 *           Postprocessor name containing the histogram that we average.
 *
 * @author Lutz Foucar
 */
class pp56 : public PostProcessor
{
public:
  /** constructor */
  pp56(const name_t &name);

  /** process event */
  virtual void process(const CASSEvent&, HistogramBackend &);

  /** load the settings */
  virtual void loadSettings(size_t);

protected:
  /** alpha for the running average */
  HistogramFloatBase::storage_t _storage;

  /** pp containing histogram to work on */
  shared_pointer _pHist;

  /** mutex to lock the storage */
  QMutex _mutex;
};















/** Weighted Projection of 2d Histogram.
 *
 * @PPList "57": Weighted Project 2D histogram onto a axis
 *
 * devides each bin by the number of non zero values that have been added in
 * this bin.
 *
 * @see PostProcessor for a list of all commonly available cass.ini
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
 * @cassttng PostProcessor/\%name\%/{ExclusionValue} \n
 *           The value that will be excluded when doing the projection. The
 *           result will be normilzed by the amount of bins that have been
 *           summed. Default is 0.
 *
 * @author Lutz Foucar
 */
class pp57 : public PostProcessor
{
public:
  /** constructor */
  pp57(const name_t &name);

  /** process event */
  virtual void process(const CASSEvent&, HistogramBackend &);

  /** load the settings of the pp */
  virtual void loadSettings(size_t);

private:
  /** integrate the rows
   *
   * @param src 2d image that will be projected
   * @param result vector were the projection will be written to
   * @param norm vector where the normalization values will be added to
   */
  void projectToX(const HistogramFloatBase::storage_t &src,
                  HistogramFloatBase::storage_t &result,
                  HistogramFloatBase::storage_t &norm);

  /** integrate the columns
   *
   * @param src 2d image that will be projected
   * @param result vector were the projection will be written to
   * @param norm vector where the normalization values will be added to
   */
  void projectToY(const HistogramFloatBase::storage_t &src,
                  HistogramFloatBase::storage_t &result,
                  HistogramFloatBase::storage_t &norm);

private:
  /** pp containing the 2d hist we want to project */
  shared_pointer _pHist;

  /** range in X we want to project */
  std::pair<size_t,size_t> _Xrange;

  /** range in Y we want to project */
  std::pair<size_t,size_t> _Yrange;

  /** the size of the original image in X */
  size_t _nX;

  /** the value that should be excluded in the summation */
  float _excludeVal;

  /** the function used to project the image */
  std::tr1::function<void(const HistogramFloatBase::storage_t&,
                          HistogramFloatBase::storage_t&,
                          HistogramFloatBase::storage_t&)> _project;
};













/** 0D,1D or 2D to 1D histogramming.
 *
 * @PPList "60": Histogram 0D, 1D or 2D values to a 1D histogram
 *
 * histograms all values of 0D, 1D or 2D into a 1D Histogram. This histogram
 * holds only the histogrammed values of one event. Use PostProcessors 61 or
 * 62 to average or sum up this histogram, respectively.
 *
 * @see PostProcessor for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng PostProcessor/\%name\%/{XNbrBins|XLow|XUp}\n
 *           properties of the resulting 1D histogram
 * @cassttng PostProcessor/\%name\%/{HistName} \n
 *           Postprocessor name containing the values to histogram
 *
 * @author Lutz Foucar
 */
class pp60 : public PostProcessor
{
public:
  /** constructor */
  pp60(const name_t &name);

  /** process event */
  virtual void process(const CASSEvent&, HistogramBackend &);

  /** load the settings */
  virtual void loadSettings(size_t);

protected:
  /** pp containing 0D histogram to work on */
  shared_pointer _pHist;
};






/** Histogram averaging.
 *
 * @PPList "61": Average of a histogram
 *
 * Running or cummulative average of a histogram.
 *
 * @see PostProcessor for a list of all commonly available cass.ini
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
class pp61 : public AccumulatingPostProcessor
{
public:
  /** constructor */
  pp61(const name_t &name);

  /** process event */
  virtual void process(const CASSEvent&, HistogramBackend &result);

  /** load the settings */
  virtual void loadSettings(size_t);

protected:
  /** alpha for the running average */
  float _alpha;

  /** pp containing histogram to work on */
  shared_pointer _pHist;
};





/** Histogram summing.
 *
 * @PPList "62": Summing up of histogram
 *
 * Sums up histograms.
 *
 * @see PostProcessor for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng PostProcessor/\%name\%/{HistName} \n
 *           Postprocessor name containing the histogram that we sum up.
 *
 * @author Lutz Foucar
 */
class pp62 : public AccumulatingPostProcessor
{
public:
  /** constructor */
  pp62(const name_t&);

  /** process event */
  virtual void process(const CASSEvent&, HistogramBackend &result);

  /** load the settings */
  virtual void loadSettings(size_t);

protected:
  /** pp containing histogram to work on */
  shared_pointer _pHist;
};






/** time average of 0d Histogram.
 *
 * @PPList "63": Time Average of a histogram over given time-intervals
 *
 * Makes an running average of a given Histogram over a given time period.
 *
 * @see PostProcessor for a list of all commonly available cass.ini
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
class pp63 : public AccumulatingPostProcessor
{
public:
  /** constructor */
  pp63(const name_t&);

  /** process event */
  virtual void process(const CASSEvent&, HistogramBackend &);

  /** load the settings of the pp */
  virtual void loadSettings(size_t);

protected:
  /** pp containing histogram to work on */
  shared_pointer _pHist;

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
 * @PPList "64": 0d into 1d (append on right end, shifting old values to the left)
 *
 * appends values from 0d histogram at end of 1d histogram and shifts the old values to the left.
 *
 * @see PostProcessor for a list of all commonly available cass.ini
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
class pp64 : public AccumulatingPostProcessor
{
public:
  /** constructor */
  pp64(const name_t&);

  /** process event */
  virtual void process(const CASSEvent&, HistogramBackend &res);

  /** load the settings of the pp */
  virtual void loadSettings(size_t);

protected:
  /** pp containing input histogram */
  shared_pointer _hist;

  /** the number of bins in the resulting histogram, range is fixed */
  size_t _size;
};






/** 0D to 2D histogramming.
 *
 * @PPList "65": Histogram two 0D values to a 2D histogram
 *
 * histograms two 0d values into one 2D Histogram. The resulting histogram
 * contains only the information from the current event. To get an average or
 * sum use PostProcessor 61 or 62.
 *
 * @see PostProcessor for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng PostProcessor/\%name\%/{XNbrBins|XLow|XUp|YNbrBins|YLow|YUp}\n
 *           properties of the 2d histogram
 * @cassttng PostProcessor/\%name\%/{HistOne|HistTwo} \n
 *           Postprocessor names containing the 0D values to histogram.
 *
 * @author Lutz Foucar
 */
class pp65 : public PostProcessor
{
public:
  /** constructor */
  pp65(const name_t &name);

  /** process event */
  virtual void process(const CASSEvent&, HistogramBackend &);

  /** load the settings */
  virtual void loadSettings(size_t);

protected:
  /** pp containing first 0D histogram to work on */
  shared_pointer _one;

  /** pp containing second 0D histogram to work on */
  shared_pointer _two;
};








/** 1D to 2D histogramming.
 *
 * @PPList "66": Histogram two 1D traces to a 2D histogram
 *
 * histograms two 1d histograms into one 2D Histogram
 *
 * @see PostProcessor for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng PostProcessor/\%name\%/{HistOne|HistTwo} \n
 *           Postprocessor names containing the 1D histograms to histogram.
 *
 * @author Lutz Foucar
 */
class pp66 : public PostProcessor
{
public:
  /** constructor */
  pp66(const name_t &name);

  /** process event */
  virtual void process(const CASSEvent&, HistogramBackend &);

  /** load the settings */
  virtual void loadSettings(size_t);

protected:
  /** pp containing first 0D histogram to work on */
  shared_pointer _one;

  /** pp containing second 0D histogram to work on */
  shared_pointer _two;
};







/** weighted 1D histogramming.
 *
 * @PPList "67": Histogram two values  with first=x, second=weight to a histogram
 *               that remembers how many times each bin has been filled.
 *
 * histograms two 0d, 1d or 2d values into a Histogram. The first of the two
 * Histogram defines the x axis bin and the second the weight. The resulting
 * Histogram is a 2d historam with 2 bins in y. The 0th bin contains the weighted
 * Histogram and the 1st bin contains the number of entries in that bin.
 *
 * @see PostProcessor for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng PostProcessor/\%name\%/{XNbrBins|XLow|XUp}\n
 *           properties of the resulting 1d histogram
 * @cassttng PostProcessor/\%name\%/{HistOne|HistTwo} \n
 *           Postprocessor names containing the values and weights to histogram.
 *
 * @author Lutz Foucar
 */
class pp67 : public PostProcessor
{
public:
  /** constructor */
  pp67(const name_t&);

  /** process event */
  virtual void process(const CASSEvent&, HistogramBackend &res);

  /** load the settings */
  virtual void loadSettings(size_t);

protected:
  /** pp containing first 0D histogram to work on */
  shared_pointer _one;

  /** pp containing second 0D histogram to work on */
  shared_pointer _two;

  /** the size of the statistics that should not be histogramed */
  size_t _statsize;
};






/** 0D and 1D to 2D histogramming.
 *
 * @PPList "68": Histogram 0D and 1D histogram to 2D histogram
 *
 * histograms a 0d and 1D Histogram to a 2d Histogram where the first 1d
 * Histogram defines the x axis and the second 0d histogram gives the y axis.
 * One only has to define the y axis since the x axis will be taken from the
 * 1d histogram
 *
 * @see PostProcessor for a list of all commonly available cass.ini
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
class pp68 : public PostProcessor
{
public:
  /** constructor */
  pp68(const name_t &name);

  /** process event */
  virtual void process(const CASSEvent&, HistogramBackend &);

  /** load the settings */
  virtual void loadSettings(size_t);

protected:
  /** pp containing first 0D histogram to work on */
  shared_pointer _one;

  /** pp containing second 0D histogram to work on */
  shared_pointer _two;
};






/** 0D to 1D scatter plot.
 *
 * @PPList "69": Use two 0D values for a scatter plot
 *
 * sets two 0d values into one 1D Histogram where the first Histogram
 * defines the x axis bin and the second is the weight.
 *
 * @see PostProcessor for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng PostProcessor/\%name\%/{XNbrBins|XLow|XUp}\n
 *           properties of the resulting 1d histogram
 * @cassttng PostProcessor/\%name\%/{HistOne|HistTwo} \n
 *           Postprocessor names containing the 0D values to histogram.
 *
 * @author Lutz Foucar
 */
class pp69 : public AccumulatingPostProcessor
{
public:
  /** constructor */
  pp69(const name_t &name);

  /** process event */
  virtual void process(const CASSEvent&, HistogramBackend &);

  /** load the settings */
  virtual void loadSettings(size_t);

protected:
  /** pp containing first 0D histogram to work on */
  shared_pointer _one;

  /** pp containing second 0D histogram to work on */
  shared_pointer _two;
};







/** Subset Histogram
 *
 * @PPList "70": Subset a Histogram
 *
 * Will copy a subset of another histogram and return it in a new histogram.
 *
 * @see PostProcessor for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng PostProcessor/\%name\%/{HistName} \n
 *           name of postprocessor that contains the histogram you want a
 *           subset from. Default is "".
 * @cassttng PostProcessor/\%name\%/{XLow|XUp} \n
 *           For 1d and 2d histogram the lower and upper range on the x-axis that one wants
 *           to include in the subset histogram. Default is 0|1
 * @cassttng PostProcessor/\%name\%/{YLow|YUp} \n
 *           In case you want to subset a 2d histogram these are the lower and upper range
 *           on the y-axis that one wants to include in the subset histogram. Default is 0|1
 *
 * @author Lutz Foucar
 */
class pp70 : public PostProcessor
{
public:
  /** constructor */
  pp70(const name_t &name);

  /** process event */
  virtual void process(const CASSEvent&, HistogramBackend &);

  /** load the settings of the pp */
  virtual void loadSettings(size_t);

protected:
  /** setup the resulting histogram
     *
     * @param hist The histogram used for setting up the resulting histogram
     */
  void setup(const HistogramFloatBase &hist);

  /** pp containing input histogram */
  shared_pointer _pHist;

  /** offset of first bin in input in Histogram coordinates */
  size_t _inputOffset;
};










/** Returns a the min or max value of a histogram
 *
 * @PPList "71": Returns the min or max value of a histogram
 *
 * @see PostProcessor for a list of all commonly available cass.ini
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
class pp71 : public PostProcessor
{
public:
  /** constructor */
  pp71(const name_t &name);

  /** process event */
  virtual void process(const CASSEvent&, HistogramBackend &);

  /** load the settings of the pp */
  virtual void loadSettings(size_t);

protected:
  /** pp containing input histogram */
  shared_pointer _pHist;

  /** the type of function used to retrive the wanted element */
  std::tr1::function<HistogramFloatBase::storage_t::const_iterator(HistogramFloatBase::storage_t::const_iterator,HistogramFloatBase::storage_t::const_iterator)> _func;
};











/** clear Histogram
 *
 * @PPList "75": Clear a Histogram
 *
 * Will clear a specific histogram when the condition is true.
 *
 * @see PostProcessor for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng PostProcessor/\%name\%/{HistName} \n
 *           name of postprocessor that contains the histogram you want a
 *           subset from. Default is "".
 *
 * @author Lutz Foucar
 */
class pp75 : public PostProcessor
{
public:
  /** constructor */
  pp75(const name_t &name);

  /** overwrite process event */
  virtual void processEvent(const CASSEvent&);

  /** load the settings of the pp */
  virtual void loadSettings(size_t);

  /** overwrite the retrieval of an histogram */
  virtual const HistogramBackend& result(const CASSEvent::id_t eventid=0);

  /** overwrite the release */
  virtual void releaseEvent(const CASSEvent &){}

protected:
  /** pp containing input histogram */
  shared_pointer _hist;
};










/** Quit Program
 *
 * @PPList "76": Quit CASS when Condition is met
 *
 * Will quit the program, when called. Make sure that it is only called when
 * you want it to be called by setting the "ConditionName" to something
 * meaningful. Defaultly "ConditionName" is set to "DefaultTrueHist" which
 * will let the program quit immediately
 *
 * @see PostProcessor for a list of all commonly available cass.ini
 *      settings.
 *
 * @author Lutz Foucar
 */
class pp76 : public PostProcessor
{
public:
  /** constructor */
  pp76(const name_t &name);

  /** process event */
  virtual void processEvent(const CASSEvent&);

  /** overwrite the retrieval of an histogram */
  virtual const HistogramBackend& result(const CASSEvent::id_t eventid=0);

  /** overwrite the release */
  virtual void releaseEvent(const CASSEvent &){}

  /** load the settings of the pp */
  virtual void loadSettings(size_t);
};







/** Checks for id on list
 *
 * @PPList "77": Checks if eventid is on a user provided list
 *
 * Checks if the id of the current event is on a user provided list. The
 * user provided list of id should be an ascii file where the ids are in lines.
 *
 * @see PostProcessor for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng PostProcessor/\%name\%/{List} \n
 *           Path and name of the file containing the list of id that should
 *           be checked. Default is "".
 *
 * @author Lutz Foucar
 */
class pp77 : public PostProcessor
{
public:
  /** constructor */
  pp77(const name_t &name);

  /** process event */
  virtual void process(const CASSEvent&, HistogramBackend &);

  /** load the settings of the pp */
  virtual void loadSettings(size_t);

protected:
  /** the list of ids */
  std::vector<uint64_t> _list;
};












/** Counter
 *
 * @PPList "78": Count how many times it has been called (Counter)
 *
 * Increases the value by one everytime its process function is called
 *
 * @PPList "78": Counter
 *
 * @see PostProcessor for a list of all commonly available cass.ini
 *      settings.
 *
 * @author Lutz Foucar
 */
class pp78 : public AccumulatingPostProcessor
{
public:
  /** constructor */
  pp78(const name_t &name);

  /** process event */
  virtual void process(const CASSEvent&, HistogramBackend &result);

  /** load the settings of the pp */
  virtual void loadSettings(size_t);
};













/** retrieve user choosable bin of 1D histogram
 *
 * @PPList "81": retrieve user choosable bin of 1D histogram
 *
 * @see PostProcessor for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng PostProcessor/\%name\%/{HistName} \n
 *           histogram name to retrieve the bin for
 * @cassttng PostProcessor/\%name\%/{RetrieveType} \n
 *           The type of bin to retrieve. Default is "max". Options are:
 *           - max: the bin containing the maximum value
 *           - min: the bin containing the minimum value
 *
 * @author Stephan Kassemeyer
 * @author Lutz Foucar
 */
class pp81 : public PostProcessor
{
public:
  /** constructor */
  pp81(const name_t &name);

  /** process event */
  virtual void process(const CASSEvent&, HistogramBackend &);

  /** load the settings of the pp */
  virtual void loadSettings(size_t);

protected:
  /** pp containing input histogram */
  shared_pointer _pHist;

  /** the type of function used to retrive the wanted bin */
  std::tr1::function<HistogramFloatBase::storage_t::const_iterator(HistogramFloatBase::storage_t::const_iterator,HistogramFloatBase::storage_t::const_iterator)> _func;
};
















/** return the statistic values of all bins of incomming histogram
 *
 * @PPList "82": user choosable statistics value of all bins of a histogram
 *
 * @see PostProcessor for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng PostProcessor/\%name\%/{HistName} \n
 *           histogram name for which the mean is calculated.
 *           Default is blubb
 * @cassttng PostProcessor/\%name\%/{Statistics} \n
 *           Type of statistic that one wants to retrieve. Default is "sum".
 *           Possible values are:
 *           - sum: Sum of values of all bins
 *           - mean: Mean value of values of all bins
 *           - stdv: Standart deviation of values of all bins
 *           - variance: Variance of values of all bins
 *
 * @author Lutz Foucar
 */
class pp82 : public PostProcessor
{
public:
  /** constructor */
  pp82(const name_t &name);

  /** process event */
  virtual void process(const CASSEvent&, HistogramBackend &);

  /** load the settings of the pp */
  virtual void loadSettings(size_t);

protected:
  /** pp containing input histogram */
  shared_pointer _pHist;

  /** define the type of statistics used */
  typedef CummulativeStatisticsCalculator<HistogramFloatBase::value_t> stat_t;

  /** the type of function used to retrive the wanted element */
  std::tr1::function<stat_t::value_type(const stat_t&)> _val;
};

















/** return full width at half maximum in given range of 1D histgoram
 *
 * @PPList "85": full width at half maximum for a peak in given range
 *
 * @see PostProcessor for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng PostProcessor/\%name\%/{HistName} \n
 *           Name of PostProcessor that contains the histogram that we want to
 *           analyze and find the FWHM. Default is 0.
 * @cassttng PostProcessor/\%name\%/{XLow|XUp} \n
 *           Lower and upper limit of the range that we look for the width at half maximum.
 *           Default is 0|1.
 * @cassttng PostProcessor/\%name\%/{Fraction} \n
 *           At which fraction of the height the width should be taken. Default
 *           is 0.5
 *
 * @author Lutz Foucar
 */
class pp85 : public PostProcessor
{
public:
  /** constructor */
  pp85(const name_t &name);

  /** process event */
  virtual void process(const CASSEvent&, HistogramBackend &);

  /** load the settings of the pp */
  virtual void loadSettings(size_t);

protected:
  /** pp containing input histogram */
  shared_pointer _pHist;

  /** the requested x-axis limits in histogram coordinates */
  std::pair<size_t,size_t> _xRange;

  /** the fraction of the range */
  float _fraction;
};









/** find step in 1d hist
 *
 * @PPList "86": find step in a given range of 1d histo
 *
 * finds the x-position of a step in a 1d hist. It does this by defining a
 * baseline from the user selected range. It then searches for the highest
 * point in the range that should contain the step. Now it looks for the first
 * x position where the y value is Fraction * (highestPoint + baselline).
 *
 * @see PostProcessor for a list of all commonly available cass.ini
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
class pp86 : public PostProcessor
{
public:
  /** constructor */
  pp86(const name_t &name);

  /** process event */
  virtual void process(const CASSEvent&, HistogramBackend &);

  /** load the settings of the pp */
  virtual void loadSettings(size_t);

protected:
  /** pp containing input histogram */
  shared_pointer _pHist;

  /** the requested x-axis limits for find the step in histogram coordinates */
  std::pair<size_t,size_t> _xRangeStep;

  /** the requested x-axis limits for find the baseline in histogram coordinates */
  std::pair<size_t,size_t> _xRangeBaseline;

  /** user fraction of the height between low and up */
  float _userFraction;
};






/** find center of Mass of 1d hist in a user given range
 *
 * @PPList "87": find center of mass in given range of 1d histo
 *
 * calculates the center of Mass in the user given range.
 *
 * @see PostProcessor for a list of all commonly available cass.ini
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
class pp87 : public PostProcessor
{
public:
  /** constructor */
  pp87(const name_t &name);

  /** process event */
  virtual void process(const CASSEvent&, HistogramBackend &);

  /** load the settings of the pp */
  virtual void loadSettings(size_t);

protected:
  /** pp containing input histogram */
  shared_pointer _pHist;

  /** the requested x-axis limits in histogram coordinates */
  std::pair<size_t,size_t> _xRange;
};





/** return axis parameter of an histogram
 *
 * @PPList "88": retrieve an axis parameter of the histogram
 *
 * @see PostProcessor for a list of all commonly available cass.ini
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
class pp88 : public PostProcessor
{
public:
  /** constructor */
  pp88(const name_t &name);

  /** process event */
  virtual void process(const CASSEvent&, HistogramBackend &);

  /** load the settings of the pp */
  virtual void loadSettings(size_t);

protected:
  /** pp containing input histogram */
  shared_pointer _pHist;

  /** the id of the axis */
  int _axisId;

  /** function to retrieve the parameter from the axis */
  std::tr1::function<float(const AxisProperty&)> _func;
};



/** low / high pass filter of 1d histogram
 *
 * @PPList "89":high or low pass filter on 1d histo
 *
 * @see PostProcessor for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng PostProcessor/\%name\%/{HistName} \n
 *           histogram name for which we count fills. Default is 0.
 * @cassttng PostProcessor/\%name\%/{FilterType} \n
 *           The filter type to use. LowPass or HighPass
 * @cassttng PostProcessor/\%name\%/{Cutoff} \n
 * @cassttng PostProcessor/\%name\%/{SampleRate} \n
 *
 * @author Lutz Foucar
 */
class pp89 : public PostProcessor
{
public:
  /** constructor */
  pp89(const name_t &name);

  /** process event */
  virtual void process(const CASSEvent&, HistogramBackend &);

  /** load the settings of the pp */
  virtual void loadSettings(size_t);

protected:
  /** high pass filtering function
   *
   * @param orig iterator to the original value
   * @param filtered iterator to the filtered value
   */
  void highPass(HistogramFloatBase::storage_t::const_iterator &orig,
                HistogramFloatBase::storage_t::iterator &filtered);

  /** low pass filtering function
   *
   * @param orig iterator to the original value
   * @param filtered iterator to the filtered value
   */
  void lowPass(HistogramFloatBase::storage_t::const_iterator &orig,
               HistogramFloatBase::storage_t::iterator &filtered);

  /** pp containing input histogram */
  shared_pointer _pHist;

  /** factor used for filtering */
  float _alpha;

  /** function to retrieve the parameter from the axis */
  std::tr1::function<void(HistogramFloatBase::storage_t::const_iterator &,
                          HistogramFloatBase::storage_t::iterator &)> _func;
};





/** returns a list of local minimal in a Histogram
 *
 * @PPList "91": returns a list of local minima in a Histogram
 *
 * @see PostProcessor for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng PostProcessor/\%name\%/{HistName} \n
 *           Histogram name of the 1d Histogram that we look for the step in.
 *           Default is 0.
 * @cassttng PostProcessor/\%name\%/{Range} \n
 *           The range to check for the local minima. Default is 10
 *
 * @author Lutz Foucar
 */
class pp91 : public PostProcessor
{
public:
  /** constructor */
  pp91(const name_t &name);

  /** process event */
  virtual void process(const CASSEvent&, HistogramBackend &);

  /** load the settings of the pp */
  virtual void loadSettings(size_t);

protected:
  /** definition of the table */
  typedef HistogramFloatBase::storage_t table_t;

  /** enum describing the contents of the resulting table */
  enum ColumnNames
  {
    Index     =  0,
    Position  =  1,
    Value     =  2,
    nbrOf
  };

  /** pp containing input histogram */
  shared_pointer _pHist;

  /** the requested x-axis limits in histogram coordinates */
  size_t _range;
};




}//end namspace cass

#endif
