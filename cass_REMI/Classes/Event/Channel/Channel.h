#ifndef __Channel_h__
#define __Channel_h__

#include <iostream>
#include <vector>
#include "pdsdata/acqiris/ConfigV1.hh"
#include "pdsdata/acqiris/DataDescriptorV1.hh"

namespace cass
{
namespace REMI
{
class Peak;
typedef std::vector<short> waveform_t;
typedef std::vector<Peak> peaks_t;
class Channel
{
public:
	Channel(int chNbr, const Pds::Acqiris::ConfigV1&, const Pds::Acqiris::DataDescV1&, const cass::REMI::Parameter&);

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
};
}//end namespace REMI
}//end namespace cass
#endif
