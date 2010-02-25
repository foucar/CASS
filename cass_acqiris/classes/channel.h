#ifndef _CHANNEL_H_
#define _CHANNEL_H_


#include <stdint.h>
#include <iostream>
#include <vector>

namespace cass
{
  namespace ACQIRIS
  {
    class Channel
    {
      public:
        Channel()  {}
        ~Channel() {}

      public:
        typedef std::vector<int16_t> waveform_t;

      public:
        double             horpos()const          {return _horpos;}
        double            &horpos()               {return _horpos;}
        int16_t            offset()const          {return _offset;}
        int16_t           &offset()               {return _offset;}
        double             gain()const            {return _gain;}
        double            &sampleInterval()       {return _sampleInterval;}
        double             sampleInterval()const  {return _sampleInterval;}
        double            &gain()                 {return _gain;}
        const waveform_t  &waveform()const        {return _waveform;}
        waveform_t        &waveform()             {return _waveform;}

      public:
        size_t             channelNbr()const  {return _chNbr;}
        size_t            &channelNbr()       {return _chNbr;}
        double             fullscale()const   {return _gain*0xffff;}

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
#endif
