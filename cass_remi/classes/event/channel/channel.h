#ifndef __Channel_h__
#define __Channel_h__

#ifndef ROOT_Rtypes
#include <Rtypes.h>
#endif

#ifndef ROOT_TObject
#include <TObject.h>
#endif

#include <stdint.h>
#include <iostream>
#include <vector>
#include "peak.h"

namespace Pds
{
    namespace Acqiris
    {
        class ConfigV1;
        class DataDescV1;
    }
}

namespace cass
{
    namespace REMI
    {
        class ChannelParameter;
        class Channel
        {
        public:
            Channel():_type(0)  {}
            ~Channel() {}

        public:
            typedef std::vector<int16_t> waveform_t;
            typedef std::vector<Peak> peaks_t;

        public:
            peaks_t             &peaks()            {return _peaks;}
            const peaks_t       &peaks()const       {return _peaks;}

        public:
            double               horpos()const      {return _horpos;}
            double              &horpos()           {return _horpos;}
            int16_t              fullscale()const   {return _fullscale;}
            int16_t             &fullscale()        {return _fullscale;}
            int16_t              offset()const      {return _offset;}
            int16_t             &offset()           {return _offset;}
            double               gain()const        {return _gain;}
            double              &gain()             {return _gain;}
            const waveform_t    &waveform()const    {return _waveform;}
            waveform_t          &waveform()         {return _waveform;}

        public:
            size_t               channelNbr()const  {return _chNbr;}
            size_t              &channelNbr()       {return _chNbr;}
            int16_t              threshold()const   {return _threshold;}
            int16_t             &threshold()        {return _threshold;}
            int32_t              delay()const       {return _delay;}
            int32_t             &delay()            {return _delay;}
            double               fraction()const    {return _fraction;}
            double              &fraction()         {return _fraction;}
            double               walk()const        {return _walk;}
            double              &walk()             {return _walk;}
            int32_t              type()const        {return _type;}
            int32_t             &type()             {return _type;}

        private:
            //values extracted from the acqiris//
            waveform_t  _waveform;               //the waveform
            double      _horpos;                 //Horizontal position of first data point with respect to the trigger
            int16_t     _fullscale;              //the fullscale for this channel (in mV)
            int16_t     _offset;                 //the offset for this channel (in mV)
            double      _gain;                   //Vertical gain in Volts/LSB. (V = vGain * data - vOffset) the conversion factor from adc bytes to mV (adc bytes * fGain = mV)

            //things important to know to analyze the waveform//
            //set by the user via parameters//
            size_t      _chNbr;                  //This Channels Number in the Acqiris Crate
            int16_t     _threshold;              //the Noiselevel for this channel (in adc bytes)
            int32_t     _delay;                  //the delay of the cfd
            double      _fraction;               //the fraction of the cfd
            double      _walk;                   //the walk of the cfd
            int32_t     _type;                   //type of analysis to analyze this channel

            //there are the results of the analysis of the waveform//
            peaks_t     _peaks;                  //Container storing the found peaks

            ClassDefNV(Channel,1)                //A channel of the Acqiris instrument
        };
    }//end namespace remi
}//end namespace cass
#endif
