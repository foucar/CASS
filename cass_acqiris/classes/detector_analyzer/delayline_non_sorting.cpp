// Copyright (C) 2010 Lutz Foucar

/**
 * @file delayline_non_sorting.cpp ile contains the class that finds detectorhits
 *                               without sorting
 *
 * @author Lutz Foucar
 */

#include "delayline_non_sorting.h"

using namespace std;
using namespace cass::ACQIRIS;
using namespace std::tr1;

DelaylineNonSorting::DelaylineNonSorting()
  :DetectorAnalyzerBackend()
{}

detectorHits_t& DelaylineNonSorting::operator()(detectorHits_t &hits)
{
  return hits;
}

void DelaylineNonSorting::loadSettings(CASSSettings& s, DelaylineDetector &d)
{

}
