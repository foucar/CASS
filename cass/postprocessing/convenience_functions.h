// Copyright (C) 2010 Lutz Foucar

#ifndef __CONVENIENCE_FUNCTIONS_H__
#define __CONVENIENCE_FUNCTIONS_H__

#include "postprocessor.h"

namespace cass
{
  /** binary function for weighted substracting.
   *
   * @author Lutz Foucar
   */
  class weighted_minus : std::binary_function<float, float, float>
  {
  public:
    /** constructor.
     *
     * @param first_weight the weight value of the first substrant
     * @param second_weight the weight value of the second substrant
     */
    weighted_minus(float first_weight, float second_weight)
      :_first_weight(first_weight),_second_weight(second_weight)
    {}
    /** operator */
    float operator() (float first, float second)
    { return first * _first_weight - second * _second_weight;}
  protected:
    float _first_weight, _second_weight;
  };


  /** binary function for averaging.
   * this operator is capable of performing a cumulative moving average and
   * a Exponential moving average.
   * @see http://en.wikipedia.org/wiki/Moving_average
   * @author Lutz Foucar
   */
  class Average : std::binary_function<float,float,float>
  {
  public:
    /** constructor.
     * initializes the \f$\alpha\f$ value
     * @param alpha The \f$\alpha\f$ value
     */
    explicit Average(float alpha)
      :_alpha(alpha)
    {}
    /** the operator calculates the average using the function
     * \f$Y_N = Y_{N-1} + \alpha(y-Y_{N-1})\f$
     * where when \f$\alpha\f$ is equal to N it is a cumulative moving average,
     * otherwise it will be a exponential moving average.
     */
    float operator()(float currentValue, float Average_Nm1)
    {
      return Average_Nm1 + _alpha*(currentValue - Average_Nm1);
    }
  protected:
    /** \f$\alpha\f$ for the average calculation */
    float _alpha;
  };


  /** binary function for averaging.
   * this operator performs a moving sum
   * @author Nicola Coppola
   */
  class TimeAverage : std::binary_function<float,float,float>
  {
  public:
    /** constructor.
     * initializes the nEvents value
     * @param nEvents The number of Events used up to now
     */
    explicit TimeAverage(float nEvents)
      :_nEvents(nEvents)
    {}
    /** the operator calculates the average over the last _nEvents
     */
    float operator()(float currentValue, float Average_Nm1)
    {
      if(_nEvents!=0)
        return ( Average_Nm1 * (_nEvents-1) + currentValue ) /_nEvents;
      else
        return currentValue;
    }
  protected:
    /** nEvents for the average calculation */
    float _nEvents;
  };


  /** function to set the 1d histogram properties from the ini file.
   * @param[out] hist pointer to the 1D Histogram whos properties should be updated
   *            (will be deleted and created with new settings)
   * @param[in] key the key of the postprocessor too look up in cass.ini
   * @author Lutz Foucar
   */
  void set1DHist(cass::HistogramBackend*& hist, PostProcessors::key_t key);


  /** function to set the 2d histogram properties from the ini file.
   * @param[out] hist pointer to the 2D Histogram whos properties should be updated
   *            (will be deleted and created with new settings)
   * @param[in] key the key of the postprocessor too look up in cass.ini
   * @author Lutz Foucar
   */
  void set2DHist(cass::HistogramBackend*& hist, PostProcessors::key_t key);


  /** function to retrieve and validate a postprocessors dependency
   * @return true when the dependcy exists
   * @param[in] pp reference to the postprocessor instance that contains the histograms
   * @param[in] key the key of the postprocessor asking for another postprocessors id
   * @param[in] param_name paramenter name of the dependency in qsettings
   * @param[out] dependid reference to the pp id that we retrieve from qsettings
   */
  bool retrieve_and_validate(cass::PostProcessors &pp,
                             cass::PostProcessors::key_t key,
                             const char * param_name,
                             cass::PostProcessors::key_t &dependid);

}

#endif
