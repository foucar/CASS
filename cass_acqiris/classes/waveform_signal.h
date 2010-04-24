//Copyright (C) 2010 Lutz Foucar

#ifndef _SIGNAL_H_
#define _SIGNAL_H_

#include <algorithm>

#include <QtCore/QSettings>
#include "cass_acqiris.h"
#include "results_backend.h"
#include "peak.h"

namespace cass
{
  namespace ACQIRIS
  {
    /** Functor returning whether Peak is in Range.
     * This class is beeing used by std::find_if as Predicate
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





    /** Class containing information about how to extract the signals of a waveform.
     * It also contains an array of signals (called peaks for now)
     * @todo rename this class to somehting more meaningful
     *       In the delayline it represents the wireends of the anodelayers and the mcp output
     *       In the tof it should just represent the way to extract the singals and the signals itselve
     * @author Lutz Foucar
     */
    class CASS_ACQIRISSHARED_EXPORT Signal : public ResultsBackend
    {
    public:
      /** default constructor.
       * intializing the variables describing the extraction with nonsense,
       * since they have to be loaded by loadParameters from cass.ini
       */
      Signal()
        :_instrument(Camp1),
         _chNbr(99),
         _trLow(0),
         _trHigh(0),
         _polarity(Bad),
         _threshold(5000),
         _delay(0),
         _fraction(5),
         _walk(200),
         _analyzerType(com8),
         _isNewEvent(true)
      {}

    public:
      /** loads the parameters from cass.ini, should only be called by class containing this class */
      void loadParameters(QSettings *p, const char * signalname);
      /** save your parameters to cass.ini, should only be called by parent*/
      void saveParameters(QSettings *p, const char * signalname);

    public:
      /** convience for easier readable code, is a vector of Peak*/
      typedef std::vector<Peak> peaks_t;

    public:
      /** setter
       * @note we should make sure, that it will not be needed anymore, since the getter should
       *     make sure that all peaks are set using the correct function
       */
      peaks_t           &peaks()              {return _peaks;}
      /** getter
       * @note when calling this we should check whether it has alredy been called for the event
       *     if not so, then create the peaks using the requested waveform analysis
       */
      const peaks_t     &peaks()const         {return _peaks;}

    public:
      /** return the time of the first peak in the "good" time range*/
      double firstGood() const;

    public: //setters /getters
      size_t             channelNbr()const    {return _chNbr;}
      size_t            &channelNbr()         {return _chNbr;}
      double             trLow()const         {return _trLow;}
      double            &trLow()              {return _trLow;}
      double             trHigh()const        {return _trHigh;}
      double            &trHigh()             {return _trHigh;}
      double             grLow()const         {return _grLow;}
      double            &grLow()              {return _grLow;}
      double             grHigh()const        {return _grHigh;}
      double            &grHigh()             {return _grHigh;}
      Polarity           polarity()const      {return _polarity;}
      Polarity          &polarity()           {return _polarity;}
      int16_t            threshold()const     {return _threshold;}
      int16_t           &threshold()          {return _threshold;}
      int32_t            delay()const         {return _delay;}
      int32_t           &delay()              {return _delay;}
      double             fraction()const      {return _fraction;}
      double            &fraction()           {return _fraction;}
      double             walk()const          {return _walk;}
      double            &walk()               {return _walk;}
      Instruments        instrument()const    {return _instrument;}
      Instruments       &instrument()         {return _instrument;}
      WaveformAnalyzers  analyzerType()const  {return _analyzerType;}
      WaveformAnalyzers &analyzerType()       {return _analyzerType;}
      //bool               isNewEvent()const    {return _isNewEvent;}
      //bool              &isNewEvent()         {return _isNewEvent;}

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
      int16_t _threshold;
      /** the delay of the cfd*/
      int32_t _delay;
      /** the fraction of the cfd*/
      double _fraction;
      /** the walk of the cfd*/
      double _walk;
      /** type of analysis to analyze this channel*/
      WaveformAnalyzers _analyzerType;

