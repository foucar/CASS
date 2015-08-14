// Copyright (C) 2010 -2015 Lutz Foucar

/**
 * @file operations.h file contains processors that will operate
 *                     on results of other processors
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
#include "statistics_calculator.hpp"

namespace cass
{



/** Operation on 2 results
 *
 * @PPList "1":Operation on 2 results value by value
 *
 * @see Processor for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng Processor/\%name\%/{InputOne} \n
 *           the processor name that contain the first result.
 *           Needs to be of the same dimension and size as the second.
 * @cassttng Processor/\%name\%/{InputTwo} \n
 *           the processor name that contain the second result.
 *           Needs to be of the same dimension and size as the first
 * @cassttng Processor/\%name\%/{Operation} \n
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
class pp1 : public Processor
{
public:
  /** constructor */
  pp1(const name_t & name);

  /** load the settings of this pp */
  virtual void loadSettings(size_t);

  /** process event */
  virtual void process(const CASSEvent& evt, result_t&);

protected:
  /** processor containing the first result */
  shared_pointer _one;

  /** processor containing the second result */
  shared_pointer _two;

  /** the operand */
  std::tr1::function<result_t::value_t(result_t::value_t, result_t::value_t)> _op;
};






/** Operation on result with value
 *
 * @PPList "2":Operation on result with value
 *
 * @see Processor for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng Processor/\%name\%/{InputName} \n
 *           the processor name that contains the result to operate on. Needs to
 *           be implemented, because default is "Unknown", which is invalid.
 * @cassttng Processor/\%name\%/{Value} \n
 *           Either the constant value for the operation or a 0D Processor
 *           containing the value for the operation. Default is 1
 * @cassttng Processor/\%name\%/{Operation} \n
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
 * @cassttng Processor/\%name\%/{ValuePos} \n
 *           Chooses where in the operation the Value or the value taken from
 *           the 0D Processor will be. Default is "first". Possible values
 *           are:
 *           - first: Value will be first operand
 *           - second: Value will be second operand
 *
 * @author Lutz Foucar
 */
class pp2 : public Processor
{

public:
  /** constructor */
  pp2(const name_t &name);

  /** load the settings of this pp */
  virtual void loadSettings(size_t);

  /** process event */
  virtual void process(const CASSEvent& evt, result_t&);

protected:
  /** define the unary operation */
  typedef std::tr1::function<result_t::value_t(result_t::value_t)> unaryoperation_t;

  /** define the binary operation */
  typedef std::tr1::function<result_t::value_t(result_t::value_t,result_t::value_t)> binaryoperation_t;

  /** define how to get the value */
  typedef std::tr1::function<result_t::value_t(const CASSEvent::id_t &)> valueRetrieval_t;

  /** define how to get the right parameter position */
  typedef std::tr1::function<unaryoperation_t(result_t::value_t)> setParamPos_t;

  /** bind the value to the first parameter of the binaryoperation
   *
   * @return function call where value is bound to the fist parameter
   */
  unaryoperation_t ValAtFirst(result_t::value_t val);

  /** bind the value to the second parameter of the binaryoperation
   *
   * @return function call where value is bound to the second parameter
   */
  unaryoperation_t ValAtSecond(result_t::value_t val);

  /** retrieve value from Processor
   *
   * @returns value stored in _valuePP
   * @param id id of the event for which the value should be returned
   */
  result_t::value_t valueFromPP(const CASSEvent::id_t &id);

  /** retrieve value constant
   *
   * @returns _value
   * @param evt ignored
   */
  result_t::value_t valueFromConst(const CASSEvent::id_t &evt);

protected:
  /** processor containing input result */
  shared_pointer _hist;

  /** processor containing 0D value for the unary operation */
  shared_pointer _valuePP;

  /** the value for the unary operation */
  result_t::value_t _value;

  /** the operand */
  binaryoperation_t _op;

  /** function to set the value to the requested parameter position */
  setParamPos_t _setParamPos;

  /** function to retrieve the value for the unary operation */
  valueRetrieval_t _retrieveValue;
};







/** Apply boolean NOT to 0D result
 *
 * @PPList "4": Apply boolean NOT to 0D result
 *
 * @see Processor for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng Processor/\%name\%/{InputName} \n
 *           the processor name that contains the result to invert
 *
 * @author Lutz Foucar
 */
class pp4 : public Processor
{
public:
  /** constructor */
  pp4(const name_t &name);

  /** process event */
  virtual void process(const CASSEvent&, result_t&);

  /** load the settings of this pp */
  virtual void loadSettings(size_t);

protected:
  /** processor containing result */
  shared_pointer _one;
};








/** Check whether result is in range.
 *
 * @PPList "9": Check whether sum value of result is in range
 *
 * In case the input is not a 0D result, the contents of all bins will be
 * summed and the sum is then checked whether it is within the limits.
 *
 * @see Processor for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng Processor/\%name\%/{InputName} \n
 *           the processor name that contain the result for checking the value
 * @cassttng Processor/\%name\%/{UpperLimit|LowerLimit} \n
 *           Upper and Lower limit of the range to check. Default is 0,0.
 *           The following check will be done
 *           \f$ LowerLimit < value < UpperLimit \f$ Thus, both enpoints are
 *           exclusive.
 *
 * @author Lutz Foucar
 */
class pp9 : public Processor
{
public:
  /** constructor */
  pp9(const name_t &name);

  /** process event */
  virtual void process(const CASSEvent&, result_t&);

  /** load the settings of this pp */
  virtual void loadSettings(size_t);

protected:
  /** processor containing first result */
  shared_pointer _one;

  /** the requested range that the value should be in */
  std::pair<result_t::value_t,result_t::value_t> _range;
};










