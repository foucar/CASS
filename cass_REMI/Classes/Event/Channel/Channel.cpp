#include "Channel.h"
#include "Peak.h"

//______________________________________________________________________________________________________________________
cass::REMI::Channel::Channel(int chNbr, const Pds::Acqiris::ConfigV1& config, const Pds::Acqiris::DataDescV1& ddesc, const cass::REMI::Parameter&):
	fChNbr(chNbr)
{
	fFullscale		 = config.vert(fChNbr).fullScale();
	fOffset			 = config.vert(fChNbr).offset();
	fGain			 = config.vert(fChNbr).slope();
	fIdxToFirstPoint = config.horiz().indexFirstPoint();
	fDataLength		 = config.horiz().nbrSamples();
	fThreshold		 = param.threshold();
	fStsi			 = param.stepsize();
	fBs				 = param.backsize();
	
	//extract waveform//
	const short* waveform = ddesc.waveform(config.horiz());
	waveform += fIdxToFirstPoint;

	//we have to invert the byte order for some reason that still has to be determined//
	for (size_t i=0;i<fDataLength;++i)
		fWaveform[i] = (waveform[i]&0xff<<8) | (waveform[i]&0xff00>>8);
}

//______________________________________________________________________________________________________________________
cass::REMI::Peak& cass::REMI::Channel::addPeak()
{
	//add a peak to the container
	fPeaks.push_back(Peak());
	return fPeaks.back();
}