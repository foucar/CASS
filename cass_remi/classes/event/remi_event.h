#ifndef __REMIEvent_h__
#define __REMIEvent_h__

#include <vector>
#include "detector.h"
#include "channel.h"

namespace cass
{
  namespace REMI
  {
    class Parameter;


    class REMIEvent
    {
      public:
            REMIEvent():
              _isFilled(false),
            _isConfigured(false)  {}
            ~REMIEvent()    {}

      public:
        typedef std::vector<Channel> channels_t;
        typedef std::vector<Detector> detectors_t;

      public:
        const channels_t   &channels()const         {return _channels;}
        channels_t         &channels()              {return _channels;}

      public:
        const detectors_t  &detectors()const        {return _dets;}
        detectors_t        &detectors()             {return _dets;}
        
      public:
        bool                isFilled()const         {return _isFilled;}
        bool               &isFilled()              {return _isFilled;}
        bool                isConfigured()const     {return _isConfigured;}
        bool               &isConfigured()          {return _isConfigured;}

      public:
        double              sampleInterval()const   {return _sampleInterval;}
        double             &sampleInterval()        {return _sampleInterval;}

      private:
        //status flags//
        bool                _isFilled;              //! flag to tell whether the event has been filled
        bool                _isConfigured;          //! flag to tell whether the event has been initalized with configv1

        //containers for acqiris and delayline data//
        channels_t          _channels;              //Container for all Channels
        detectors_t         _dets;                  //Container for all Detektors

        //variables from the acqiris instrument//
        double              _sampleInterval;        //the time between two consecutive points (in ns)
    };
  }//end namespace remi
}//end namespace cass
#endif