/** Constant Value processor.
 *
 * @PPList "12": Constant Value
 *
 * @cassttng Processor/\%name\%/{Value} \n
 *           The value of the processors 0d result Default is 0.
 * @cassttng Processor/\%name\%/{ValueType} \n
 *           The type of constant that will we returned. Default is '0D'.
 *           Possible values are:
 *           - '0D': A 0d result will be returned
 *           - '1D': A 1d array result will be returned
 *           - '2D': A 2d image result will be returned
 * @cassttng Processor/\%name\%/{XNbrBins|XLow|XUp|YNbrBins|YLow|YUp}\n
 *           Optional settings, needed when selected 1D or 2D as type
 *
 * @author Lutz Foucar
 */
class pp12 : public Processor
{
public:
  /** constructor */
  pp12(const name_t &);

  /** overwrite default behaviour and just return the constant */
  virtual const result_t& result(const CASSEvent::id_t)
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
  result_t::shared_pointer _res;
};











/** return the input
 *
 * @PPList "13": return the input (identiy operation)
 *
 * @see Processor for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng Processor/\%name\%/{InputName} \n
 *           the processor name that contain the input result to be copied
 *
 * @author Lutz Foucar
 */
class pp13 : public Processor
{
public:
  /** constructor */
  pp13(const name_t &name);

  /** process event */
  virtual void process(const CASSEvent&, result_t&);

  /** load the settings of this pp */
  virtual void loadSettings(size_t);

protected:
  /** processor containing result */
  shared_pointer _one;
};








/** Check whether value has changed.
 *
 * @PPList "15": Check whether value of 0d result has changed
 *
 * check whether a value has changed with respekt to the previous event.
 *
 * @cassttng Processor/\%name\%/{InputName} \n
 *           The Processor name that contain the 0D result that should be
 *           monitored.
 * @cassttng Processor/\%name\%/{Difference} \n
 *           The maximum allowed difference between the previous and the current
 *           value of the 0D result. Default is 0 which results in a value
 *           given by std::numeric_limits<float>::epsilon().
 *
 * @author Lutz Foucar
 */
class pp15 : public AccumulatingProcessor
{
public:
  /** constructor */
  pp15(const name_t &name);

  /** process event */
  virtual void process(const CASSEvent&, result_t&);

  /** load the settings of this pp */
  virtual void loadSettings(size_t);

protected:
  /** pp containing the value to check */
  shared_pointer _hist;

  /** the value of the previous event */
  result_t::value_t _previousVal;

  /** the maximum difference to previous val that is accepted */
  result_t::value_t _difference;
};







/** Threshold result.
 *
 * @PPList "40": Threshold result
 *
 * @see Processor for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng Processor/\%name\%/{InputName} \n
 *           processor name with result that should be thresholded. Default is 0.
 * @cassttng Processor/\%name\%/{Threshold} \n
 *           Factor with which threshold value. Default is 0.
 *
 * @author Thomas White
 */
class pp40 : public Processor
{
public:
  /** constructor */
  pp40(const name_t &name);

  /** process event */
  virtual void process(const CASSEvent&, result_t&);

  /** load the settings of this pp */
  virtual void loadSettings(size_t);

protected:
  /** pp containing input result */
  shared_pointer _one;

  /** the threshold */
  result_t::value_t _threshold;
};












/** Threshold result based upon information from another result
 *
 * @PPList "41": Threshold result based upon information from another result
 *
 * set the bin of a result to a user requested value when the corresponding
 * value of the threshold is within a user requested range.
 *
 * @see Processor for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng Processor/\%name\%/{InputName} \n
 *           processor name with result that should be thresholded.
 *           Default is Unknown.
 * @cassttng Processor/\%name\%/{ThresholdName} \n
 *           Processor that will be used to threshold the Input
 *           Default is Unknown.
 * @cassttng Processor/\%name\%/{UserVal} \n
 *           The value that will be set when the value of the corresponding bin
 *           is within the boundaries. Default is 0
 * @cassttng Processor/\%name\%/{LowerLimit|UpperLimit} \n
 *           The boundaries within which the value of the ThresholdName has to
 *           be in order to set the value of the result to UserVal.
 *           Default is 0.5|1.5. Both Limits are exclusive.
 *
 * @author Lutz Foucar
 */
class pp41 : public Processor
{
public:
  /** constructor */
  pp41(const name_t &name);

  /** process event */
  virtual void process(const CASSEvent&, result_t&);

  /** load the settings of this pp */
  virtual void loadSettings(size_t);

protected:
  /** check if value is in range and return another value
   *
   * @return user set value if value is in range
   * @param val the value that will be returned if checkval is not in range
   * @param checkval the value to check if it is in range
   */
  result_t::value_t checkrange(result_t::value_t val, result_t::value_t checkval);

protected:
  /** pp containing input result */
  shared_pointer _one;

  /** pp containing threshold result */
  shared_pointer _threshold;

  /** the value that will be set */
  result_t::value_t _userVal;

  /** the lower boundary of the range */
  result_t::value_t _lowerBound;

  /** the upper boundary of the range */
  result_t::value_t _upperBound;
};







/** Projection of 2d result
 *
 * @PPList "50": Project 2D result onto a axis
 *
 * Project a user defined range of a 2d result onto a user defined axis.
 *
 * @see Processor for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng Processor/\%name\%/{InputName} \n
 *           Name of the processor containing the  2D result that will be
 *           projected
 * @cassttng Processor/\%name\%/{Low|Up} \n
 *           Upper and lower bound of the area to project. The endpoints are
 *           defined like \f$ [Low,Up[ \f$. Default is -1e6|1e6
 * @cassttng Processor/\%name\%/{Axis} \n
 *           The axis we want to project to. Default is "xAxis".
 *           Possible choises are:
 *           - "xAxis": projects the selected range in y to the x-Axis
 *           - "yAxis": projects the selected range in x to the y-Axis
 *
 * @author Lutz Foucar
 */
