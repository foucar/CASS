// Copyright (C) 2009, 2010 Lutz Foucar

#include "pnccd_converter.h"

#include <iostream>
#include <stdexcept>

#include "pdsdata/xtc/Xtc.hh"
#include "pdsdata/xtc/TypeId.hh"
#include "pdsdata/xtc/DetInfo.hh"
#include "pdsdata/pnCCD/ConfigV1.hh"
#include "pdsdata/pnCCD/ConfigV2.hh"
#include "pdsdata/pnCCD/FrameV1.hh"
#include "cass_event.h"
#include "pnccd_device.h"
#include "pixel_detector.h"
#include "pnccd_detector.h"

void cass::pnCCD::Converter::operator()(const Pds::Xtc* xtc, cass::CASSEvent* cassevent)
{
  // Check whether the xtc object id contains configuration or event data:
  switch( xtc->contains.id() )
  {
  case (Pds::TypeId::Id_pnCCDconfig) :
    {
      //Get the the detecotor id //
      const Pds::DetInfo& info = *(Pds::DetInfo*)(&xtc->src);
      const size_t detectorId = info.devId();
      //the configuration version//
      uint32_t version = xtc->contains.version();

      std::cout << "pnCCDConverter::ConfigXTC: DeviceId:"<<detectorId
          <<" Version: "<<version
          <<std::endl;
      //if necessary resize the config container//
      if (detectorId >= _pnccdConfig.size())
        _pnccdConfig.resize(detectorId+1,std::make_pair<uint32_t,Pds::PNCCD::ConfigV2*>(version,0));

      //retrieve the reference to the right pointer to config//
      //and store the transmitted config//
      Pds::PNCCD::ConfigV2 *&pnccdConfig = _pnccdConfig[detectorId].second;
      delete pnccdConfig;
      pnccdConfig = new Pds::PNCCD::ConfigV2();
      *pnccdConfig = *(reinterpret_cast<const Pds::PNCCD::ConfigV2*>(xtc->payload()));
    }
    break;


  case (Pds::TypeId::Id_pnCCDframe) :
    {
      // Get a reference to the pnCCDDevice
      pnCCDDevice &dev =
          *dynamic_cast<pnCCDDevice*>(cassevent->devices()[cass::CASSEvent::pnCCD]);
      //Get the frame from the xtc
      const Pds::PNCCD::FrameV1* frameSegment =
          reinterpret_cast<const Pds::PNCCD::FrameV1*>(xtc->payload());
      //Get the the detecotor id //
      const Pds::DetInfo& info = *(Pds::DetInfo*)(&xtc->src);
      const size_t detectorId = info.devId();

      //if necessary resize the detector container//
      if (detectorId >= dev.detectors()->size())
        dev.detectors()->resize(detectorId+1,pnCCD::PnCCDDetector());

      //only convert if we have a config for this detector
      if (_pnccdConfig.size() > detectorId)
      if (_pnccdConfig[detectorId].second)
      {
        //get a reference to the detector we are working on right now//
        PnCCDDetector& det = dynamic_cast<PnCCDDetector&>((*dev.detectors())[detectorId]);
        //get the pointer to the config for this detector//
        const Pds::PNCCD::ConfigV2 *pnccdConfig = _pnccdConfig[detectorId].second;

        size_t rowsOfSegment(0);
        size_t columnsOfSegment(0);
        //check the version and retrieve info of detector//
        switch(_pnccdConfig[detectorId].first)
        {
        case 1:
          //we need to set the rows and columns hardcoded since the information is not yet
          //provided by LCLS in Version 1//
          det.rows() = det.columns() = pnCCD_default_size;
          det.originalrows() = det.originalcolumns() = pnCCD_default_size;
          rowsOfSegment = pnCCD_default_size/2;
          columnsOfSegment = pnCCD_default_size/2;
          break;
        case 2:
          //get the rows and columns from config//
          det.rows() = pnccdConfig->numRows();
          det.columns() = pnccdConfig->numChannels();
          det.originalrows() =  pnccdConfig->numRows();
          det.originalcolumns() = pnccdConfig->numChannels();
          columnsOfSegment = pnccdConfig->numSubmoduleChannels();
          rowsOfSegment = pnccdConfig->numSubmoduleRows();
          break;
        default:
          throw std::range_error("Unsupported pnCCD configuration version");
        }

        //find out the total size of this frame//
        const size_t sizeOfOneSegment = frameSegment->sizeofData(*pnccdConfig);
        const size_t NbrOfSegments = pnccdConfig->numLinks();
        const size_t FrameSize = sizeOfOneSegment * NbrOfSegments;

        //resize the frame to what we will receive//
        det.frame().resize(FrameSize);

        //create a container for pointers to the beginning of the data for all segments//
        std::vector<const uint16_t*> xtcSegmentPointers(NbrOfSegments,0);
        //go through all segments and get the pointers to the beginning//
        for (size_t i=0; i<NbrOfSegments ;++i)
        {
          //pointer to first data element of segment//
          xtcSegmentPointers[i] = frameSegment->data();
          //iterate to the next frame segment//
          frameSegment = frameSegment->next(*pnccdConfig);
        }

        //reorient the xtc segements to real frame segments//
        std::vector<const uint16_t*> frameSegmentPointers(NbrOfSegments,0);
        frameSegmentPointers[0] = xtcSegmentPointers[0];
        frameSegmentPointers[1] = xtcSegmentPointers[3];
        frameSegmentPointers[2] = xtcSegmentPointers[1];
        frameSegmentPointers[3] = xtcSegmentPointers[2];

        //go through each row of each element and align the data that it is//
        //create a iterator for the raw frame//
        cass::PixelDetector::frame_t::iterator it = det.frame().begin();
        //we need to do the reordering of the segments here
        //go through the all rows of the first two segments and //
        //do first row , first row, second row , second row ...//
        //The HLL People said the one needs to ignore the upper two bits//
        //so ignore them//

        for (size_t iRow=0; iRow<rowsOfSegment ;++iRow)
        {
          //copy the row of first segment//
          for (size_t iCol=0; iCol<columnsOfSegment ;++iCol)
            *it++ =  *frameSegmentPointers[0]++ & 0x3fff;
          //copy the row of second segment//
          for (size_t iCol=0; iCol<columnsOfSegment ;++iCol)
            *it++ =  *frameSegmentPointers[1]++ & 0x3fff;
        }
        //go through the all rows of the next two segments and //
        //do last row reversed, last row reversed, last row -1 reversed , last row -1 reversed...//
        //therefore we need to let the datapointers of the last two segments//
        //point to the end of the segement//
        frameSegmentPointers[2] += rowsOfSegment*columnsOfSegment - 1;
        frameSegmentPointers[3] += rowsOfSegment*columnsOfSegment - 1;
        for (size_t iRow=0; iRow<rowsOfSegment ;++iRow)
        {
          //copy row of 3rd segment reversed
          for (size_t iCol=0; iCol<columnsOfSegment ;++iCol)
            *it++ = *frameSegmentPointers[2]-- & 0x3fff;
          //copy row of 4th segement reversed
          for (size_t iCol=0; iCol<columnsOfSegment ;++iCol)
            *it++ = *frameSegmentPointers[3]--  & 0x3fff;
        }
      }
    }
    break;

  default:
    break;
  }
}
