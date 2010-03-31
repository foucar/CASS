//Copyright (C) 2010 lmf

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
    //helperclass for evaluating the first good hit in a given range//
    class PeakInRange
    {
    public:
      PeakInRange(double from, double to)
        :_from(from),
        _to(to)
      {}

      //return whether the time of the peak is in the requested range
      bool operator()(const Peak &peak)const
      {
        return (_from  < peak.time() && peak.time() < _to);
      }

    private:
      double _from;
      double _to;
    };

    //----------------------------------------------------------------------------------
    class CASS_ACQIRISSHARED_EXPORT Signal : public ResultsBackend    //the properties of one Wire-end of the Layerwire
    {
    public:
      Signal()
        :_chNbr(99),
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
      ~Signal()   {}

    public:
      void loadParameters(QSettings *p, const char * signalname)
      {
        //std::cerr<<"loading wavefrom signal parameters for signal \""<<signalname<<"\""<<std::endl;
        p->beginGroup(signalname);
        _chNbr        = p->value("ChannelNumber",0).toInt();
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
        //std::cerr <<"anty "<<_analyzerType<<" should be "<<com16<<std::endl;
        //std::cerr<<"done"<<std::endl;
        p->endGroup();
      }
      void saveParameters(QSettings *p, const char * signalname)
      {
        p->beginGroup(signalname);
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

    public:
      typedef std::vector<Peak> peaks_t;

    public:
      peaks_t           &peaks()              {return _peaks;}
      const peaks_t     &peaks()const         {return _peaks;}

    public:
      double firstGood() const
      {
        //if this is called for the new event for the first time, then evaluate//
        if(_isNewEvent)
        {
          //find first occurence of peak that is in the given timerange//
          peaks_t::const_iterator it = std::find_if(_peaks.begin(),_peaks.end(),PeakInRange(_grLow,_grHigh));
          //if it is not there retrun 0, otherwise the time of the found peak//
          _goodHit = (it==_peaks.end())? 0. : it->time();
          _isNewEvent = false;
        }
        return _goodHit;
      }

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
      WaveformAnalyzers  analyzerType()const  {return _analyzerType;}
      WaveformAnalyzers &analyzerType()       {return _analyzerType;}
      //bool               isNewEvent()const    {return _isNewEvent;}
      //bool              &isNewEvent()         {return _isNewEvent;}

    private:
      //things important to know how to analyze the waveform//
      //set by the user via parameters//
      size_t            _chNbr;         //This Channels Number in the Acqiris Crate
      double            _trLow;         //lower edge of the timerange events can happen in
      double            _trHigh;        //upper edge of the timerange events can happen in
      double            _grLow;         //lower edge of the timerange good single events can happen in
      double            _grHigh;        //upper edge of the timerange good single  events can happen in
      Polarity          _polarity;      //the Polarity the Signal has
      int16_t           _threshold;     //the Noiselevel for this channel (in adc bytes)
      int32_t           _delay;         //the delay of the cfd
      double            _fraction;      //the fraction of the cfd
      double            _walk;          //the walk of the cfd
      WaveformAnalyzers _analyzerType;  //type of analysis to analyze this channel
      //container for the results of the waveform analysis
      peaks_t           _peaks;         //container for the peaks of the waveform
      mutable double    _goodHit;       //time of the first peak in the "good" range
      mutable bool      _isNewEvent;    //flag to tell when we are working on a new event
    };
  }//end namespace acqiris
}//end namespace cass

#endif