class pp50 : public Processor
{
public:
  /** constructor */
  pp50(const name_t &name);

  /** process event */
  virtual void process(const CASSEvent&, result_t&);

  /** load the settings of the pp */
  virtual void loadSettings(size_t);

private:
  /** project 2d result to x axis
   *
   * @param src iterator to the begining of the 2d result that one wants to
   *            project
   * @param dest iterator to the beginning of the resulting projection
   */
   void projectToX(result_t::const_iterator src,
                   result_t::iterator dest);

  /** project 2d result to y axis
   *
   * @param src iterator to the beginning of the 2d result that one wants to
   *            project
   * @param dest iterator to the beginning of the resulting projection
   */
   void projectToY(result_t::const_iterator src,
                   result_t::iterator dest);

private:
  /** processor containing the 2d result we want to project */
  shared_pointer _pHist;

  /** range we want to project */
  std::pair<int,int> _xRange;

  /** range we want to project */
  std::pair<int,int> _yRange;

  /** the nbr of bins in the original image */
  size_t _nX;

  /** function that will do the projection */
  std::tr1::function<void(result_t::const_iterator,
                          result_t::iterator)> _project;
};










/** Integral of 1D result.
 *
 * @PPList "51": Integral of 1D result
 *
 * integrate the values of a 1d result within a user set range
 *
 * @see Processor for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng Processor/\%name\%/{InputName} \n
 *           processor name with 1D-result that we create the intgral from.
 *           Default is Unknown.
 * @cassttng Processor/\%name\%/{XLow|XUp} \n
 *           Upper and lower bound of the area to integrate. The endpoints are
 *           defined like \f$ [XLow,Xup[ \f$. Default is -1e6 ... 1e6
 *
 * @author Lutz Foucar
 */
class pp51 : public Processor
{
public:
  /** constructor */
  pp51(const name_t &name);

  /** process event */
  virtual void process(const CASSEvent&, result_t&);

  /** load the settings of the pp */
  virtual void loadSettings(size_t);

private:
  /** processor containing the 1d result we want to integrate */
  shared_pointer _input;

  /** range we want to have the integral over in result bins */
  std::pair<int,int> _range;
};










/** store previous result of other Processor
 *
 * @PPList "56": Contains the result of the previous event
 *
 * Stores a previous version of another result.
 *
 * @see Processor for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng Processor/\%name\%/{InputName} \n
 *           processor name containing the result that will be stored
 *
 * @author Lutz Foucar
 */
class pp56 : public Processor
{
public:
  /** constructor */
  pp56(const name_t &name);

  /** process event */
  virtual void process(const CASSEvent&, result_t&);

  /** load the settings */
  virtual void loadSettings(size_t);

protected:
  /** the previous result */
  result_t _previous;

  /** processor containing result to store */
  shared_pointer _pHist;
};















/** Weighted Projection of 2D result.
 *
 * @PPList "57": Weighted Project 2D result onto a axis
 *
 * devides each bin by the number of values that have been added in
 * this bin. Exclusion values will not be added to the bin.
 *
 * @see Processor for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng Processor/\%name\%/{InputName} \n
 *           Name of the processor containing the  2D result that will be
 *           projected
 * @cassttng Processor/\%name\%/{Low|Up} \n
 *           Upper and lower bound of the area to project. Default is
 *           -1e6 ... 1e6
 * @cassttng Processor/\%name\%/{Axis} \n
 *           The axis we want to project to. Default is "xAxis".
 *           Possible choises are:
 *           - "xAxis": projects the selected range in y to the x-Axis
 *           - "yAxis": projects the selected range in x to the y-Axis
 * @cassttng Processor/\%name\%/{ExclusionValue} \n
 *           The value that will be excluded when doing the projection. The
 *           result will be normilzed by the amount of bins that have been
 *           summed. Default is 0.
 *
 * @author Lutz Foucar
 */
class pp57 : public Processor
{
public:
  /** constructor */
  pp57(const name_t &name);

  /** process event */
  virtual void process(const CASSEvent&, result_t&);

  /** load the settings of the pp */
  virtual void loadSettings(size_t);

private:
  /** integrate the rows
   *
   * @param src 2d image that will be projected
   * @param result vector were the projection will be written to
   * @param norm vector where the normalization values will be added to
   */
  void projectToX(result_t::const_iterator src,
                  result_t::iterator result,
                  result_t::iterator norm);

  /** integrate the columns
   *
   * @param src 2d image that will be projected
   * @param result vector were the projection will be written to
   * @param norm vector where the normalization values will be added to
   */
  void projectToY(result_t::const_iterator src,
                  result_t::iterator result,
                  result_t::iterator norm);

private:
  /** processor containing the 2d result we want to project */
  shared_pointer _pHist;

  /** range in X we want to project */
  std::pair<int,int> _Xrange;

  /** range in Y we want to project */
  std::pair<int,int> _Yrange;

  /** the size of the original image in X */
  size_t _nX;

  /** the value that should be excluded in the summation */
  float _excludeVal;

  /** the function used to project the image */
  std::tr1::function<void(result_t::const_iterator,
                          result_t::iterator,
                          result_t::iterator)> _project;
};













