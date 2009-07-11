#include "Channel.h"
#include "Peak.h"
#include "REMIAnalysis.h"

//______________________________________________________________________________________________________________________
cass::REMI::Channel::Channel(int chNbr, Pds::Acqiris::ConfigV1& config, Pds::Acqiris::DataDescV1& ddesc):
	fChNbr(chNbr)
{
	fFullscale		 = config.vert(fChNbr).fullScale();
	fOffset			 = config.vert(fChNbr).offset();
	fGain			 = config.vert(fChNbr).slope();
	//fIdxToFirstPoint = config.horiz().indexFirstPoint();
	fDataLength		 = config.horiz().nbrSamples();
	
	//extract waveform//
	const short* waveform = ddesc.waveform(config.horiz());
	waveform += fIdxToFirstPoint;

	//we have to invert the byte order for some reason that still has to be determined//
	for (size_t i=0;i<fDataLength;++i)
		fWaveform[i] = (waveform[i]&0xff<<8) | (waveform[i]&0xff00>>8);
}

//______________________________________________________________________________________________________________________
void cass::REMI::Channel::CopyChannelParameters(const cass::REMI::ChannelParameter& param)
{
    fThreshold		 = param.fThreshold;
    fStsi			 = param.fStepsize;
    fBs				 = param.fBacksize;
    fDelay			 = param.fDelay;
    fWalk			 = param.fWalk;
    fFraction		 = param.fFraction;
}

//______________________________________________________________________________________________________________________
cass::REMI::Peak& cass::REMI::Channel::addPeak()
{
	//add a peak to the container
	fPeaks.push_back(Peak());
	return fPeaks.back();
}
