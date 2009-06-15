#ifndef __RemiAnalysisEvent_h__
#define __RemiAnalysisEvent_h__

#include <vector>
#include "pdsdata/acqiris/ConfigV1.hh"
#include "pdsdata/acqiris/DataDescV1.hh"
#include "Detector.h"
#include "Channel.h"



namespace cass
{
	namespace REMI
	{

                //class Channel;
                //class Detector;
		class Parameter;
		
		typedef std::vector<Channel> channels_t;
		typedef std::vector<Detector> detectors_t;

		class RemiAnalysisEvent
		{
		public:
			RemiAnalysisEvent(Pds::Acqiris::ConfigV1&, Pds::Acqiris::DataDescV1&, const Parameter&);

		public:
			size_t			 nbrOfChannels()const			{return fChannels.size();}
			Channel			&channel(long idx)				{return fChannels[idx];}
			const Channel	&channel(long idx)const			{return fChannels[idx];}
			const channels_t&channels()const				{return fChannels;}

		public:
			size_t			 nbrOfDetektors()const			{return fDets.size();}
			Detector		&detector(long idx)				{return fDets[idx];}
			const Detector	&detector(long idx)const		{return fDets[idx];}

		public:
                        unsigned __int64	 eventID()const					{return fEventID;}
			double			 horpos()const					{return fHorpos;}
			short			 nbrBytes()const				{return fNbrBytes;}
			double			 sampleInterval()const			{return fSampInter;}
			long			 nbrSamples()const				{return fNbrSamples;}
			double			 delayTime()const				{return fDelayTime;}
			double			 trigLevel()const				{return fTrigLevel;}
			short			 trigSlope()const				{return fTrigSlope;}
			long			 chanCombUsedChannels()const	{return fChanCombUsedChans;}
			short			 nbrConvPerChan()const			{return fNbrConPerCh;}

		private:
                        unsigned __int64          fEventID;						//an id for this event. It is just the time the event was recorded
			double			 fHorpos;						//the fHorpos value for this event
			channels_t		 fChannels;						//Container for all Channels
			detectors_t		 fDets;							//Container for all Detektors

			short			 fNbrBytes;						//Nbr of bytes of the adc values (either 1 or 2)
			double			 fSampInter;					//the time between two consecutive points (in ns)
			long			 fNbrSamples;					//Nbr of Points (multiplied by the fSampInter it will give the timewindow in ns)
			double			 fDelayTime;					//the delay of the trigger with respect to the window
			short			 fTrigChan;						//the fTriggering Channel
			double			 fTrigLevel;					//the trigger Level from the Offset
			short			 fTrigSlope;					//which Slope was used by the fTrigger
			long			 fChanCombUsedChans;			//Bitmask discribing which Converters per Channel have been used
			short			 fNbrConPerCh;					//tells how many converts per channel have been used
		};

	}//end namespace remi
}//end namespace cass
#endif
