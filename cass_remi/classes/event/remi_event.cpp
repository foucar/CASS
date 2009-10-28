#include <iostream>
#include "remi_event.h"
#include "remi_analysis.h"
#include "pdsdata/acqiris/ConfigV1.hh"
#include "pdsdata/acqiris/DataDescV1.hh"
#include "channel.h"

//______________________________________________________________________________________________________________________
void cass::REMI::REMIEvent::init(const Pds::Acqiris::ConfigV1& config)
{
    fIsInitialized      = true;
    fIsFilled           = false;
    fNbrBytes           = sizeof(int16_t);
    fSampleInterval     = config.horiz().sampInterval();
    fNbrSamples         = config.horiz().nbrSamples();
    fDelayTime          = config.horiz().delayTime();
    fTrigChannel        = config.trig().trigInput();
    fTrigLevel          = config.trig().trigLevel();
    fTrigSlope          = config.trig().trigSlope();
    fChanCombUsedChans  = config.channelMask();
    fNbrConPerCh        = config.nbrConvertersPerChannel();

    //output data//
    std::cout <<"config:"<<std::endl;
    std::cout <<" SampleInterval: "<<config.horiz().sampInterval()<<std::endl;
    std::cout <<" NbrSamples: "<<config.horiz().nbrSamples()<<std::endl;
    std::cout <<" DelayTime: "<<fDelayTime<<std::endl;
    std::cout <<" TrigChannel: "<<fTrigChannel<<std::endl;
    std::cout <<" TrigLevel: "<<fTrigLevel<<std::endl;
    std::cout <<" TrigSlope: "<<fTrigSlope<<std::endl;
    std::cout <<" ChanCombUsedChans: "<<fChanCombUsedChans<<std::endl;
    std::cout <<" NbrConPerCh: "<<fNbrConPerCh<<std::endl;
    std::cout <<" SizeOfHoriz: "<<sizeof(Pds::Acqiris::HorizV1)<<std::endl;
    std::cout <<" SizeOfTrig: " <<sizeof(Pds::Acqiris::TrigV1)<<std::endl;

    //create the channels//
    for (size_t i=0;i<config.nbrChannels();++i)
        fChannels.push_back(cass::REMI::Channel(i,config));
}

void cass::REMI::REMIEvent::init(const Pds::Acqiris::DataDescV1& ddesc)
{
    //only use this function if event is initialized//
    if (fIsInitialized)
    {
        fIsFilled   = true;
        Pds::Acqiris::DataDescV1 * dd = const_cast<Pds::Acqiris::DataDescV1*>(&ddesc);
        fHorpos     = dd->timestamp(0).horPos();                //horpos from acqiris
//        std::cout <<"datadesc:"<<std::endl;
//        std::cout <<"  horpos: "<<fHorpos<<std::endl;

        for (size_t i=0;i<fChannels.size();++i)
        {
            fChannels[i].init(*dd);
            dd = dd->nextChannel();
        }
    }
}

void cass::REMI::REMIEvent::CopyParameters(const cass::REMI::Parameter& param)
{
    //add the parameters to the channels//
    for (size_t i=0;i<fChannels.size();++i)
        fChannels[i].CopyChannelParameters(param.fChannelParameters[i]);

    //create the detectors
    for (size_t i=0;i<param.fDetectorParameters.size();++i)
        fDets.push_back(Detector(param.fDetectorParameters[i]));
}