/** 0D,1D or 2D to 1D histogramming.
 *
 * @PPList "60": Histogram 0D, 1D or 2D values to a 1D result
 *
 * histograms all values of 0D, 1D or 2D into a 1D result. This result
 * holds only the histogrammed values of one event. Use Processors 61 or
 * 62 to average or sum up this result, respectively.
 * It has the capability to histogram the values with user provided weights.
 * If the infomration about how many times a particular bin has been filled is
 * needed use pp67 instead.
 *
 * @see Processor for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng Processor/\%name\%/{ValuesName} \n
 *           processor name containing the values to histogram
 * @cassttng Processor/\%name\%/{Weight} \n
 *           The weight, Can either be a constant value or a processor name
 *           containing the weights. The processor needs to be of the same
 *           shape as the input. The individual entires are the weights of the
 *           corresponding bins in the input.
 *           Default 1
 * @cassttng Processor/\%name\%/{XNbrBins|XLow|XUp}\n
 *           properties of the resulting 1D histogram
 *
 * @author Lutz Foucar
 */
class pp60 : public Processor
{
public:
  /** constructor */
  pp60(const name_t &name);

  /** process event */
  virtual void process(const CASSEvent&, result_t&);

  /** load the settings */
  virtual void loadSettings(size_t);

protected:
  /** define the function for histogramming */
  typedef std::tr1::function<void(CASSEvent::id_t,
                                  result_t::const_iterator,
                                  result_t::const_iterator,
                                  result_t &)> func_t;

  /** histogam with weights from another processor
   *
   * @param id the event id to get the right weight from the processor
   * @param in iterator to the beginning of the input
   * @param last iterator to the end of the  data input
   * @param result reference to the result that does the histograming
   */
  void histogramWithWeights(CASSEvent::id_t id,
                            result_t::const_iterator in,
                            result_t::const_iterator last,
                            result_t & result);

  /** histogam with user provided constant weight
   *
   * @param unused an unused paramter
   * @param in iterator to the beginning of the input
   * @param last iterator to the end of the  data input
   * @param result reference to the result that does the histograming
   */
  void histogramWithConstant(CASSEvent::id_t unused,
                             result_t::const_iterator in,
                             result_t::const_iterator last,
                             result_t & result);
protected:
  /** processor containing result to histogram */
  shared_pointer _input;

  /** the weight in case it is a constant */
  result_t::value_t _weight;

  /** processor containing the weight */
  shared_pointer _weightProc;

  /** function used for histogramming */
  func_t _histogram;
};






/** result averaging.
 *
 * @PPList "61": Average of a result
 *
 * Running or cummulative average of a result. Could also be a squared average.
 *
 * @see Processor for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng Processor/\%name\%/{AveragingType}\n
 *           The type of averaging that should be performed. Default is "Normal"
 *           - "Normal": a normal average will be used
 *           - "Square"; a square averaging will be performed
 * @cassttng Processor/\%name\%/{NbrOfAverages}\n
 *           how many images should be averaged. When value is 0 its a cummulative
 *           average. Default is 1.
 * @cassttng Processor/\%name\%/{InputName} \n
 *           processor name containing the result that we average.
 *
 * @author Lutz Foucar
 */
class pp61 : public AccumulatingProcessor
{
public:
  /** constructor */
  pp61(const name_t &name);

  /** process event */
  virtual void process(const CASSEvent&, result_t&);

  /** load the settings */
  virtual void loadSettings(size_t);

protected:
  /** function for normal averaging.
   *
   * this operator is capable of performing a cumulative moving average and
   * a Exponential moving average.
   * @see http://en.wikipedia.org/wiki/Moving_average
   *
   * the operator calculates the average using the function
   * \f$ave_{new} = ave_{old} + \scale(val-ave_{old})\f$
   * where when \f$\scale\f$ is equal to N it is a cumulative moving average,
   * otherwise it will be a exponential moving average.
   *
   * @return the new average
   * @param val the current value to be added to the average
   * @param aveOld the old average
   * @param scale the scale with which the new value will be weighted
   */
  result_t::value_t average(result_t::value_t val,
                            result_t::value_t aveOld,
                            result_t::value_t scale);

  /** function that will calculate the square average
   *
   * the operator calculates the square average using the function
   * \f$ave_{new} = ave_{old} + \scale(val*val-ave_{old})\f$
   * where when \f$\scale\f$ is equal to N it is a cumulative moving average.
   *
   * @return the new average
   * @param val the current value to be added to the average
   * @param aveOld the old average
   * @param scale the scale with which the new value will be weighted
   */
  result_t::value_t squareAverage(result_t::value_t val,
                                  result_t::value_t aveOld,
                                  result_t::value_t scale);

protected:
  /** alpha for the running average */
  result_t::value_t _alpha;

  /** function that will do the averagin */
  std::tr1::function<result_t::value_t(result_t::value_t,result_t::value_t,result_t::value_t)> _func;

  /** processor containing result to average */
  shared_pointer _pHist;
};





/** result summing.
 *
 * @PPList "62": Summing up of results
 *
 * Sums up results.
 *
 * @see Processor for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng Processor/\%name\%/{InputName} \n
 *           processor name containing the result that we sum up.
 *
 * @author Lutz Foucar
 */
class pp62 : public AccumulatingProcessor
{
public:
  /** constructor */
  pp62(const name_t&);

  /** process event */
  virtual void process(const CASSEvent&, result_t&);

  /** load the settings */
  virtual void loadSettings(size_t);

protected:
  /** processor containing result to sum */
  shared_pointer _pHist;
};






