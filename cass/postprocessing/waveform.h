// Copyright (C) 2010 Lutz Foucar

#ifndef _WAVEFORM_POSTPROCESSOR_H_
#define _WAVEFORM_POSTPROCESSOR_H_

#include "postprocessing/postprocessor.h"
#include "postprocessing/backend.h"
#include "cass_acqiris.h"

namespace cass
{
  //forward declarations
  class CASSEvent;
  class Histogram1DFloat;

  /*! Last Waveforms

  class to show the last wavefrom of a channel
  @todo include also the other instruments
  @author Lutz Foucar
  */
  class pp4 : public PostprocessorBackend
  {
  public:

    pp4(PostProcessors::histograms_t &hist, PostProcessors::id_t id);

    /*! delete the histogram when you are destroyed*/
    virtual ~pp4();

    /*! copy the last waveform from the expected channel*/
    virtual void operator()(const CASSEvent&);

  protected:
    /** the instrument for this postprocessor*/
    cass::ACQIRIS::Instruments _instrument;
    /*! the Acqiris channel Nbr of this processor*/
    size_t _channel;
    /*! this is where we store the last waveform */
    Histogram1DFloat  *_waveform;
  };



  /*! Averaged Wavefrom

  class that lets you average the waveforms
  depending on the factor it will make a cumulative average or
  in exponential moving average

  @todo include also the other instruments
  @author Lutz Foucar
  */
  class pp500 : public PostprocessorBackend
  {
  public:
    pp500(PostProcessors::histograms_t &hist, PostProcessors::id_t id);
    /*! delete the histogram when you are destroyed */
    virtual ~pp500();
    /*! read the average factor from cass.ini*/
    virtual void loadParameters(size_t);
    /*! copy the last waveform from the expected channel*/
    virtual void operator()(const CASSEvent&);

  protected:
    /** the instrument for this postprocessor*/
    cass::ACQIRIS::Instruments _instrument;
    /*! the Acqiris channel Nbr of this processor*/
    size_t _channel;
    /*! this is where we store the averaged waveform*/
    Histogram1DFloat *_waveform;
    /*! the averaging factor */
    float _alpha;
  };
}

#endif
