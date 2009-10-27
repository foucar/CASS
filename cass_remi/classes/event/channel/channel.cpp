#include "channel.h"
#include "peak.h"
#include "remi_analysis.h"
#include "pdsdata/acqiris/ConfigV1.hh"
#include "pdsdata/acqiris/DataDescV1.hh"

//______________________________________________________________________________________________________________________
cass::REMI::Channel::Channel(int chNbr, const Pds::Acqiris::ConfigV1& config):
    fChNbr(chNbr)
{
    fFullscale  = static_cast<int16_t>(config.vert(fChNbr).fullScale());
    fOffset     = static_cast<int16_t>(config.vert(fChNbr).offset());
    fGain       = config.vert(fChNbr).slope();
    fDataLength = config.horiz().nbrSamples();
    fWaveform.resize(fDataLength);
    printf("channel config Fullscale= %i fGain= %f\n",fFullscale,fGain);
 }

//______________________________________________________________________________________________________________________
void cass::REMI::Channel::init(const Pds::Acqiris::DataDescV1& ddesc)
{
   fIdxToFirstPoint = const_cast<Pds::Acqiris::DataDescV1&>(ddesc).indexFirstPoint();

    //extract waveform//
    const short* waveform = const_cast<Pds::Acqiris::DataDescV1&>(ddesc).waveform();
    waveform += fIdxToFirstPoint;

    //we have to invert the byte order for some reason that still has to be determined//
    for (size_t i=0;i<fDataLength;++i)
      {
        fWaveform[i] = (waveform[i]&0xff<<8) | (waveform[i]&0xff00>>8);
	//       printf("%i ",fWaveform[i]);
      }
}

//______________________________________________________________________________________________________________________
void cass::REMI::Channel::CopyChannelParameters(const cass::REMI::ChannelParameter& param)
{
    fThreshold  = static_cast<int16_t>(param.fThreshold);
    fOffset     = static_cast<int16_t>(param.fOffset);
    fStsi       = param.fStepsize;
    fBs         = param.fBacksize;
    fDelay      = param.fDelay;
    fWalk       = param.fWalk;
    fFraction   = param.fFraction;
}

//______________________________________________________________________________________________________________________
cass::REMI::Peak& cass::REMI::Channel::addPeak()
{
    //add a peak to the container
    fPeaks.push_back(Peak());
    return fPeaks.back();
}
