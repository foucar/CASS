#include <iostream>
#include "REMIEvent.h"
#include "REMIAnalysis.h"
#include "Channel.h"

//______________________________________________________________________________________________________________________
cass::REMI::REMIEvent::REMIEvent(Pds::Acqiris::ConfigV1& config, Pds::Acqiris::DataDescV1& ddesc, const cass::REMI::Parameter &param)
{
	//fHorpos				= ddesc.timestamp(0).horpos();				//horpos from acqiris
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
        fChannels.push_back(cass::REMI::Channel(i,config,*dd,param.fChannelParameters[i]));
		dd = dd->nextChannel(config.horiz());
	}

	//create the detectors
	for (size_t i=0;i<param.nbrOfDetectors();++i)
        fDets.push_back(Detector(param.fDetectorParameters[i]));
}
