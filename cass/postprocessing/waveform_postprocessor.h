// Copyright (C) 2010 lmf

#ifndef _WAVEFORM_POSTPROCESSOR_H_
#define _WAVEFORM_POSTPROCESSOR_H_

#include "post_processor.h"

namespace cass
{
  //forward declarations
  class CASSEvent;
  class Histogram1DFloat;

  //class to show the last wavefrom of a channel
  class LastWaveform : public PostprocessorBackend
  {
    LastWaveform(PostProcessors::histograms_t &hist, PostProcessors::id_t id);

    //delete the histogram when you are destroyed//
    virtual ~LastWaveform();

    //copy the last waveform from the expected channel//
    virtual void operator()(const CASSEvent&);

  protected:
    size_t             _channel;  //the Acqiris channel Nbr of this processor
    Histogram1DFloat  *_waveform; //this is where we store the last waveform
  };

  //class that lets you average the waveforms
  //depending on the factor it will make a cumulative average or
  //an exponential moving average
  class AverageWaveform : public PostprocessorBackend
  {
    AverageWaveform(PostProcessors::histograms_t &hist, PostProcessors::id_t id);

    //delete the histogram when you are destroyed//
    virtual ~AverageWaveform();

    //read the average factor from cass.ini//
    virtual void loadParameters();

    //copy the last waveform from the expected channel//
    virtual void operator()(const CASSEvent&);

  protected:
    size_t             _channel;  //the Acqiris channel Nbr of this processor
    Histogram1DFloat  *_waveform; //this is where we store the averaged waveform
    float              _alpha;    //the averaging factor
  };
}

#endif
