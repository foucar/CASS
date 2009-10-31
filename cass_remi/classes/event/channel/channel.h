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
        typedef std::vector<short> waveform_t;
        class ChannelParameter;
        class Channel
        {
        public:
            Channel(int chNbr, const Pds::Acqiris::ConfigV1&);
            void init(const Pds::Acqiris::DataDescV1&);
            void CopyChannelParameters(const cass::REMI::ChannelParameter&);
            Channel() {};
            ~Channel() {};

        public:
            Peak       &addPeak();
            size_t      nbrPeaks()const         {return fPeaks.size();}
            Peak       &peak(long idx)          {return fPeaks[idx];}
            const Peak &peak(long idx)const     {return fPeaks[idx];}

        public:
            int         channelNbr()const       {return fChNbr;}
            int16_t     threshold()const        {return fThreshold;}
            int16_t     fullscale()const        {return fFullscale;}
            int16_t     vertOffset()const       {return fOffset;}
            double      vertGain()const         {return fGain;}
            int32_t     stepsize()const         {return fStsi;}
            int32_t     backsize()const         {return fBs;}
            const void *waveform()const         {return &fWaveform[0];}
            size_t      waveformLength()const   {return fDataLength;}
            size_t      idxToFirstPoint()const  {return fIdxToFirstPoint;}
            int32_t     delay()const            {return fDelay;}
            double      fraction()const         {return fFraction;}
            double      walk()const             {return fWalk;}


        private:
            uint32_t    fChNbr;                  //This Channels Number
            peaks_t     fPeaks;                  //Container storing the found peaks
            waveform_t  fWaveform;               //the waveform
            size_t      fDataLength;             //the length of the waveform
            size_t      fIdxToFirstPoint;        //the index to the first point in the waveform

            int16_t     fFullscale;              //the fullscale for this channel (in mV)
            int16_t     fOffset;                 //the offset for this channel (in mV)
            double      fGain;                   //the conversion factor from adc bytes to mV (adc bytes * fGain = mV)
            int16_t     fThreshold;              //the Noiselevel for this channel (in adc bytes)
            int32_t     fStsi;                   //the stepsize for this channel
            int32_t     fBs;                     //the backsize for this channel
            int32_t     fDelay;                  //the delay of the cfd
            double      fFraction;               //the fraction of the cfd
            double      fWalk;                   //the walk of the cfd

            ClassDefNV(Channel,1)
        };
        typedef std::vector<Channel> channels_t;
    }//end namespace remi
}//end namespace cass
#endif
