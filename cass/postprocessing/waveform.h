// Copyright (C) 2010 Lutz Foucar

/** @file waveform.h file contains acqiris data retrieval postprocessor
 *                   declaration
 *
 * @author Lutz Foucar
 */

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
   * @see PostprocessorBackend for a list of all commonly available cass.ini
   *      settings.
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
   *
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
    /** the instrument that contains the channel this postprocessor will work on */
    cass::ACQIRIS::Instruments _instrument;

    /** the Acqiris channel number of this processor */
    size_t _channel;
  };
}

#endif
