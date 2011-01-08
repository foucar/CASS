//Copyright (C) 2003-2010 Lutz Foucar

/**
 * @file cfd.h file contains declaration of class that does a constant fraction
 *             descrimination like analysis of a waveform
 *
 * @author Lutz Foucar
 */

#ifndef _CFD_H_
#define _CFD_H_

#include <iostream>
#include "cass_acqiris.h"
#include "cass.h"
#include "signal_extractor.h"

namespace cass
{
  namespace ACQIRIS
  {
    class Channel;

    /** struct to combine the parameters that the Constant Fraction Extractors need
     *
     * @author Lutz Foucar
     */
     struct CFDParameters
     {
       typedef std::pair<double,double> timerange_t;
       typedef std::vector<timerange_t> timeranges_t;

       /** the time ranges in which the signals are found */
       timeranges_t _timeranges;

       /** the polarity that the signals have */
       Polarity _polarity;

       /** the level above which we think this is a signal (in V) */
       double _threshold;

       /** the delay of the cfd */
       int32_t _delay;

       /** the fraction of the cfd */
       double _fraction;

       /** the walk of the cfd (in V) */
       double _walk;
     };


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
    class CASS_ACQIRISSHARED_EXPORT CFD8Bit : public SignalExtractor
    {
    public:
      /** extract signals form the CASSEvent
       *
       * Calls cfd to extract the Signal from _chan. For details how see cfd.
       *
       * @return reference of the input result container
       * @param[in] sig this is the container for the results
       */
      SignalProducer::signals_t& operator()(SignalProducer::signals_t& sig);

      /** associate the event with this analyzer
       *
       * Extracts a pointer the channel for which we are there for from the
       * event with the help of extractRightChannel.
       *
       * @param evt The event from which we get the pointer to the channel.
       */
      void associate(const CASSEvent& evt);

      /** load the settings of the extractor
       *
       * Calls the loadSettings implementation to retrieve all information to be
       * able extract the signals from _channel. And to be able to extract the
       * right channel from the events in associate().
       *
       * @param s the CASSSettings object to retrieve the information from
       */
      void loadSettings(CASSSettings&);

    private:
      /** parameters for extracting the signals from the channels waveform */
      CFDParameters _parameters;

      /** the instrument that the channel is in */
      Instruments _instrument;

      /** the channelnumber of the channel we extracting the signals from */
      size_t _chNbr;

      /** pointer to the channel we are extracting the signals from */
      const Channel * _chan;
     };


    /** Finds signals in a 16 bit waveform.
     *
     * Member description is the same as in the 8 Bit verison. @see class CoM8Bit
     *
     * @author Lutz Foucar
     */
    class CASS_ACQIRISSHARED_EXPORT CFD16Bit : public SignalExtractor
    {
    public:
      SignalProducer::signals_t& operator()(SignalProducer::signals_t& sig);
      void associate(const CASSEvent& evt);
      void loadSettings(CASSSettings&);

    private:
      CFDParameters _parameters;
      Instruments   _instrument;
      size_t        _chNbr;
      const Channel*_chan;
    };

  }//end namespace acqiris
}//end namespace cass
#endif
