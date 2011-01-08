//Copyright (C) 2010 Lutz Foucar

/**
 * @file waveform_signal.h file contains the classes that describe how to
 *                         analyze the waveform and stores the result.
 *
 * @author Lutz Foucar
 */

#ifndef _SIGNAL_H_
#define _SIGNAL_H_

#include <algorithm>
#include <map>
#include <vector>
#include <string>

#include "cass_acqiris.h"
#include "peak.h"

namespace cass
{
  //forward declaration
  class CASSSettings;

  class CASSEvent;

  namespace ACQIRIS
  {
    class SignalExtractor;

    /** Functor returning whether Peak is in Range.
     *
     * This class is beeing used by std::find_if as Predicate
     *
     * @author Lutz Foucar
     */
    class PeakInRange
    {
    public:
      /** constructor intializing the range*/
      PeakInRange(double low, double up)
        :_range(std::make_pair(low,up))
      {}

      /** operator used by find_if.
       *
       * @return peak is smaller than or equal to up AND greater than low
       */
      bool operator()(const Peak &peak)const
      {
        return (peak >_range.first && peak <= _range.second);
      }

    private:
      /** the range*/
      std::pair<double,double> _range;
    };





    /** A Signal Producer.
     *
     * This class describes all signal producing elements of a detector. It
     * contains an extractor for the produced signals from the event and a list
     * of the signals it produced.
     *
     * User settable parameters via CASS.ini
     * - general access to these parameters depends on the detector type:
     *   - In Delaylinedetectors its for
     *      - MCP: AcqirisDetectors/%detectorname%/MCP
     *      - Layers Wireends: AcqirisDetectors/%detectorname%/%Layername%/%Wireendname%
     *   - In TofDetectors: AcqirisDetectors/%detectorname%/Signal
     *
     * Then the specific settings for these objects are:
     * @cassttng .../{AcqirisInstrument}\n
     *           Acqiris Instrument that this channel is in
     * @cassttng .../{ChannelNumber} \n
     *           Channel within the instrument (starts counting from 0)
     * @cassttng .../{LowerTimeRangeLimit|UpperTimeRangeLimit}\n
     *           time range of the channel that we are interested in.
     * @cassttng .../{LowerGoodTimeRangeLimit|UpperGoodTimeRangeLimit}\n
     *           time range of the channel that "good" signals will appear. This
     *           is used by delayline detectors for displaying the first good
     *           hits and the timesum.
     * @cassttng .../{WaveformAnalysisMethod}\n
     *           the method type that will be used to analyze the waveform.
     *           there are the following options :
     *           - 0:com 8 bit waveform
     *           - 1:com 16 bit waveform
     *           - 2:cfd 8 bit waveform
     *           - 3:cfd 16 bit waveform
     *           @see cass::ACQIRIS::CoM, cass::ACQIRIS::CFD
     * @cassttng .../{Polarity}\n
     *           the polarity of the signals that we are interested in:
     *           - 1: Positive Polarity
     *           - 2: Negative Polarity
     * @cassttng .../{Threshold}\n
     *           the theshold for the signals in Volts:
     * @cassttng .../{Delay}\n
     *           delay in ns used by the constant fraction method:
     * @cassttng .../{Fraction}\n
     *           fraction used by the constant fraction method:
     * @cassttng .../{Walk}\n
     *            walk in Volts used by the constant fraction method:
     *
     * @author Lutz Foucar
     */
    class CASS_ACQIRISSHARED_EXPORT SignalProducer
    {
    public:
      typedef std::vector<std::map<std::string,double> > signals_t;

    public:
      /** default constructor.
       *
       * intializing the variables describing the extraction with nonsense,
       * since they have to be loaded by loadSettings from cass.ini
       */
      SignalProducer()
        :_instrument(Camp1),
         _chNbr(99),
         _trLow(0),
         _trHigh(0),
         _polarity(Bad),
         _threshold(5000),
         _delay(0),
         _fraction(5),
         _walk(200),
         _isNewEvent(true)
      {}

    public:
      /** loads the parameters.
       *
       * load the parameters from cass.ini, should only be called by class
       * containing this class
       */
      void loadSettings(CASSSettings *p, const char * signalname);

      /** assciate the event with this signalproducer */
      void associate(const CASSEvent& evt);

    public:
      /** convience for easier readable code, is a vector of Peak*/
      typedef std::vector<Peak> peaks_t;

    public:
      /** setter.
       *
       * @note we should make sure, that it will not be needed anymore, since the
       *       getter should make sure that all peaks are set using the correct
       *       function
       */
      peaks_t           &peaks()              {return _peaks;}

      /** getter.
       *
       * @note when calling this we should check whether it has alredy been called
       *       for the event if not so, then create the peaks using the requested
       *        waveform analysis
       */
      const peaks_t     &peaks()const         {return _peaks;}

    public:
      /** return the time of the first peak in the "good" time range*/
      double firstGood() const;

      /** return the signals
       *
       * When a new event was associated with this prodcuer, then it will first
       * extract all signals from the event data otherwise it will just return
       * the signals
       */
      signals_t& output();

    public:
      //@{
      /** setter */
      size_t            &channelNbr()         {return _chNbr;}
      double            &trLow()              {return _trLow;}
      double            &trHigh()             {return _trHigh;}
      double            &grLow()              {return _grLow;}
      double            &grHigh()             {return _grHigh;}
      Polarity          &polarity()           {return _polarity;}
      double            &threshold()          {return _threshold;}
      int32_t           &delay()              {return _delay;}
      double            &fraction()           {return _fraction;}
      double            &walk()               {return _walk;}
      Instruments       &instrument()         {return _instrument;}
      //@}
      //@{
      /** getter */
      size_t             channelNbr()const    {return _chNbr;}
      double             trLow()const         {return _trLow;}
      double             trHigh()const        {return _trHigh;}
      double             grLow()const         {return _grLow;}
      double             grHigh()const        {return _grHigh;}
      Polarity           polarity()const      {return _polarity;}
      double             threshold()const     {return _threshold;}
      int32_t            delay()const         {return _delay;}
      double             fraction()const      {return _fraction;}
      double             walk()const          {return _walk;}
      Instruments        instrument()const    {return _instrument;}
      //@}

    private:
      //things important to know how to analyze the waveform//
      //set by the user via parameters//
      /** the Instrument that the channel will be in*/
      Instruments _instrument;

      /** This Channels Number in the Acqiris Instrument*/
      size_t _chNbr;

      /** lower edge of the timerange events can happen in*/
      double _trLow;

      /** upper edge of the timerange events can happen in*/
      double _trHigh;

      /** lower edge of the timerange good single events can happen in*/
      double _grLow;

      /** upper edge of the timerange good single  events can happen in*/
      double _grHigh;

      /** the Polarity the Signal has*/
      Polarity _polarity;

      /** the Noiselevel for this channel (in adc bytes)*/
      double _threshold;

      /** the delay of the cfd*/
      int32_t _delay;

      /** the fraction of the cfd*/
      double _fraction;

      /** the walk of the cfd*/
      double _walk;

      //container for the results of the waveform analysis
      /** container for the peaks of the waveform*/
      peaks_t _peaks;

      /** time of the first peak in the "good" range*/
      mutable double _goodHit;

      /** flag to tell when we are working on a new event*/
      mutable bool _isNewEvent;

      /** the extractor of the produced signals */
      SignalExtractor * _signalextractor;

      /** the signals produces by this producer */
      signals_t _signals;

      /** flag to show whether there is a new event associated whith this signal producer */
      bool _newEventAssociated;
    };
  }//end namespace acqiris
}//end namespace cass


#endif
