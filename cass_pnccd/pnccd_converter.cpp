#include "pnccd_converter.h"

#include <iostream>

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
        const Pds::PNCCD::FrameV1* frameSegment = reinterpret_cast<const Pds::PNCCD::FrameV1*>(xtc->payload());
        //Get the the detecotor id //
        const Pds::DetInfo& info = *(Pds::DetInfo*)(&xtc->src);
        const size_t detectorId = info.devId();

        //if necessary resize the detector container//
        if (detectorId >= pnccdevent.detectors().size())
          pnccdevent.detectors().resize(detectorId+1);

        //get a reference to the detector we are working on right now//
        cass::pnCCD::pnCCDDetector& det = pnccdevent.detectors()[detectorId];

        //find out the total size of this frame//
        const size_t sizeOfOneSegment = frameSegment->sizeofData(*_pnccdConfig);
        const size_t NbrOfSegments = _pnccdConfig->numLinks();
        const size_t FrameSize = sizeOfOneSegment * NbrOfSegments;

        //resize the frame to what we will receive//
        det.rawFrame().resize(FrameSize);

        //create a container for pointers to the beginning of the data for all segments//
        std::vector<const uint16_t*> datapointers(NbrOfSegments,0);
        //go through all segments and get the pointers to the beginning//
        for (size_t i=0; i<NbrOfSegments ;++i)
        {
          //pointer to first data element of segment//
          datapointers[i] = frameSegment->data();
          //iterate to the next frame segment//
          frameSegment = frameSegment->next(*_pnccdConfig);
        }

        //calc the Number of Rows and Colums in one Segment//
        ////
        const size_t rowsOfSegment = det.rows() / 2;
        const size_t columnsOfSegment = det.columns() / 2;


        //go through each row of each element and align the data that it is//
        //1 row of 1segment : 1 row of 2segment : ...  : 1 row of last segment : 2 row of 1 segment : ... : 2 row of last segment : .... : last row of last segment
        //create a iterator for the raw frame//
        cass::pnCCD::pnCCDDetector::frame_t::iterator it = det.rawFrame().begin();
        for (size_t iRow = 0; iRow<rowsOfSegment; ++iRow)
        {
          for (size_t iSegment = 0; iSegment<NbrOfSegments; ++iSegment)
          {
            //copy the row of this segment//
            std::copy(datapointers[iSegment],
                      datapointers[iSegment] + columnsOfSegment,
                      it);
            //advance the iterators by size of columns of one segment//
            it += columnsOfSegment;
            datapointers[iSegment] += columnsOfSegment;
          }
        }
      }
    }
    break;

  default:
    break;
  }
}
