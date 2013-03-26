//Copyright (C) 2013 Lutz Foucar

/**
 * @file statistics_calculator.cpp contains definitions of statistic calculators
 *
 * @author Lutz Foucar
 */

#include <iostream>
#include <stdexcept>

#include "statistics_calculator.h"

using namespace std;
using namespace cass;

void StatisticsCalculator::addDatum(const float)
{
  throw runtime_error("StatisticsCalculator::addDatum(): Function not implemented");
}

float StatisticsCalculator::getMean() const
{
  throw runtime_error("StatisticsCalculator::getMean(): Function not implemented");
}

float StatisticsCalculator::getStdv() const
{
  throw runtime_error("StatisticsCalculator::getStdv(): Function not implemented");
}

float StatisticsCalculator::getSkewness() const
{
  throw runtime_error("StatisticsCalculator::getSkewness(): Function not implemented");
}
