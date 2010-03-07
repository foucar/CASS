#ifndef _CHANNEL_H_
#define _CHANNEL_H_


#include <stdint.h>
#include <iostream>
#include <vector>
#include "cass_acqiris.h"
#include "serializer.h"

namespace cass
{
  namespace ACQIRIS
  {
    class CASS_ACQIRISSHARED_EXPORT Channel
    {
      public:
        Channel()  {}
        ~Channel() {}

      public:
        typedef std::vector<int16_t> waveform_t;

      public:
        void serialize(cass::Serializer&)const;
        void deserialize(cass::Serializer&);

      public:
        double             horpos()const          {return _horpos;}
        double            &horpos()               {return _horpos;}
        double             offset()const          {return _offset;}
        double            &offset()               {return _offset;}
        double             gain()const            {return _gain;}
        double            &sampleInterval()       {return _sampleInterval;}
        double             sampleInterval()const  {return _sampleInterval;}
        double            &gain()                 {return _gain;}
        const waveform_t  &waveform()const        {return _waveform;}
        waveform_t        &waveform()             {return _waveform;}

      public:
        size_t             channelNbr()const      {return _chNbr;}
        size_t            &channelNbr()           {return _chNbr;}
        double             fullscale()const       {return _gain*0xffff;}

      private:
        size_t      _chNbr;         //This Channels Number in the Acqiris Crate
        //values extracted from the acqiris//
        double      _horpos;        //Horizontal position of first data point with respect to the trigger
        double      _offset;        //the offset for this channel (in V)
        double      _gain;          //Vertical gain in Volts/LSB. (V = vGain * data - vOffset)
        double      _sampleInterval;//the time between two consecutive datapoints in s
        waveform_t  _waveform;      //the waveform
    };
  }//end namespace remi
}//end namespace cass

inline void cass::ACQIRIS::Channel::serialize(cass::Serializer &out) const
{
//  //output all variables obtained from the acqiris, then the waveform//
//  std::copy( reinterpret_cast<const char*>(&_chNbr),
//             reinterpret_cast<const char*>(&_chNbr)+sizeof(size_t),
//             out);
//  std::copy( reinterpret_cast<const char*>(&_horpos),
//             reinterpret_cast<const char*>(&_horpos)+sizeof(double),
//             out);
//  std::copy( reinterpret_cast<const char*>(&_offset),
//             reinterpret_cast<const char*>(&_offset)+sizeof(double),
//             out);
//  std::copy( reinterpret_cast<const char*>(&_gain),
//             reinterpret_cast<const char*>(&_gain)+sizeof(double),
//             out);
//  std::copy( reinterpret_cast<const char*>(&_sampleInterval),
//             reinterpret_cast<const char*>(&_sampleInterval)+sizeof(double),
//             out);
//
//  size_t nSamples = _waveform.size();
//  std::copy( reinterpret_cast<const char*>(&nSamples),
//             reinterpret_cast<const char*>(&nSamples)+sizeof(size_t),
//             out);
//  std::copy( reinterpret_cast<const char*>(&_waveform[0]),
//             reinterpret_cast<const char*>(&_waveform[0])+sizeof(int16_t)*_waveform.size(),
//             out);
}

inline void cass::ACQIRIS::Channel::deserialize(cass::Serializer &in)
{
//  //read all variables and then the waveform from the input stream
//  std::copy(in,
//            in+sizeof(size_t),
//            reinterpret_cast<char*>(&_chNbr));
//  in += sizeof(size_t);
//  std::copy(in,
//            in+sizeof(double),
//            reinterpret_cast<char*>(&_horpos));
//  in += sizeof(double);
//  std::copy(in,
//            in+sizeof(double),
//            reinterpret_cast<char*>(&_offset));
//  in += sizeof(double);
//  std::copy(in,
//            in+sizeof(double),
//            reinterpret_cast<char*>(&_gain));
//  in += sizeof(double);
//  std::copy(in,
//            in+sizeof(double),
//            reinterpret_cast<char*>(&_sampleInterval));
//  in += sizeof(double);
//
//  size_t nSamples;
//  std::copy(in,
//            in+sizeof(size_t),
//            reinterpret_cast<char*>(&nSamples));
//  in += sizeof(size_t);
//  _waveform.resize(nSamples);
//  std::copy(in,
//            in+sizeof(int16_t)*_waveform.size(),
//            reinterpret_cast<char*>(&_waveform[0]));
//  in += sizeof(int16_t)*_wavform.size();
}

#endif
