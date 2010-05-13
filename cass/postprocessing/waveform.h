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

  /** last acqiris channel waveform.
   *
   * Class to show the last wavefrom of a channel.
   *
   * implements postprocessor id's 4-23.
   *
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
    /** delete the histogram when you are destroyed*/
    virtual ~pp4();
    /** copy the last waveform from the channel*/
    virtual void operator()(const CASSEvent&);

  protected:
    /** Mutex for locking this postprocessor*/
    QMutex _mutex;
    /** the instrument that contains the channel this postprocessor will work on*/
    cass::ACQIRIS::Instruments _instrument;
    /** the Acqiris channel number of this processor*/
    size_t _channel;
    /** this is where we store the waveform */
    Histogram1DFloat  *_waveform;
  };



  /** Averaged Acqiris channel's wavefrom.
   *
   * Class that lets you average the waveforms depending on the factor it will
   * make a cumulative average or an exponential moving average. Uses the Average
   * binary operator to do the averaging.
   * @see Average
   *
   * @cassttng PostProcessor/p\%id\%/{NumberOfAverages}\n
   *           averaging length. In case one want to have a cummulative
   *           averaging set averaging length to 1.
   *
   * implements postprocessor id's 500-519.
   *
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
    virtual void loadSettings(size_t);
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
