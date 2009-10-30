#ifndef __REMIEvent_h__
#define __REMIEvent_h__

#include <vector>
#include "detector.h"
#include "channel.h"

namespace Pds
{
    namespace Acqiris
    {
        class ConfigV1;
        class DataDescV1;
    }
    namespace Camera
    {
        class FrameV1;
    }
}

#ifndef ROOT_Rtypes
#include <Rtypes.h>
#endif

#ifndef ROOT_TObject
#include <TObject.h>
#endif

namespace cass
{
    namespace REMI
    {
        class Parameter;


        class REMIEvent
        {
        public:
            REMIEvent():fIsFilled(false),fIsInitialized(false)  {}
            void                init(const Pds::Acqiris::DataDescV1&);
            void                init(const Pds::Acqiris::ConfigV1&);
            void                CopyParameters(const Parameter&);

        public:
            size_t              nbrOfChannels()const        {return fChannels.size();}
            Channel            &channel(long idx)           {return fChannels[idx];}
            const Channel      &channel(long idx)const      {return fChannels[idx];}
            const channels_t   &channels()const             {return fChannels;}

        public:
            size_t              nbrOfDetectors()const       {return fDets.size();}
            Detector           &detector(long idx)          {return fDets[idx];}
            const Detector     &detector(long idx)const     {return fDets[idx];}
        
        public:
            bool                isFilled()const             {return fIsFilled;}
            bool                isInitialized()const        {return fIsInitialized;}

        public:
            double              horpos()const               {return fHorpos;}
            int16_t             nbrBytes()const             {return fNbrBytes;}
            double              sampleInterval()const       {return fSampleInterval;}
            int32_t             nbrSamples()const           {return fNbrSamples;}
            double              delayTime()const            {return fDelayTime;}
            double              trigLevel()const            {return fTrigLevel;}
            int16_t             trigSlope()const            {return fTrigSlope;}
            int16_t             trigChannel()const          {return fTrigChannel;}
            uint32_t            chanCombUsedChannels()const {return fChanCombUsedChans;}
            int16_t             nbrConvPerChan()const       {return fNbrConPerCh;}

        private:
            bool                fIsFilled;                  //flag to tell whether the event has been filled
            bool                fIsInitialized;             //flag to tell whether the event has been initalized with configv1
            double              fHorpos;                    //the fHorpos value for this event
            channels_t          fChannels;                  //Container for all Channels
            detectors_t         fDets;                      //Container for all Detektors

            int16_t             fNbrBytes;                  //Nbr of bytes of the adc values (either 1 or 2)
            double              fSampleInterval;            //the time between two consecutive points (in ns)
            uint32_t            fNbrSamples;                //Nbr of Points (multiplied by the fSampInter it will give the timewindow in ns)
            double              fDelayTime;                 //the delay of the trigger with respect to the window
            uint32_t            fTrigChannel;               //the fTriggering Channel
            double              fTrigLevel;                 //the trigger Level from the Offset
            uint32_t            fTrigSlope;                 //which Slope was used by the fTrigger
            uint32_t            fChanCombUsedChans;         //Bitmask discribing which Converters per Channel have been used
            uint32_t            fNbrConPerCh;               //tells how many converts per channel have been used
            ClassDefNV(REMIEvent,1)
        };
    }//end namespace remi
}//end namespace cass
#endif
