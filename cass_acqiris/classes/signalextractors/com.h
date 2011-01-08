// Copyright (C) 2003 - 2010 Lutz Foucar

/**
 * @file com.h file contains declaration of class that does a center of mass
 *             analysis of a waveform
 *
 * @author Lutz Foucar
 */

#ifndef __COM_H__
#define __COM_H__

#include <iostream>
#include <utility>
#include <vector>

#include "cass.h"
#include "cass_acqiris.h"
#include "signal_extractor.h"

namespace cass
{
  namespace ACQIRIS
  {
    class Channel;

    /** struct to combine the parameters that the Center of Mass Extractors need
     *
     * @author Lutz Foucar
     */
     struct CoMParameters
     {
       typedef std::pair<double,double> timerange_t;
       typedef std::vector<timerange_t> timeranges_t;

       /** the time ranges in which the signals are found */
       timeranges_t _timeranges;

       /** the polarity that the signals have */
       Polarity _polarity;

       /** the level above which we think this is a signal (in V) */
       double _threshold;

     };

    /** Finds Signals in a waveform.
     *
     * Analyzes a waveform and find signals when they have three consecutive
     * points above the defined threshold. It then does all the further analysis
     * of the identified Signal.
     *
     * This class will work on waveforms when a datapoint depth of 8 bit is chosen
     *
     * User settable parameters via CASS.ini:\n
     * One can set these parameters for each SignalProducer of the Detectortype.
     * Therefore the settings will be for the the following signal producers:
     * - For MCP in Delayline and TofDetectors its :
     *   - AcqirisDetectors/%detectorname%/MCP
     * - For Layer Wireends in Delaylinedetectors its:
     *   - AcqirisDetectors/%detectorname%/%Layername%/%Wireendname%
     *
     * @cassttng .../CenterOfMass/{AcqirisInstrument}\n
     *           Acqiris Instrument that this channel is in:
     *           - 0: Camp (Acqiris Multiinstrument with 5 Cards (20 Channels))
     *           - 1: AMO I-ToF
     *           - 2: AMO Magnetic Bottle
     *           - 3: AMO Gas Detector
     * @cassttng .../CenterOfMass/{ChannelNumber} \n
     *           Channel within the instrument (starts counting from 0)
     * @cassttng .../CenterOfMass/Timeranges/(0,1,...)/{LowerLimit|UpperLimit}\n
     *           set of timeranges. One can set more than one range of interest.
     *           Default is no timerange, which will result in no signal will be
     *           found.
     * @cassttng .../CenterOfMass/{Polarity}\n
     *           the polarity of the signals that we are interested in:
     *           - 1: Positive Polarity
     *           - 2: Negative Polarity
     * @cassttng .../CenterOfMass/{Threshold}\n
     *           the theshold for the signals in Volts:
     *
     * @note we should let this class only identify the Signals and create the
     *       Signal list. The further analysis of the Signal should be done,
     *       when the user requests a property.
     *
     * @author Lutz Foucar
     */
    class CASS_ACQIRISSHARED_EXPORT CoM8Bit : public SignalExtractor
    {
    public:
      /** extract signals form the CASSEvent
       *
       * Calls com to extract the Signal from _chan. For details how see com.
       *
       * @return reference of the input result container
       * @param[in] sig this is the container for the results
       */
      SignalProducer::signals_t& operator()(SignalProducer::signals_t& sig);

      /** associate the event with this analyzer */
      void associate(const CASSEvent& evt);

      /** load the settings of the extractor */
      void loadSettings(CASSSettings&);

    private:
      /** parameters for extracting the signals from the channels waveform */
      CoMParameters _parameters;

      /** the instrument that the channel is in */
      Instruments _instrument;

      /** the channelnumber of the channel we extracting the signals from */
      size_t _chNbr;

      /** pointer to the channel we are extracting the signals from */
      const Channel * _chan;
     };

    /** Finds signals in a 16 bit waveform.
     *
     * Member description is the same as in the 8 Bit version. @see class CoM8Bit
     *
     * @author Lutz Foucar
     */
    class CASS_ACQIRISSHARED_EXPORT CoM16Bit : public SignalExtractor
    {
    public:
      SignalProducer::signals_t& operator()(SignalProducer::signals_t& sig);
      void associate(const CASSEvent& evt);
      void loadSettings(CASSSettings&);

    private:
      CoMParameters  _parameters;
      Instruments    _instrument;
      size_t         _chNbr;
      const Channel *_chan;
    };
  }//end namespace acqiris
}//end namespace cass

#endif
