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
      Channel():_version(1)  {}
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
        uint16_t    _version;       //the version for de/serialization
    };
  }//end namespace remi
}//end namespace cass

inline void cass::ACQIRIS::Channel::serialize(cass::Serializer &out) const
{
  //the version//
  out.addUint16(_version);
  //output all variables obtained from the acqiris, then the waveform//
  out.addDouble(_horpos);
  out.addDouble(_offset);
  out.addDouble(_gain);
  out.addDouble(_sampleInterval);
  //the length of the waveform//
  const size_t nSamples = _waveform.size();
  out.addSizet(nSamples);
  //the waveform itselve//
  for (waveform_t::const_iterator it=_waveform.begin();it!=_waveform.end();++it)
    out.addInt16(*it);
}

inline void cass::ACQIRIS::Channel::deserialize(cass::Serializer &in)
{
  //check whether the version fits//
  uint16_t ver = in.retrieveUint16();
  if(ver!=_version)
  {
    std::cerr<<"version conflict in acqiris channel: "<<ver<<" "<<_version<<std::endl;
    return;
  }
  //read all variables and then the waveform from the input stream
  _horpos = in.retrieveDouble();
  _offset = in.retrieveDouble();
  _gain = in.retrieveDouble();
  _sampleInterval = in.retrieveDouble();
  //read the length of the waveform//
  size_t nSamples = in.retrieveSizet();
  //make sure the waveform is long enough//
  _waveform.resize(nSamples);
  for (waveform_t::iterator it=_waveform.begin();it!=_waveform.end();++it)
    *it = in.retrieveInt16();
}

#endif