/** time average of 0d result.
 *
 * @PPList "63": Time Average of a result over given time-intervals
 *
 * Makes an running average of a given result over a given time period.
 *
 * @see Processor for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng Processor/\%name\%/{InputName} \n
 * @cassttng Processor/\%name\%/{MinTime|MaxTime} \n
 *           Minimum and Maximum Time to plot in the result. Default
 *           is 0 ... 300 (WARNING: for the moment this setting is not active)
 * @cassttng Processor/\%name\%/{NbrSamples} \n
 *           Number of values that are used per second to calculate the average.
 *           Default is 5
 *
 * @author Nicola Coppola
 */
class pp63 : public AccumulatingProcessor
{
public:
  /** constructor */
  pp63(const name_t&);

  /** process event */
  virtual void process(const CASSEvent&, result_t&);

  /** load the settings of the pp */
  virtual void loadSettings(size_t);

protected:
  /** processor containing result to work on */
  shared_pointer _pHist;

  /** range of time that we use for the angular distribution */
  std::pair<size_t,size_t> _timerange;

  /** the number of bins in the result, range is fixed */
  size_t _nbrSamples;

  /** the number of samples seen up to now and used in the point */
  size_t _num_seen_evt;

  //@{
  /** time when the first samples was used in the point in time */
  time_t _when_first_evt;
  uint32_t _first_fiducials;
  //@}
};





/** record 0d result into 1d result.
 *
 * @PPList "64": 0d into 1d (append on right end, shifting old values to the left)
 *
 * appends values from results at end of this result and shifts the old values
 * to the left.
 *
 * @see Processor for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng Processor/\%name\%/{InputName} \n
 *           Name of the processor that conatins the result whos values will
 *           be appended to this.
 * @cassttng Processor/\%name\%/{Size} \n
 *           Number of values that are stored
 *           Default is 10000
 *
 * @author Stephan Kassemeyer
 */
class pp64 : public AccumulatingProcessor
{
public:
  /** constructor */
  pp64(const name_t&);

  /** process event */
  virtual void process(const CASSEvent&, result_t&);

  /** load the settings of the pp */
  virtual void loadSettings(size_t);

protected:
  /** processor containing input result */
  shared_pointer _hist;

  /** the number of bins in the result, range is fixed */
  size_t _size;
};






/** 0D to 2D histogramming.
 *
 * @PPList "65": Histogram two 0D values to a 2D result
 *
 * histograms two 0d values into one 2D result. The result
 * contains only the information from the current event. To get an average or
 * sum use Processor 61 or 62.
 *
 * @see Processor for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng Processor/\%name\%/{XNbrBins|XLow|XUp|YNbrBins|YLow|YUp}\n
 *           properties of the 2d histogram
 * @cassttng Processor/\%name\%/{XName|YName} \n
 *           processor names containing the 0D values to histogram.
 *
 * @author Lutz Foucar
 */
class pp65 : public Processor
{
public:
  /** constructor */
  pp65(const name_t &name);

  /** process event */
  virtual void process(const CASSEvent&, result_t&);

  /** load the settings */
  virtual void loadSettings(size_t);

protected:
  /** processor containing X axis value */
  shared_pointer _one;

  /** processor containing Y-axis value */
  shared_pointer _two;
};








/** 1D to 2D combining
 *
 * @PPList "66": combines two 1D traces to a 2D result
 *
 * combines two 1d results into one 2D result
 * The 2d result will be computed as follows
 * \f$ result_{i,j} = X_{i} * Y_{j} \f$, where \f$ 0 \leq i < X_{max}\f$ and
 * \f$ 0 \leq j < Y_{max} \f$
 *
 * This processor relies on the fact the the input shapes are fixed for all
 * events.
 *
 * @see Processor for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng Processor/\%name\%/{XName|YName} \n
 *           processor names containing the 1D results for the x an y values.
 *
 * @author Lutz Foucar
 */
class pp66 : public Processor
{
public:
  /** constructor */
  pp66(const name_t &name);

  /** process event */
  virtual void process(const CASSEvent&, result_t&);

  /** load the settings */
  virtual void loadSettings(size_t);

protected:
  /** pp containing X-axis 1D result to combine */
  shared_pointer _one;

  /** pp containing Y-axis 1D result to combine */
  shared_pointer _two;
};







/** 1D histogramming with keeping track of how many times a bin has been filled
 *
 * @PPList "67": Histogram two values  with first=x, second=weight to a histogram
 *               that remembers how many times each bin has been filled.
 *
 * Histograms two 0d, 1d or 2d values into a 1D result. The first of the two
 * results defines the x-axis bin and the second the weight. The result
 * is a 2d result with 2 bins in y. The 0th bin contains the weighted
 * Histogram and the 1st bin contains the number of entries in the bins.
 *
 * @see Processor for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng Processor/\%name\%/{XNbrBins|XLow|XUp}\n
 *           properties of the resulting 1d histogram
 * @cassttng Processor/\%name\%/{ValuesName|WeightsName} \n
 *           processor names containing the values and weights to histogram.
 *
 * @author Lutz Foucar
 */
class pp67 : public Processor
{
public:
  /** constructor */
  pp67(const name_t&);

  /** process event */
  virtual void process(const CASSEvent&, result_t&);

  /** load the settings */
  virtual void loadSettings(size_t);

protected:
  /** processor containing the values to histogram */
  shared_pointer _one;

  /** processor containing the weights for the values */
  shared_pointer _two;
};






/** 0D and 1D to 2D combining.
 *
 * @PPList "68": Combines 0D and 1D result to 2D result
 *
 * combines a 0D and 1D result to a 2d result where the 1d
 * result defines the x axis and the second 0d result defines the bin on the y
 * axis where the 1D result will be written to.
 * One only has to define the y axis since the x axis will be taken from the
 * 1D result
 *
 * @see Processor for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng Processor/\%name\%/{YNbrBins|YLow|YUp}\n
 *           properties of the y axis of the 2D result
 * @cassttng Processor/\%name\%/{XName}\n
 *           processr containing the 1D result.
 * @cassttng Processor/\%name\%/{YName} \n
 *           processor containing the 0D values to define the bin on the y-axis
 *
 * @author Lutz Foucar
 */
