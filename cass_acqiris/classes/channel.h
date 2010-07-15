//Copyright (C) 2009,2010 Lutz Foucar

/**
 * @file channel.h file contains the classes that describe an acqiris channel
 *
 * @author Lutz Foucar
 */

#ifndef _CHANNEL_H_
#define _CHANNEL_H_


#include <stdint.h>
#include <iostream>
#include <vector>

#include "cass_acqiris.h"
#include "serializer.h"
#include "serializable.h"

namespace cass
{
  namespace ACQIRIS
  {
    /** A Channel of an Acqiris Instrument.
     *
     * contains all information that an Acqiris instrument will provide about
     * a channel
     *
     * @author Lutz Foucar
     */
    class CASS_ACQIRISSHARED_EXPORT Channel : public Serializable
    {
    public:
      /** constructor that will set the serialize version*/
      Channel():Serializable(1)  {}

    public:
      /** will serialize this channel to the serializer*/
      void serialize(cass::SerializerBackend&)const;

      /** deserialize this channel from the serializer*/
      bool deserialize(cass::SerializerBackend&);

    public:
      //@{
      /** setter */
      double            &horpos()               {return _horpos;}
      double            &offset()               {return _offset;}
      double            &sampleInterval()       {return _sampleInterval;}
      double            &gain()                 {return _gain;}
      waveform_t        &waveform()             {return _waveform;}
      size_t            &channelNbr()           {return _chNbr;}
      //@}
      //@{
      /** getter */
      double             horpos()const          {return _horpos;}
      double             offset()const          {return _offset;}
      double             gain()const            {return _gain;}
      double             sampleInterval()const  {return _sampleInterval;}
      const waveform_t  &waveform()const        {return _waveform;}
      size_t             channelNbr()const      {return _chNbr;}
      //@}

      /** getter
       *
       * will calculate the fullscale from the gain value, provided by the
       * instrument
       */
      double             fullscale()const       {return _gain*0xffff;}

    private:
      /** This Channels Number in the Acqiris Instrument*/
      size_t _chNbr;

      /** Horizontal position of first data point with respect to the trigger*/
      double _horpos;

      /** the vertical offset of this channel (in V)*/
      double _offset;

      /** Vertical gain in Volts/LSB. (V = vGain * data - vOffset)*/
      double _gain;

      /** the time between two consecutive datapoints in seconds*/
      double _sampleInterval;

      /** the waveform of this channel*/
      waveform_t  _waveform;
    };

  }//end namespace remi
}//end namespace cass

inline void cass::ACQIRIS::Channel::serialize(cass::SerializerBackend &out)const
{
  //the version//
  out.addSizet(_version);
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

inline bool cass::ACQIRIS::Channel::deserialize(cass::SerializerBackend &in)
{
  //check whether the version fits//
  uint16_t ver = in.retrieveSizet();
  if(ver!=_version)
  {
    std::cerr<<"version conflict in acqiris channel: "<<ver<<" "<<_version<<std::endl;
    return false;
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
  return true;
}

#endif
