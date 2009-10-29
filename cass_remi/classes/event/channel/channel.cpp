#include "channel.h"
#include "peak.h"
#include "remi_analysis.h"
#include "pdsdata/acqiris/ConfigV1.hh"
#include "pdsdata/acqiris/DataDescV1.hh"

//______________________________________________________________________________________________________________________
cass::REMI::Channel::Channel(int chNbr, const Pds::Acqiris::ConfigV1& config):
    fChNbr(chNbr)
{
    fFullscale  = static_cast<int16_t>(config.vert(fChNbr).fullScale()*1000.);
    fOffset     = static_cast<int16_t>(config.vert(fChNbr).offset()*1000.);
    fGain       = config.vert(fChNbr).slope()*1000;
    fDataLength = config.horiz().nbrSamples();
    fWaveform.resize(fDataLength);

    //output//
    std::cout <<" channel: "<<fChNbr<<std::endl;
    std::cout <<"  Fullscale: "<<config.vert(fChNbr).fullScale()*1000<<std::endl;
    std::cout <<"  Gain: "<<config.vert(fChNbr).slope()*1000<<std::endl;
    std::cout <<"  Offset: " <<config.vert(fChNbr).offset()*1000.<<std::endl;
    std::cout <<"  WaveformLength: "<<config.horiz().nbrSamples()<<std::endl;
    std::cout <<"  SizeofVert: " <<sizeof(Pds::Acqiris::VertV1)<<std::endl;
}

//______________________________________________________________________________________________________________________
void cass::REMI::Channel::init(const Pds::Acqiris::DataDescV1& ddesc)
{
    fIdxToFirstPoint = const_cast<Pds::Acqiris::DataDescV1&>(ddesc).indexFirstPoint();
//    std::cout <<" channel "<<fChNbr<<":"<<std::endl;
//    std::cout <<"   idx to 1 point: " <<fIdxToFirstPoint<<std::endl;

    //extract waveform//
    const short* waveform = const_cast<Pds::Acqiris::DataDescV1&>(ddesc).waveform();
    waveform += fIdxToFirstPoint;

    //we have to invert the byte order for some reason that still has to be determined//
    for (size_t i=0;i<fDataLength;++i)
        fWaveform[i] = (waveform[i]&0xff<<8) | (waveform[i]&0xff00>>8);
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
