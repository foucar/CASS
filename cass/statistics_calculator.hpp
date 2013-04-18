//Copyright (C) 2013 Lutz Foucar

/**
 * @file statistics_calculator.h contains declarations of statistic calculators
 *
 * @author Lutz Foucar
 */

#ifndef _STATISTICS_CALCULATOR_H_
#define _STATISTICS_CALCULATOR_H_

#include <vector>
#include <algorithm>

namespace cass
{
/** statistics calculator for a cummulative statistic
 *
 * This class is based on Knuths algorithm
 *
 * Donald E. Knuth (1998).
 * The Art of Computer Programming,
 * volume 2: Seminumerical Algorithms,
 * 3rd edn., p. 232.
 * Boston: Addison-Wesley.
 *
 * @tparam type of the values for the average, defines the precision
 *
 * @author Lutz Foucar
 */
template <typename Type>
class CummulativeStatisticsCalculator
{
public:
  /** define the value type */
  typedef Type value_type;

  /** default constructor
   *
   * resets the values.
   */
  CummulativeStatisticsCalculator()
  {
    reset();
  }

  /** add a datum to the distribution
   *
   * @param datum The datum to be added
   */
  void addDatum(const value_type &datum)
  {
    ++_N;
    const value_type delta(datum - _mean);
    _mean += (delta / static_cast<value_type>(_N));
    _tmp += (delta * (datum - _mean));
  }

  /** retrieve the mean of the distribution
   *
   * @return mean of the distribution
   */
  value_type mean() const
  {
    return _mean;
  }

  /** retrieve the variance of the distribution
   *
   * @return variance of the distribution
   */
  value_type variance() const
  {
    return (_tmp/static_cast<value_type>(_N - 1));
  }

  /** retrieve the standart deviation of the distribution
   *
   * @return standart deviation of the distribution
   */
  value_type stdv() const
  {
    return sqrt(variance());
  }

  /** retrieve the number of datum that have been added
   *
   * @return counts
   */
  unsigned long long count()
  {
    return _N;
  }

  /** reset the statistics */
  void reset()
  {
    _mean = _tmp = _N = 0.;
  }

private:
  /** the current mean value */
  value_type _mean;

  /** the current intermediate value that one calcs the stdv from */
  value_type _tmp;

  /** counter to see how many values have been added to the statistics */
  unsigned long long _N;
};




/** statistics calculator for a exponential moving statistics
 *
 * This class is based on Knuths algorithm
 *
 * Donald E. Knuth (1998).
 * The Art of Computer Programming,
 * volume 2: Seminumerical Algorithms,
 * 3rd edn., p. 232.
 * Boston: Addison-Wesley.
 *
 * Instead of taking 1/N one uses a fixed value. This will weigh the last N
 * values higher than the values preceeding them.
 *
 * @tparam type of the values for the average, defines the precision
 *
 * @author Lutz Foucar
 */
template <typename Type>
class MovingStatisticsCalculator
{
public:
  /** define the value type */
  typedef Type value_type;

  /** constructor
   *
   * @param nAverages the last how many datums should have highest contribution
   *                  to this.
   */
  MovingStatisticsCalculator(unsigned int nAverages=200)
  {
    nbrAverages(nAverages);
    reset();
  }

  /** add a datum to the distribution
   *
   * @param datum The datum to be added
   */
  void addDatum(const value_type &datum)
  {
    if (_firstdatum)
    {
      _mean = datum;
      _firstdatum = false;
    }
    else
    {
      const value_type delta(datum - _mean);
      _mean += (delta * _alpha);
      _tmp += (delta * (datum - _mean));
    }
  }

  /** retrieve the mean of the distribution
   *
   * @return mean of the distribution
   */
  value_type mean() const
  {
    return _mean;
  }

  /** retrieve the variance of the distribution
   *
   * @return variance of the distribution
   */
  value_type variance() const
  {
    return (_tmp * _alpha);
  }

  /** retrieve the standart deviation of the distribution
   *
   * @return standart deviation of the distribution
   */
  value_type stdv() const
  {
    return sqrt(variance());
  }

  /** reset the statistics */
  void reset()
  {
    _mean = _tmp = 0.;
    _firstdatum = true;
  }

  /** set the nbr of averages
   *
   * convert the nbr of averages to the alpha for the statistic calculation
   *
   * @param nAverages the last how many datums should have highest contribution
   *                  to this.
   */
  void nbrAverages(unsigned int nAverages)
  {
    _alpha = (2./(1.+ static_cast<value_type>(nAverages)));
  }

private:
  /** the current mean value */
  value_type _mean;

  /** the current intermediate value that one calcs the stdv from */
  value_type _tmp;

  /** how much should the current datum be weighted */
  value_type _alpha;

  /** flag to see whether first datum is added */
  bool _firstdatum;
};


/** statistics calculator for a median
 *
 * adds the datums to an internal vector, which is sorted using nth_element
 * when getting the median.
 *
 * @tparam type of the values for the average, defines the precision
 *
 * @author Lutz Foucar
 */
template <typename Type>
class MedianCalculator
{
public:
  /** define the value type */
  typedef Type value_type;
  typedef std::vector<value_type> container_type;

  /** default constructor
   *
   * resets the values.
   */
  MedianCalculator()
  {
    reset();
  }

  /** add a datum to the distribution
   *
   * @param datum The datum to be added
   */
  void addDatum(const value_type &datum)
  {
    _container.push_back(datum);
  }

  /** retrieve the median of the distribution
   *
   * uses median(const container_type &container) to calc the median
   *
   * @return median of the distribution
   */
  value_type median() const
  {
    median(_container);
  }

  /** reset the statistics */
  void reset()
  {
    _container.clear();
  }

  /** calculate the mean of a container_type container
   *
   * a static function so that one can use it without having an object of this
   * class.
   *
   * copy the container, sort the copied container and retrieve the central
   * element
   *
   * @return the median of the values inside the container
   * @param container the container from whos values the median should be
   *                  calculated
   */
  static value_type median(const container_type &container)
  {
    container_type cc(container);
    const size_t medianpos(0.5*cc.size());
    std::nth_element(cc.begin, cc.begin() + medianpos, cc.end());
    const value_type medianval(cc[medianpos]);
    return medianval;
  }

private:
  /** the current mean value */
  container_type _container;
};

}//end namespace cass
#endif