      //container for the results of the waveform analysis
      /** container for the peaks of the waveform*/
      peaks_t _peaks;
      /** time of the first peak in the "good" range*/
      mutable double _goodHit;
      /** flag to tell when we are working on a new event*/
      mutable bool _isNewEvent;
    };
  }//end namespace acqiris
}//end namespace cass


inline void cass::ACQIRIS::Signal::loadParameters(QSettings *p, const char * signalname)
{
  std::cerr<<"Signal load parameters:  load signal parameters for signal \""<<signalname<<"\""
      <<" of  "<< p->group().toStdString()<<std::endl;
  p->beginGroup(signalname);
  _instrument   = static_cast<Instruments>(p->value("AcqirisInstrument",Camp1).toInt());
  std::cerr <<"Signal load parameters: Instrument "<<_instrument<<std::endl;
  _chNbr        = p->value("ChannelNumber",0).toInt();
  std::cerr <<"Signal load parameters: chNbr "<<_chNbr<<std::endl;
  _trLow        = p->value("LowerTimeRangeLimit",0.).toDouble();
  _trHigh       = p->value("UpperTimeRangeLimit",20000.).toDouble();
  _grLow        = p->value("LowerGoodTimeRangeLimit",0.).toDouble();
  _grHigh       = p->value("UpperGoodTimeRangeLimit",20000.).toDouble();
  _polarity     = static_cast<Polarity>(p->value("Polarity",Negative).toInt());
  _threshold    = p->value("Threshold",50.).toInt();
  _delay        = p->value("Delay",5).toInt();
  _fraction     = p->value("Fraction",0.6).toDouble();
  _walk         = p->value("Walk",0.).toDouble();
  _analyzerType = static_cast<WaveformAnalyzers>(p->value("WaveformAnalysisMethod",com16).toInt());
//  std::cerr <<"Signal load parameters: ana type "<<_analyzerType<<" should be "<<com16<<std::endl;
  std::cerr<<"Signal load parameters: done loading"<<std::endl;
  p->endGroup();
}
inline void cass::ACQIRIS::Signal::saveParameters(QSettings *p, const char * signalname)
{
  p->beginGroup(signalname);
  p->setValue("AcqirisInstrument",static_cast<int>(_instrument));
  p->setValue("ChannelNumber",static_cast<int>(_chNbr));
  p->setValue("LowerTimeRangeLimit",_trLow);
  p->setValue("UpperTimeRangeLimit",_trHigh);
  p->setValue("LowerGoodTimeRangeLimit",_grLow);
  p->setValue("UpperGoodTimeRangeLimit",_grHigh);
  p->setValue("Polarity",static_cast<int>(_polarity));
  p->setValue("Threshold",_threshold);
  p->setValue("Delay",_delay);
  p->setValue("Fraction",_fraction);
  p->setValue("Walk",_walk);
  p->setValue("WaveformAnalysisMethod",static_cast<int>(_analyzerType));
  p->endGroup();
}

inline double cass::ACQIRIS::Signal::firstGood() const
{
  //if this is called for the new event for the first time, then evaluate//
  if(_isNewEvent)
  {
    //find first occurence of peak that is in the given timerange//
    peaks_t::const_iterator it =
        std::find_if(_peaks.begin(),_peaks.end(),
                     PeakInRange(_grLow,_grHigh));
                     //std::logical_and<bool>,bind2nd(less_equal<???>(),_grHigh);
    /*std::vector<int>::iterator int_it=std::find_if(
      ints.begin(),
      ints.end(),
      boost::bind(std::logical_and<bool>(),
        boost::bind(std::greater<int>(),_1,5),
        boost::bind(std::less_equal<int>(),_1,10)));*/

    //if it is not there retrun 0, otherwise the time of the found peak//
    _goodHit = (it==_peaks.end())? 0. : it->time();
    _isNewEvent = false;
  }
  return _goodHit;
}

#endif
