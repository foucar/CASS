//Copyright (C) 2013 Lutz Foucar

/**
 * @file statistics_calculator.h contains declarations of statistic calculators
 *
 * @author Lutz Foucar
 */

#ifndef _STATISTICS_CALCULATOR_H_
#define _STATISTICS_CALCULATOR_H_

namespace cass
{
/** statistics calculator base class
 *
 * @author Lutz Foucar
 */
class StatisticsCalculator
{
public:
  /** default constructor */
  StatisticsCalculator() {}

  /** virtual default destructor */
  ~StatisticsCalculator() {}

  /** add a datum to the distribution
   *
   * @param datum The datum to be added
   */
  virtual void addDatum(const float /*datum*/);

  /** retrieve the mean of the distribution
   *
   * @return mean of the distribution
   */
  virtual float getMean() const;

  /** retrieve the standart deviation of the distribution
   *
   * @return standart deviation of the distribution
   */
  virtual float getStdv() const;

  /** retrieve the skewness of the distribution
   *
   * @return skewness of the distribution
   */
  virtual float getSkewness() const;
};
}//end namespace cass
#endif
