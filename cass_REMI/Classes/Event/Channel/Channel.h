#ifndef __Channel_h__
#define __Channel_h__

#include <iostream>
#include <vector>
#include "pdsdata/acqiris/ConfigV1.hh"
#include "pdsdata/acqiris/DataDescV1.hh"
#include "Peak.h"

namespace cass
{
    namespace REMI
    {
        typedef std::vector<short> waveform_t;
        class ChannelParameter;
        class Channel
        {
        public:
            Channel(int chNbr, Pds::Acqiris::ConfigV1&, Pds::Acqiris::DataDescV1&, const cass::REMI::ChannelParameter&);

        public:
            Peak		&addPeak();
            size_t		 nbrPeaks()const			{return fPeaks.size();}
            Peak		&peak(long idx)				{return fPeaks[idx];}
            const Peak	&peak(long idx)const		{return fPeaks[idx];}

        public:
            int			 channelNbr()const			{return fChNbr;}
            short		 threshold()const			{return fThreshold;}
            short		 fullscale()const 			{return fFullscale;}
            short		 vertOffset()const			{return fOffset;}
            double		 vertGain()const 			{return fGain;}
            long		 stepsize()const			{return fStsi;}
            long		 backsize()const			{return fBs;}
            const void	*waveform()const			{return &fWaveform[fIdxToFirstPoint];}
            size_t		 waveformLength()const		{return fDataLength;}
            size_t		 idxToFirstPoint()const		{return fIdxToFirstPoint;}
            int			 delay()const				{return fDelay;}
            double		 fraction()const			{return fFraction;}
            double		 walk()const				{return fWalk;}


        private:
            int			 fChNbr;					//This Channels Number
            peaks_t		 fPeaks;					//Container storing the found peaks
            waveform_t	 fWaveform;					//the waveform
            size_t		 fDataLength;				//the length of the waveform
            size_t		 fIdxToFirstPoint;			//the index to the first point in the waveform

            short		 fFullscale;				//the fullscale for this channel (in mV)
            short		 fOffset;					//the offset for this channel (in mV)
            double		 fGain;						//the conversion factor from adc bytes to mV (adc bytes * fGain = mV)
            short		 fThreshold;				//the Noiselevel for this channel (in adc bytes)
            long		 fStsi;						//the stepsize for this channel
            long		 fBs;						//the backsize for this channel
            int			 fDelay;					//the delay of the cfd
            double		 fFraction;					//the fraction of the cfd
            double		 fWalk;						//the walk of the cfd
        };
        typedef std::vector<Channel> channels_t;
    }//end namespace remi
}//end namespace cass
#endif
