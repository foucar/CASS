//Copyright (C) 2010 lmf

#ifndef _SIGNAL_H_
#define _SIGNAL_H_

#include <QtCore/QSettings>
#include "cass_acqiris.h"
#include "results_backend.h"
#include "peak.h"

namespace cass
{
  namespace ACQIRIS
  {
    //----------------------------------------------------------------------------------
    class CASS_ACQIRISSHARED_EXPORT Signal : public ResultsBackend    //the properties of one Wire-end of the Layerwire
    {
    public:
      Signal()    {}
      ~Signal()   {}

    public:
      loadParameters(QSettings *p, const char * signalname)
      {
        p->beginGroup(signalname);
        _chNbr        = p->value("ChannelNumber",0).toInt();
        _trLow        = p->value("LowerTimeRangeLimit",0.).toDouble();
        _trHigh       = p->value("UpperTimeRangeLimit",20000.).toDouble();
        _polarity     = static_cast<Polarity>(p->value("Polarity",Negative).toInt());
        _threshold    = p->value("Threshold",50.).toInt();
        _delay        = p->value("Delay",5).toInt();
        _fraction     = p->value("Fraction",0.6).toDouble();
        _walk         = p->value("Walk",0.).toDouble();
        _analyzerType = static_cast<WaveformAnalyzers>(p->value("WaveformAnalysisMethod",CoM16Bit).toInt());
        p->endGroup();
      }
      saveParameters(QSettings *p, const char * signalname)
      {
        p->beginGroup(signalname);
        p->setValue("ChannelNumber",static_cast<int>(_chNbr));
        p->setValue("LowerTimeRangeLimit",_trLow);
        p->setValue("UpperTimeRangeLimit",_trHigh);
        p->setValue("Polarity",static_cast<int>(_polarity()));
        p->setValue("Threshold",_threshold());
        p->setValue("Delay",_delay());
        p->setValue("Fraction",_fraction());
        p->setValue("Walk",_walk());
        p->setValue("WaveformAnalysisMethod",static_cast<int>(_analyzerType));
        p->endGroup();
      }

    public:
      typedef std::vector<Peak> peaks_t;

    public:
      peaks_t           &peaks()             {return _peaks;}
      const peaks_t     &peaks()const        {return _peaks;}

    public:
      size_t             channelNbr()const    {return _chNbr;}
      size_t            &channelNbr()         {return _chNbr;}
      double             trLow()const         {return _trLow;}
      double            &trLow()              {return _trLow;}
      double             trHigh()const        {return _trHigh;}
      double            &trHigh()             {return _trHigh;}
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

    private:
      //things important to know how to analyze the waveform//
      //set by the user via parameters//
      size_t            _chNbr;         //This Channels Number in the Acqiris Crate
      double            _trLow;         //lower edge of the timerange events can happen in
      double            _trHigh;        //upper edge of the timerange events can happen in
      Polarity          _polarity;      //the Polarity the Signal has
      int16_t           _threshold;     //the Noiselevel for this channel (in adc bytes)
      int32_t           _delay;         //the delay of the cfd
      double            _fraction;      //the fraction of the cfd
      double            _walk;          //the walk of the cfd
      WaveformAnalyzers _analyzerType;  //type of analysis to analyze this channel
      //container for the results of the waveform analysis
      peaks_t           _peaks;         //container for the peaks of the waveform
    };
  }//end namespace acqiris
}//end namespace cass
