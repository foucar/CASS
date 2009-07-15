#include <iostream>
#include "REMIEvent.h"
#include "REMIAnalysis.h"
#include "Channel.h"

//______________________________________________________________________________________________________________________
void cass::REMI::REMIEvent::CreateEventFromLCLSData(Pds::Acqiris::ConfigV1& config, Pds::Acqiris::DataDescV1& ddesc)
{
    fIsFilled             = true;
    fHorpos				= 0;//ddesc.timestamp(0).horpos();				//horpos from acqiris
	fNbrBytes			= 2;
    fSampleInterval		= config.horiz().sampInterval();
	fNbrSamples			= config.horiz().nbrSamples();
	fDelayTime			= config.horiz().delayTime();
    fTrigChannel		= config.trig().trigInput();
    fTrigLevel  		= config.trig().trigLevel();
    fTrigSlope          = config.trig().trigSlope();
	fChanCombUsedChans	= config.channelMask();
	fNbrConPerCh		= config.nbrConvertersPerChannel();

	Pds::Acqiris::DataDescV1 * dd = &ddesc;

	//create the channels//
	for (size_t i=0;i<config.nbrChannels();++i)
	{
        fChannels.push_back(cass::REMI::Channel(i,config,*dd));
		dd = dd->nextChannel(config.horiz());
	}
}

void cass::REMI::REMIEvent::CopyParameters(const cass::REMI::Parameter& param)
{
    //add the parameters to the channels//
    for (size_t i=0;i<fChannels.size();++i)
        fChannels[i].CopyChannelParameters(param.fChannelParameters[i]);

    //create the detectors
    for (size_t i=0;i<param.nbrOfDetectors();++i)
        fDets.push_back(Detector(param.fDetectorParameters[i]));
}
