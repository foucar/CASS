// Copyright (C) 2010 lmf

#ifndef _WAVEFORM_POSTPROCESSOR_H_
#define _WAVEFORM_POSTPROCESSOR_H_

#include "postprocessing/postprocessor.h"
#include "postprocessing/backend.h"

namespace cass
{
  //forward declarations
  class CASSEvent;
  class Histogram1DFloat;

  //class to show the last wavefrom of a channel
  class pp4 : public PostprocessorBackend
  {
  public:

    pp4(PostProcessors::histograms_t &hist, PostProcessors::id_t id);

    //delete the histogram when you are destroyed//
    virtual ~pp4();

    //copy the last waveform from the expected channel//
    virtual void operator()(const CASSEvent&);

  protected:
    size_t             _channel;  //the Acqiris channel Nbr of this processor
    Histogram1DFloat  *_waveform; //this is where we store the last waveform
  };

  //class that lets you average the waveforms
  //depending on the factor it will make a cumulative average or
  //an exponential moving average
  class pp500 : public PostprocessorBackend
  {
  public:
    pp500(PostProcessors::histograms_t &hist, PostProcessors::id_t id);

    //delete the histogram when you are destroyed//
    virtual ~pp500();

    //read the average factor from cass.ini//
    virtual void loadParameters(size_t);

    //copy the last waveform from the expected channel//
    virtual void operator()(const CASSEvent&);

  protected:
    size_t             _channel;  //the Acqiris channel Nbr of this processor
    Histogram1DFloat  *_waveform; //this is where we store the averaged waveform
    float              _alpha;    //the averaging factor
  };
}

#endif
