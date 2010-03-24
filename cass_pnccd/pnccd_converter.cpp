#include "pnccd_converter.h"

#include <iostream>

#include "pdsdata/xtc/Xtc.hh"
#include "pdsdata/xtc/TypeId.hh"
#include "pdsdata/xtc/DetInfo.hh"
#include "pdsdata/pnCCD/ConfigV1.hh"
#include "pdsdata/pnCCD/FrameV1.hh"
#include "cass_event.h"
#include "pnccd_event.h"
#include "pnccd_analysis.h"

#include <sys/time.h>

int timeval_subtract1(struct timeval *result, struct timeval *t2, struct timeval *t1)
{
    long int diff = (t2->tv_usec + 1000000 * t2->tv_sec) - (t1->tv_usec + 1000000 * t1->tv_sec);
    result->tv_sec = diff / 1000000;
    result->tv_usec = diff % 1000000;

    return (diff<0);
}

cass::pnCCD::Converter::Converter()
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
    {
      //Get the the detecotor id //
      const Pds::DetInfo& info = *(Pds::DetInfo*)(&xtc->src);
      const size_t detectorId = info.devId();

      //if necessary resize the config container//
      if (detectorId >= _pnccdConfig.size())
        _pnccdConfig.resize(detectorId+1,0);

      //retrieve the reference to the right pointer to config//
      //and store the transmitted config//
      Pds::PNCCD::ConfigV1 *&pnccdConfig = _pnccdConfig[detectorId];
      delete pnccdConfig;
      pnccdConfig = new Pds::PNCCD::ConfigV1();
      *pnccdConfig = *(reinterpret_cast<const Pds::PNCCD::ConfigV1*>(xtc->payload()));
    }
    break;


  case (Pds::TypeId::Id_pnCCDframe) :
    {
#ifdef debug
      struct timeval tvBegin, tvEnd, tvDiff;
      gettimeofday(&tvBegin, NULL);
#endif
      // Get a reference to the pnCCDEvent:
      pnCCDEvent &pnccdevent = cassevent->pnCCDEvent();
      //Get the frame from the xtc
      const Pds::PNCCD::FrameV1* frameSegment =
          reinterpret_cast<const Pds::PNCCD::FrameV1*>(xtc->payload());
      //Get the the detecotor id //
      const Pds::DetInfo& info = *(Pds::DetInfo*)(&xtc->src);
      const size_t detectorId = info.devId();

//      std::cout<< detectorId << " " << pnccdevent.detectors().size()<<std::endl;
      //if necessary resize the detector container//
      if (detectorId >= pnccdevent.detectors().size())
        pnccdevent.detectors().resize(detectorId+1);

      //only run this if we have a config for this detector
      if (_pnccdConfig.size() > detectorId)
      if (_pnccdConfig[detectorId])
      {
        //get a reference to the detector we are working on right now//
        cass::pnCCD::pnCCDDetector& det = pnccdevent.detectors()[detectorId];
//        std::cout<<detectorId<< " a "<< det.rows() << " " <<  det.columns() 
//                 << " " << det.originalrows() << " " <<det.originalcolumns()
//                 <<" "<< pnccdevent.detectors().size() <<std::endl;

        //get the pointer to the config for this detector//
        Pds::PNCCD::ConfigV1 *pnccdConfig = _pnccdConfig[detectorId];
        //std::cout<<"test "<< _param.detROI[detectorId]._ROI[0].xsize<<std::endl;

        //we need to set the rows and columns hardcoded since the information is not yet
        //provided by LCLS//
        det.rows() = det.columns() = 1024;
        det.originalrows() = det.originalcolumns() = 1024;

        //find out the total size of this frame//
        const size_t sizeOfOneSegment = frameSegment->sizeofData(*pnccdConfig);
        const size_t NbrOfSegments = pnccdConfig->numLinks();
        const size_t FrameSize = sizeOfOneSegment * NbrOfSegments;

        //resize the frame to what we will receive//
        det.rawFrame().resize(FrameSize);

        //create a container for pointers to the beginning of the data for all segments//
        std::vector<const uint16_t*> xtcSegmentPointers(NbrOfSegments,0);
        std::vector<const uint16_t*> frameSegmentPointers(NbrOfSegments,0);
        //go through all segments and get the pointers to the beginning//
        for (size_t i=0; i<NbrOfSegments ;++i)
        {
          //pointer to first data element of segment//
          xtcSegmentPointers[i] = frameSegment->data();
          //iterate to the next frame segment//
          frameSegment = frameSegment->next(*pnccdConfig);
        }

        //calc the Number of Rows and Colums in one Segment//
        const size_t rowsOfSegment = det.rows() / 2;
        const size_t columnsOfSegment = det.columns() / 2;
        //if(_param._This_is_a_dark_run==0) std::cout<<"this is not a dark run"<<std::endl;

        //reorient the xtc segements to real frame segments//
        frameSegmentPointers[0] = xtcSegmentPointers[0];
        frameSegmentPointers[1] = xtcSegmentPointers[3];
        frameSegmentPointers[2] = xtcSegmentPointers[1];
        frameSegmentPointers[3] = xtcSegmentPointers[2];

        //go through each row of each element and align the data that it is//
        //1 row of 1segment : 1 row of 2segment : ...  : 1 row of last segment : 2 row of 1 segment : ... : 2 row of last segment : .... : last row of last segment
        //create a iterator for the raw frame//
        cass::pnCCD::pnCCDDetector::frame_t::iterator it = det.rawFrame().begin();
//        for (size_t iRow = 0; iRow<rowsOfSegment; ++iRow)
//        {
//          for (size_t iSegment = 0; iSegment<NbrOfSegments; ++iSegment)
//          {
//            //copy the row of this segment//
//            std::copy(datapointers[iSegment],
//                      datapointers[iSegment] + columnsOfSegment,
//                      it);
//            //advance the iterators by size of columns of one segment//
//            it += columnsOfSegment;
//            datapointers[iSegment] += columnsOfSegment;
//          }
//        }
        //for now we might want to do the reordering of the segments and
        //reallignment of the nils stuff here
        //go through the all rows of the first two segments and //
        //do first row , first row, second row , second row ...//
        for (size_t iRow=0; iRow<rowsOfSegment ;++iRow)
        {
          //copy the row of first segment//
          std::copy(frameSegmentPointers[0],
                    frameSegmentPointers[0] + columnsOfSegment,
                    it);
          //advance the iterators by size of columns of one segment//
          it += columnsOfSegment;
          frameSegmentPointers[0] += columnsOfSegment;

          //copy the row of second segment//
          std::copy(frameSegmentPointers[1],
                    frameSegmentPointers[1] + columnsOfSegment,
                    it);
          //advance the iterators by size of columns of one segment//
          it += columnsOfSegment;
          frameSegmentPointers[1] += columnsOfSegment;
        }
        //go through the all rows of the next two segments and //
        //do last row reversed, last row reversed, last row -1 reversed , last row -1 reversed...//
        //therefore we need to adjust the datapointers of the last two segments//
        for (size_t iSegment=NbrOfSegments/2; iSegment<NbrOfSegments ; ++iSegment)
          frameSegmentPointers[iSegment] += ((rowsOfSegment-1)*columnsOfSegment) + columnsOfSegment-1;
        for (size_t iRow=0; iRow<rowsOfSegment ;++iRow)
        {
          for (size_t iCol=0; iCol<columnsOfSegment ;++iCol)
            *it++ = *frameSegmentPointers[2]--;
          for (size_t iCol=0; iCol<columnsOfSegment ;++iCol)
            *it++ = *frameSegmentPointers[3]--;
        }
      }
#ifdef debug
      gettimeofday(&tvEnd, NULL);
      timeval_subtract1(&tvDiff, &tvEnd, &tvBegin);
      printf("time_diff_converter is %ld.%06ld\n", tvDiff.tv_sec, tvDiff.tv_usec);
#endif
    }
    break;

  default:
    break;
  }
}
