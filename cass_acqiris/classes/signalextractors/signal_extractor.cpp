//Copyright (C) 2010 Lutz Foucar

/**
 * @file signal_extractor.cpp file contains base class for all classes that
 *                            extract signals from the recorded data
 *
 * @author Lutz Foucar
 */

#include "signal_extractor.h"

cass::ACQIRIS::SignalExtractor* cass::ACQIRIS::SignalExtractor::instance(SignalExtractorType)
{
//  return new WaveformAnalyzerBackend;
}