class pp68 : public Processor
{
public:
  /** constructor */
  pp68(const name_t &name);

  /** process event */
  virtual void process(const CASSEvent&, result_t&);

  /** load the settings */
  virtual void loadSettings(size_t);

protected:
  /** processor containing the 1D result */
  shared_pointer _one;

  /** processor containing 0D result */
  shared_pointer _two;
};






/** 0D to 1D scatter plot.
 *
 * @PPList "69": Use two 0D values for a scatter plot
 *
 * sets two 0d values into one 1D result where the first 0D result
 * defines the x axis bin and the second defines the value of that bin.
 * Unlike in a histogram the weight will not be added but it will be set to the
 * weight value. As this is an accumulating processor the values will be kept
 * until they are overwritten.
 *
 * @see Processor for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng Processor/\%name\%/{XNbrBins|XLow|XUp}\n
 *           properties of the 1D result.
 * @cassttng Processor/\%name\%/{XName|YName} \n
 *           processor names containing the 0D values for the scatter plot
 *
 * @author Lutz Foucar
 */
class pp69 : public AccumulatingProcessor
{
public:
  /** constructor */
  pp69(const name_t &name);

  /** process event */
  virtual void process(const CASSEvent&, result_t&);

  /** load the settings */
  virtual void loadSettings(size_t);

protected:
  /** processor containing x-axis 0D result */
  shared_pointer _one;

  /** processor containing y-axis 0D result */
  shared_pointer _two;
};







/** Subset result
 *
 * @PPList "70": Subset a result
 *
 * Will copy a subset of another result and return it in a new result.
 *
 * @see Processor for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng Processor/\%name\%/{InputName} \n
 *           name of processor that contains the result you want a
 *           subset from.
 * @cassttng Processor/\%name\%/{XLow|XUp} \n
 *           For 1d and 2d result the lower and upper range on the x-axis that
 *           one wants to include in the subset result.
 *           These endpoints are defined like \f$ [XLow,XUp[ \f$
 *           Default is 0|1
 * @cassttng Processor/\%name\%/{YLow|YUp} \n
 *           In case you want to subset a 2d result these are the lower and
 *           upper range on the y-axis that one wants to include in the
 *           subset result.
 *           These endpoints are defined like \f$ [YLow,YUp[ \f$
 *           Default is 0|1
 *
 * @author Lutz Foucar
 */
class pp70 : public Processor
{
public:
  /** constructor */
  pp70(const name_t &name);

  /** process event */
  virtual void process(const CASSEvent&, result_t&);

  /** load the settings of the pp */
  virtual void loadSettings(size_t);

protected:
  /** processor containing input result */
  shared_pointer _input;

  /** offset in x and y to the first bin of the input */
  std::pair<int,int> _offset;
};










/** Returns a the min or max value of a result
 *
 * @PPList "71": Returns the min or max value of a result
 *
 * @see Processor for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng Processor/\%name\%/{RetrieveType} \n
 *           Type of function used to retrieve the requested value in the
 *           result. Default is "max". Possible values are:
 *           - "max": return the maximum value in the result
 *           - "min": return the minimum value in the result
 * @cassttng Processor/\%name\%/{InputName} \n
 *           Name of the processor that contains the result to find the
 *           requested value in.
 *
 * @author Lutz Foucar
 */
class pp71 : public Processor
{
public:
  /** constructor */
  pp71(const name_t &name);

  /** process event */
  virtual void process(const CASSEvent&, result_t&);

  /** load the settings of the pp */
  virtual void loadSettings(size_t);

protected:
  /** processor containing the input result */
  shared_pointer _pHist;

  /** the type of function used to retrive the wanted element */
  std::tr1::function<result_t::const_iterator(result_t::const_iterator,result_t::const_iterator)> _func;
};











/** clear result
 *
 * @PPList "75": Clear a result
 *
 * Will clear the result of a different processor when the condition is true.
 *
 * @see Processor for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng Processor/\%name\%/{InputName} \n
 *           name of processor that contains the result to clear
 *
 * @author Lutz Foucar
 */
class pp75 : public Processor
{
public:
  /** constructor */
  pp75(const name_t &name);

  /** overwrite process event */
  virtual void processEvent(const CASSEvent&);

  /** load the settings of the pp */
  virtual void loadSettings(size_t);

  /** overwrite the retrieval of an result */
  virtual const result_t& result(const CASSEvent::id_t eventid=0);

  /** overwrite the release */
  virtual void releaseEvent(const CASSEvent &){}

protected:
  /** processor containing input result */
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
 * @see Processor for a list of all commonly available cass.ini
 *      settings.
 *
 * @author Lutz Foucar
 */
class pp76 : public Processor
{
public:
  /** constructor */
  pp76(const name_t &name);

  /** process event */
  virtual void processEvent(const CASSEvent&);

  /** overwrite the retrieval of an result */
  virtual const result_t& result(const CASSEvent::id_t eventid=0);

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
 * @see Processor for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng Processor/\%name\%/{List} \n
 *           Path and name of the file containing the list of id that should
 *           be checked. Default is "".
 *
 * @author Lutz Foucar
 */
class pp77 : public Processor
{
public:
  /** constructor */
  pp77(const name_t &name);

