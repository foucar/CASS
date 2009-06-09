#include <iostream>
#include "Event.h"

//______________________________________________________________________________________________________________________
cass::REMI::Event::Event(const Pds::Acqiris::ConfigV1& config, const Pds::Acqiris::DataDescV1& ddesc, const cass::REMI::Parameter &param)
{
	fHorpos				= ddesc.timestamp(0).horpos();				//horpos from acqiris
	fNbrBytes			= 2
	fSampInter			= config.horiz().sampInterval();
	fNbrSamples			= config.horiz().nbrSamples();
	fDelayTime			= config.horiz().delayTime();
	fTrigChan			= config.trig().trigInput();
	fTrigLevel			= config.trig().trigLevel();
	fTrigSlope			= config.trig().trigSlope();
	fChanCombUsedChans	= config.channelMask();
	fNbrConPerCh		= config.nbrConvertersPerChannel();

	//create the channels//
	for (size_t i=0;i<config.nbrChannels();++i)
	{
		fChannels.push_back(Channel(i,config,ddesc,param));
		dd = dd->nextChannel(config.horiz());
	}

	//create the detectors
	for (size_t i=0;i<param.nbrOfDetectors;++i)
		fDets.push_back(Detector(param.fDetPara[i]));
}
