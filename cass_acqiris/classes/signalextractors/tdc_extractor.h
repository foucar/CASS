//Copyright (C) 2011 Lutz Foucar

/**
 * @file tdc_extractor.h file contains class that extracts the right hits from
 *                       the tdc data
 *
 * @author Lutz Foucar
 */

#ifndef _TDC_EXTRACTOR_H_
#define _TDC_EXTRACTOR_H_

#include <iostream>

#include "cass_acqiris.h"
#include "signal_extractor.h"

namespace cass
{
  class CASSSettings;

  namespace ACQIRISTDC
  {
    class Channel;

    /** Finds Signals in a waveform.
     *
     * Analyzes a waveform and find signals using a constant fraction algorithm.
     * It then does all the further analysis of the identified Signal.
     *
     * This class will work on waveforms with a depth of 8 Bits.
     *
     * User settable parameters via CASS.ini:\n
     * One can set these parameters for each SignalProducer of the Detectortype.
     * Therefore the settings will be for the the following signal producers:
     * - For MCP in Delayline and TofDetectors its :
     *   - AcqirisDetectors/%detectorname%/MCP
     * - For Layer Wireends in Delaylinedetectors its:
     *   - AcqirisDetectors/%detectorname%/%Layername%/%Wireendname%
     *
     * @cassttng .../ConstantFraction/{AcqirisInstrument}\n
     *           Acqiris Instrument that this channel is in:
     *           - 0: Camp (Acqiris Multiinstrument with 5 Cards (20 Channels))
     *           - 1: AMO I-ToF
     *           - 2: AMO Magnetic Bottle
     *           - 3: AMO Gas Detector
     * @cassttng .../ConstantFraction/{ChannelNumber} \n
     *           Channel within the instrument (starts counting from 0)
     * @cassttng .../ConstantFraction/Timeranges/(0,1,...)/{LowerLimit|UpperLimit}\n
     *           set of timeranges. One can set more than one range of interest.
     *           Default is no timerange, which will result in no signal will be
     *           found.
     * @cassttng .../ConstantFraction/{Polarity}\n
     *           the polarity of the signals that we are interested in:
     *           - 1: Positive Polarity
     *           - 2: Negative Polarity
     * @cassttng .../ConstantFraction/{Threshold}\n
     *           the theshold for the signals in Volts:
     * @cassttng .../ConstantFraction/{Delay}\n
     *           delay in ns used by the constant fraction method:
     * @cassttng .../ConstantFraction/{Fraction}\n
     *           fraction used by the constant fraction method:
     * @cassttng .../ConstantFraction/{Walk}\n
     *           walk of the Constant Fraction Discriminator in Volts
     *
     * @author Lutz Foucar
     */
    class CASS_ACQIRISSHARED_EXPORT TDCExtractor : public ACQIRIS::SignalExtractor
    {
    public:
      /** extract signals form the CASSEvent
       *
       * Calls cfd to extract the Signal from _chan. For details how see cfd.
       *
       * @return reference of the input result container
       * @param[in] sig this is the container for the results
       */
      ACQIRIS::SignalProducer::signals_t& operator()(ACQIRIS::SignalProducer::signals_t& sig);

      /** associate the event with this extractor
       *
       * Extracts a pointer the channel for which we are there for from the
       * event.
       *
       * @param evt The event from which we get the pointer to the channel.
       */
      void associate(const CASSEvent& evt);

      /** load the settings of the extractor
       *
       * retrieve the timerange gates that the signals of the detector came in
       *
       * @param s the CASSSettings object to retrieve the information from
       */
      void loadSettings(CASSSettings&);

    private:
      /** the ranges in which the real signals are in */
      std::vector<std::pair<double,double> > _timeranges;

      /** pointer to the channel we are extracting the signals from */
      const Channel *_chan;
    };
  }
}
#endif