  /** process event */
  virtual void process(const CASSEvent&, result_t&);

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
 * @see Processor for a list of all commonly available cass.ini
 *      settings.
 *
 * @author Lutz Foucar
 */
class pp78 : public AccumulatingProcessor
{
public:
  /** constructor */
  pp78(const name_t &name);

  /** process event */
  virtual void process(const CASSEvent&, result_t&);

  /** load the settings of the pp */
  virtual void loadSettings(size_t);
};













/** retrieve user choosable type of bin of 1D result
 *
 * @PPList "81": retrieve user choosable type of bin of 1D result
 *
 * @see Processor for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng Processor/\%name\%/{InputName} \n
 *           Name of the Processor that contains the result where the requested
 *           bin is retrieved from.
 * @cassttng Processor/\%name\%/{RetrieveType} \n
 *           The type of bin to retrieve. Default is "max". Options are:
 *           - max: the bin containing the maximum value
 *           - min: the bin containing the minimum value
 *
 * @author Stephan Kassemeyer
 * @author Lutz Foucar
 */
class pp81 : public Processor
{
public:
  /** constructor */
  pp81(const name_t &name);

  /** process event */
  virtual void process(const CASSEvent&, result_t&);

  /** load the settings of the pp */
  virtual void loadSettings(size_t);

protected:
  /** processor containing input result */
  shared_pointer _pHist;

  /** the type of function used to retrive the wanted bin */
  std::tr1::function<result_t::const_iterator(result_t::const_iterator,result_t::const_iterator)> _func;
};
















/** return the statistic values of all bins of a result
 *
 * @PPList "82": user choosable statistics value of all bins of a result
 *
 * @see Processor for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng Processor/\%name\%/{InputName} \n
 *           Processor containing the result for which the requested statistical
 *           value is calculated.
 * @cassttng Processor/\%name\%/{Statistics} \n
 *           Type of statistic that one wants to retrieve. Default is "sum".
 *           Possible values are:
 *           - sum: Sum of values of all bins
 *           - mean: Mean value of values of all bins
 *           - stdv: Standart deviation of values of all bins
 *           - variance: Variance of values of all bins
 *
 * @author Lutz Foucar
 */
class pp82 : public Processor
{
public:
  /** constructor */
  pp82(const name_t &name);

  /** process event */
  virtual void process(const CASSEvent&, result_t&);

  /** load the settings of the pp */
  virtual void loadSettings(size_t);

protected:
  /** processor containing input result */
  shared_pointer _pHist;

  /** define the type of statistics used */
  typedef CummulativeStatisticsCalculator<result_t::value_t> stat_t;

  /** the type of function used to retrive the wanted element */
  std::tr1::function<stat_t::value_type(const stat_t&)> _val;
};

















/** return full width at half maximum in given range of 1D result
 *
 * @PPList "85": full width at half maximum for a peak in given range
 *
 * @see Processor for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng Processor/\%name\%/{InputName} \n
 *           Name of Processor that contains the result that the FWHM should be
 *           extracted for.
 * @cassttng Processor/\%name\%/{XLow|XUp} \n
 *           Lower and upper endpoints of the range that the FWHM will be
 *           calculated for.
 *           The endpoints define the range as \f$ [XLow,XUp[ \f$
 *           Default is 0|1.
 * @cassttng Processor/\%name\%/{Fraction} \n
 *           At which fraction of the height the width should be taken. Default
 *           is 0.5
 *
 * @author Lutz Foucar
 */
class pp85 : public Processor
{
public:
  /** constructor */
  pp85(const name_t &name);

  /** process event */
  virtual void process(const CASSEvent&, result_t&);

  /** load the settings of the pp */
  virtual void loadSettings(size_t);

protected:
  /** processor containing input result */
  shared_pointer _pHist;

  /** the requested x-axis limits in bins */
  std::pair<int,int> _xRange;

  /** the fraction of the range */
  float _fraction;
};









/** find step in 1d result
 *
 * @PPList "86": find step in a given range of 1d result
 *
 * finds the x-position of a step in a 1d result. It does this by defining a
 * baseline from the user selected range. It then searches for the highest
 * point in the range that should contain the step. Now it looks for the first
 * x position where the y value is \f$ Fraction * (highestPoint + baseline) \f$.
 *
 * @see Processor for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng Processor/\%name\%/{InputName} \n
 *           Name  of the processor that contains the 1D result that
 *           we look for the step in.
 * @cassttng Processor/\%name\%/{XLow|XUp} \n
 *           Lower and upper endpoints of the range that we look for the step in.
 *           The endpoints define the range as \f$ [XLow,XUp[ \f$
 *           Default is 0|1.
 * @cassttng Processor/\%name\%/{BaselineLow|BaselineUp} \n
 *           Lower and upper endpoints of the range that we use to calculate the
 *           Baseline.
 *           The endpoints define the range as \f$ [BaselineLow,BaselineUp[ \f$
 *           Default is 0|1.
 * @cassttng Processor/\%name\%/{Fraction} \n
 *           The Fraction between the baseline and the highest value that
 *           should be taken when searching for the right point. Default is
 *           0.5
 *
 * @author Lutz Foucar
 */
class pp86 : public Processor
{
public:
  /** constructor */
  pp86(const name_t &name);

  /** process event */
  virtual void process(const CASSEvent&, result_t&);

  /** load the settings of the pp */
  virtual void loadSettings(size_t);

protected:
  /** processor containing input result */
  shared_pointer _pHist;

  /** the requested x-axis limits for find the step in bins */
  std::pair<int,int> _xRangeStep;

  /** the requested x-axis limits for find the baseline in bins */
  std::pair<int,int> _xRangeBaseline;

