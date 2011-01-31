//Copyright (C) 2011 Lutz Foucar

/**
 * @file tdc_extractor.cpp file contains class that extracts the right hits from
 *                         the tdc data
 *
 * @author Lutz Foucar
 */

#include "tdc_extractor.h"

#include "cass_event.h"
#include "cass_settings.h"

using namespace std;
using namespace cass;

ACQIRIS::SignalProducer::signals_t& ACQIRISTDC::TDCExtractor::operator()(ACQIRIS::SignalProducer::signals_t& sig)
{
  return sig;
}

void ACQIRISTDC::TDCExtractor::loadSettings(CASSSettings &s)
{

}

void ACQIRISTDC::TDCExtractor::associate(const CASSEvent &evt)
{

}
