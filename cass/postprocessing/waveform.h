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
   * @cassttng PostProcessor/\%name\%/{InstrumentId} \n
   *           The instrument id of the acqiris instrument that contains the
   *           channel. Default is 8. Options are:
   *           - 8: Camp Acqiris
   *           - 4: AMO ITof Acqiris
   *           - 2: AMO GD Acqiris
   *           - 5: AMO Mbes Acqiris
   * @cassttng PostProcessor/\%name\%/{ChannelNbr} \n
   *           The channel number of the acqiris instrument. Default is 0.
   * @cassttng PostProcessor/\%name\%/{ConditionName} \n
   *           0D Postprocessor name that we check before filling image.
   *           if this setting is not defined, this postprocessor is unconditional.
   *           Therefore its always true.
   * @cassttng PostProcessor/\%name\%/{Hide} \n
   *           Flag that will hide this postprocessor in cassview's combobox.
   *           Default is false
   *
   * @todo adjust the name to the id that it will have
   * @author Lutz Foucar
   */
  class pp110 : public PostprocessorBackend
  {
  public:
    /** constructor */
    pp110(PostProcessors &ppc, const PostProcessors::key_t &key);

    /** copy the last waveform from the channel*/
    virtual void process(const CASSEvent&);

    /** load the settings of this pp */
    virtual void loadSettings(size_t);

  protected:
    /** Mutex for locking this postprocessor*/
    QMutex _mutex;

    /** the instrument that contains the channel this postprocessor will work on */
    cass::ACQIRIS::Instruments _instrument;

    /** the Acqiris channel number of this processor */
    size_t _channel;
  };
}

#endif
