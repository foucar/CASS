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

       /** the delay of the cfd*/
       int32_t _delay;

       /** the fraction of the cfd*/
       double _fraction;

       /** the walk of the cfd* (in V)*/
       double _walk;
     };

    /** Finds Signals in a waveform.
     *
     * Analyzes a waveform and find signals using a constant fraction algorithm.
     * It then does all the further analysis of the identified Signal.
     *
     * This class will work on waveforms of old 8 Bit Acqiris Instruments.
     *
     * @cassttng .../{Delay}\n
     *           delay in ns used by the constant fraction method:
     * @cassttng .../{Fraction}\n
     *           fraction used by the constant fraction method:
     * @cassttng .../{Walk}\n
     *            walk in Volts used by the constant fraction method:
     *
     * @author Lutz Foucar
     */
    class CASS_ACQIRISSHARED_EXPORT CFD8Bit : public SignalExtractor
    {
    public:
      /** constructor*/
      CFD8Bit()    {VERBOSEOUT(std::cout << "adding 8 bit Constant Fraction Discriminator waveformanalyzer"<<std::endl);}

      /** extract signals form the CASSEvent
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
     * @see class CoM8Bit
     *
     * @author Lutz Foucar
     */
    class CASS_ACQIRISSHARED_EXPORT CFD16Bit : public SignalExtractor
    {
    public:
      CFD16Bit()    {VERBOSEOUT(std::cout << "adding 16 bit Constant Fraction Discriminator waveformanalyzer"<<std::endl);}

      SignalProducer::signals_t& operator()(SignalProducer::signals_t& sig);

      /** associate the event with this analyzer */
      void associate(const CASSEvent& evt);

      /** load the settings of the extractor */
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

  }//end namespace acqiris
}//end namespace cass
#endif