  /** user fraction of the height between low and up */
  float _userFraction;
};






/** find center of Mass of 1D result in a user given range
 *
 * @PPList "87": find center of mass in given range of 1D result
 *
 * calculates the center of Mass in the user given range.
 *
 * @see Processor for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng Processor/\%name\%/{InputName} \n
 *           Name of the processor that conatins the 1D resault that we look
 *           for the step in.
 * @cassttng Processor/\%name\%/{XLow|XUp} \n
 *           Lower and upper limit of the range that we look for the step.
 *           The endpoints define the range as \f$ [XLow,XUp[ \f$
 *           Default is 0|1.
 *
 * @author Lutz Foucar
 */
class pp87 : public Processor
{
public:
  /** constructor */
  pp87(const name_t &name);

  /** process event */
  virtual void process(const CASSEvent&, result_t&);

  /** load the settings of the pp */
  virtual void loadSettings(size_t);

protected:
  /** processor containing input result */
  shared_pointer _pHist;

  /** the requested x-axis limits in bins */
  std::pair<int,int> _xRange;
};





/** return axis parameter of a result
 *
 * @PPList "88": retrieve an axis parameter of a result
 *
 * @see Processor for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng Processor/\%name\%/{InputName} \n
 *           Name of the processor that conatains the result from which the
 *           requested axis parameter is retrieved.
 * @cassttng Processor/\%name\%/{AxisParameter} \n
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
class pp88 : public Processor
{
public:
  /** constructor */
  pp88(const name_t &name);

  /** process event */
  virtual void process(const CASSEvent&, result_t&);

  /** load the settings of the pp */
  virtual void loadSettings(size_t);

protected:
  /** processor containing input result */
  shared_pointer _pHist;

  /** the id of the axis */
  result_t::axis_name _axisId;

  /** function to retrieve the parameter from the axis */
  std::tr1::function<result_t::value_t(const result_t::axe_t&)> _func;
};










/** low / high pass filter of 1D result
 *
 * @PPList "89":high or low pass filter on 1D result
 *
 * inspired by code found at
 * http://stackoverflow.com/questions/13882038/implementing-simple-high-and-low-pass-filters-in-c
 * copright Slater Tyrus
 *
 * HighPass function:
@verbatim
float RC = 1.0/(CUTOFF*2*3.14);
float dt = 1.0/SAMPLE_RATE;
float alpha = RC/(RC + dt);
float filteredArray[numSamples];
filteredArray[0] = data.recordedSamples[0];
for (i = 1; i<numSamples; i++){
  filteredArray[i] = alpha * (filteredArray[i-1] + data.recordedSamples[i] - data.recordedSamples[i-1]);
}
data.recordedSamples = filteredArray;
@endverbatim
 *
 * LowPass function:
@verbatim
float RC = 1.0/(CUTOFF*2*3.14);
float dt = 1.0/SAMPLE_RATE;
float alpha = dt/(RC+dt);
float filteredArray[numSamples];
filteredArray[0] = data.recordedSamples[0];
for(i=1; i<numSamples; i++){
  filteredArray[i] = filteredArray[i-1] + (alpha*(data.recordedSamples[i] - filteredArray[i-1]));
}
data.recordedSamples = filteredArray;
@endverbatim
 *
 * @see Processor for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng Processor/\%name\%/{InputName} \n
 *           Name of the processor that contains the 1D result to filter
 * @cassttng Processor/\%name\%/{FilterType} \n
 *           The filter type to use. Default is "LowPass". Possible values are:
 *           - "LowPass": a low pass filter
 *           - "HighPass": a high pass filter
 * @cassttng Processor/\%name\%/{Cutoff} \n
 *           The cutoff of the filter.
 * @cassttng Processor/\%name\%/{SampleRate} \n
 *           The sampling rate of the filter
 *
 * @author Lutz Foucar
 */
class pp89 : public Processor
{
public:
  /** constructor */
  pp89(const name_t &name);

  /** process event */
  virtual void process(const CASSEvent&, result_t&);

  /** load the settings of the pp */
  virtual void loadSettings(size_t);

protected:
  /** high pass filtering function
   *
   * @param orig iterator to the original value
   * @param filtered iterator to the filtered value
   */
  void highPass(result_t::const_iterator orig,
                result_t::iterator filtered);

  /** low pass filtering function
   *
   * @param orig iterator to the original value
   * @param filtered iterator to the filtered value
   */
  void lowPass(result_t::const_iterator orig,
               result_t::iterator filtered);

  /** processor containing input result */
  shared_pointer _pHist;

  /** factor used for filtering */
  float _alpha;

  /** function to retrieve the parameter from the axis */
  std::tr1::function<void(result_t::const_iterator,
                          result_t::iterator)> _func;
};





/** returns a list of local minima in a 1D result
 *
 * @PPList "91": returns a list of local minima in a 1D result
 *
 * It will look for a maximum value that is an actual values within the user
 * defined range. If data is the 1D array the range is as follows:
 * \f$ Range \leq i < size-range \f$, where i is the index of the array and size
 * is the size of the array.
 *
 * All found minima will be added to a table like result
 *
 * @see Processor for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng Processor/\%name\%/{InputName} \n
 *           Name of the Processor that contains the 1D result where the local
 *           minima will be retrieved from
 * @cassttng Processor/\%name\%/{Range} \n
 *           The range to check for the local minima. Default is 10
 *
 * @author Lutz Foucar
 */
class pp91 : public Processor
{
public:
  /** constructor */
  pp91(const name_t &name);

  /** process event */
  virtual void process(const CASSEvent&, result_t&);

  /** load the settings of the pp */
  virtual void loadSettings(size_t);

protected:
  /** definition of the table */
  typedef result_t::storage_t table_t;

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
