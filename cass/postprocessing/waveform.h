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

  /** Last Waveform.
   * Class to show the last wavefrom of a channel. Objects created from this
   * class will work as postprocessor 4-23.
   * @todo make sure that y axis will show up in volts
   * @todo include also the other Acqiris instruments
   * @author Lutz Foucar
   */
  class pp4 : public PostprocessorBackend
  {
  public:
    /** constructor.
     * @param ppc reference to the postprocessor container that contains
     *        this postprocessor.
     * @param id the id of this postprocessor object
     */
    pp4(PostProcessors &ppc, PostProcessors::id_t id);
    /*! delete the histogram when you are destroyed*/
    virtual ~pp4();
    /*! copy the last waveform from the expected channel*/
    virtual void operator()(const CASSEvent&);

  protected:
    /** Mutex for locking this postprocessor*/
    QMutex _mutex;
    /** the instrument that contains the channel this postprocessor will work on*/
    cass::ACQIRIS::Instruments _instrument;
    /*! the Acqiris channel number of this processor*/
    size_t _channel;
    /*! this is where we store the last waveform */
    Histogram1DFloat  *_waveform;
  };



  /** Averaged Wavefrom.
   * class that lets you average the waveforms
   * depending on the factor it will make a cumulative average or
   * in exponential moving average.
   * Objects created from this class will work as postprocessor 500-519.
   * @todo make sure that y axis will show up in volts
   * @todo include also the other Acqiris instruments
   * @author Lutz Foucar
   */
  class pp500 : public PostprocessorBackend
  {
  public:
    /** constructor.
     * @param ppc reference to the postprocessor container that contains
     *        this postprocessor.
     * @param id the id of this postprocessor object
     */
    pp500(PostProcessors &ppc, PostProcessors::id_t id);
    /*! delete the histogram when you are destroyed */
    virtual ~pp500();
    /*! read the average factor from cass.ini*/
    virtual void loadParameters(size_t);
    /*! copy the last waveform from the expected channel*/
    virtual void operator()(const CASSEvent&);

  protected:
    /** Mutex for locking this postprocessor*/
    QMutex _mutex;
    /** the instrument that contains the channel this postprocessor will work on*/
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
