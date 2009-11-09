#include "pnccd_converter.h"

#include "pdsdata/xtc/Xtc.hh"
#include "pdsdata/xtc/TypeId.hh"
#include "pdsdata/xtc/DetInfo.hh"
#include "pdsdata/pnCCD/ConfigV1.hh"
#include "pdsdata/pnCCD/FrameV1.hh"
#include "cass_event.h"
#include "pnccd_event.h"


cass::pnCCD::Converter::Converter():
        _pnccdConfig(0)
{
    //this converter should react on pnccd config and frame//
    _types.push_back(Pds::TypeId::Id_pnCCDconfig);
    _types.push_back(Pds::TypeId::Id_pnCCDframe);
}


void cass::pnCCD::Converter::operator()(const Pds::Xtc* xtc, cass::CASSEvent* cassevent)
{
    // Check whether the xtc object id contains configuration or event data:
    switch( xtc->contains.id() )
    {
    case (Pds::TypeId::Id_pnCCDconfig) :
        //just store the config
        delete _pnccdConfig;
        _pnccdConfig = new Pds::PNCCD::ConfigV1();
        *_pnccdConfig = *(reinterpret_cast<const Pds::PNCCD::ConfigV1*>(xtc->payload()));
        break;


    case (Pds::TypeId::Id_pnCCDframe) :
        {
            //only run this if we have a config
            if (_pnccdConfig)
            {
                // Get a reference to the pnCCDEvent:
                pnCCDEvent &pnccdevent = cassevent->pnCCDEvent();
                //Get the frame from the xtc
                const Pds::PNCCD::FrameV1* frame = reinterpret_cast<const Pds::PNCCD::FrameV1*>(xtc->payload());
                //Get the the detecotor id //
                const Pds::DetInfo& info = *(Pds::DetInfo*)(&xtc->src);
                const size_t detectorId = info.devId();

                //if necessary resize the detector container//
                if (detectorId >= pnccdevent.detectors().size())
                    pnccdevent.detectors().resize(detectorId+1);

                //find out the total size of this frame//
                size_t sizeOfOneSegment = frame->sizeofData(*_pnccdConfig);
                size_t FrameSize = sizeOfOneSegment * _pnccdConfig->numLinks();
                //resize the frame to what we will receive//
                pnccdevent.detectors()[detectorId].rawFrame().resize(FrameSize);

                for (size_t i=0;i<_pnccdConfig->numLinks();++i)
                {
                    //pointer to first data element//
                    const uint16_t* data = frame->data();
                    //copy frame data to container using std::copy//
                    std::copy(data, data + sizeOfOneSegment,pnccdevent.detectors()[detectorId].rawFrame().begin()+(i*sizeOfOneSegment));
                    //iterate to the next frame segment//
                    frame = frame->next(*_pnccdConfig);
                }
            }
        }
        break;
    default:
        break;
    }
}
