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

    //create the channels//
    printf("in config %i\n",config.nbrChannels());
    //for (size_t i=0;i<1;++i)
    for (size_t i=0;i<config.nbrChannels();++i)
        fChannels.push_back(cass::REMI::Channel(i,config));
}

void cass::REMI::REMIEvent::init(const Pds::Acqiris::DataDescV1& ddesc)
{
    fIsFilled   = true;
    fHorpos     = 0;//ddesc.timestamp(0).horpos();                //horpos from acqiris
    Pds::Acqiris::DataDescV1 * dd = const_cast<Pds::Acqiris::DataDescV1*>(&ddesc);
    //printf("in DataDesc %i\n",fChannels.size());

    for (size_t i=0;i<fChannels.size();++i)
    {
      //printf("Channel %i\n",i);
        fChannels[i].init(*dd);
	//	printf("almost done %i\n",i);
        dd = dd->nextChannel();
	//printf("done %i\n",i);
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
