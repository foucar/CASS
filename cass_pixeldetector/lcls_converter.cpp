// Copyright (C) 2009, 2010, 2011 Lutz Foucar

/**
 * @file lcls_converter.cpp contains the converters to convert ccd and pnccd data
 *                          to CASSEvent
 *
 * @author Lutz Foucar
 */

#include "lcls_converter.h"

#include <iostream>
#include <stdexcept>

#include "pdsdata/xtc/Xtc.hh"
#include "pdsdata/xtc/TypeId.hh"
#include "pdsdata/xtc/DetInfo.hh"
#include "pdsdata/pnCCD/ConfigV1.hh"
#include "pdsdata/pnCCD/ConfigV2.hh"
#include "pdsdata/pnCCD/FrameV1.hh"

#include "cass_event.h"
#include "pixeldetector.hpp"

using namespace cass;
using namespace pixeldetector;
using namespace lclsid;
using namespace std;
using std::tr1::shared_ptr;
using namespace Pds;

namespace cass
{
namespace pixeldetector
{
namespace lclsid
{
/** Key for IdConversion Map
 *
 * description
 *
 * @author Lutz Foucar
 */
class Key
{
public:
  /** constructor
   *
   * @param
   */
  Key(const TypeId::Type& detectorType,
      const DetInfo::Detector& det, uint32_t detId,
      const DetInfo::Device& dev, uint32_t devId)
    :_detectorType(detectorType),
     _detector(((det&0xff)<<24) | ((detId&0xff)<<16) | ((dev&0xff)<<8) |(devId&0xff))
  {}

  /** constructor
   *
   * @param
   */
  Key(TypeId::Type detectorType, uint32_t physicalId)
    :_detectorType(detectorType),_detector(physicalId)
  {}

  /** check whether this is less than other
   *
   * @param
   */
  bool operator <(const Key& other) const
  {
    if (_detectorType != other._detectorType)
      return _detectorType < other._detectorType;
    return _detector < other._detector;
  }

private:
  /** */
  uint32_t _detectorType;

  /** */
  uint32_t _detector;
};
}//end namespace Id
}//end namepsace pixeldetector
}//end namespace cass

// =================define static members =================
cass::ConversionBackend::shared_pointer Converter::_instance;
QMutex Converter::_mutex;

cass::ConversionBackend::shared_pointer Converter::instance()
{
  QMutexLocker locker(&_mutex);
  if(!_instance)
  {
    _instance = ConversionBackend::shared_pointer(new Converter());
  }
  return _instance;
}
// ========================================================


Converter::Converter()
{
  _pdsTypeList.push_back(TypeId::Id_pnCCDconfig);
  _pdsTypeList.push_back(TypeId::Id_pnCCDframe);
  _pdsTypeList.push_back(TypeId::Id_Frame);

  Key FrontPnCCD(TypeId::Id_pnCCDframe,
                 DetInfo::Camp, 3,
                 DetInfo::pnCCD, 5);
  Key BackPnCCD(TypeId::Id_pnCCDframe,
                 DetInfo::Camp, 3,
                 DetInfo::pnCCD, 5);
  Key commCCD1(TypeId::Id_Frame,
               DetInfo::Camp, 3,
               DetInfo::pnCCD, 5);
  Key commCCD2(TypeId::Id_Frame,
               DetInfo::Camp, 3,
               DetInfo::pnCCD, 5);
  Key commCCD3(TypeId::Id_Frame,
               DetInfo::Camp, 3,
               DetInfo::pnCCD, 5);
  LCLSToCASSId[FrontPnCCD] = 1;
}

void Converter::operator()(const Pds::Xtc* xtc, CASSEvent* cassevent)
{
  cout<< "XTC: '"<<TypeId::name(xtc->contains.id())
      <<"'("<<xtc->contains.id()
      <<"), '"<<DetInfo::name(reinterpret_cast<const DetInfo*>(&xtc->src)->detector())
      <<"'("<<reinterpret_cast<const DetInfo*>(&xtc->src)->detId()
      <<"), '"<<DetInfo::name(reinterpret_cast<const DetInfo*>(&xtc->src)->device())
      <<"'("<<reinterpret_cast<const DetInfo*>(&xtc->src)->detId()
      <<")"<<endl;

  idmap_t::key_type lclskey(xtc->contains.id(), xtc->src.phy());
  idmap_t::iterator lclsmapIt(LCLSToCASSId.find(lclskey));
  if (lclsmapIt == LCLSToCASSId.end())
  {
    throw runtime_error("pixeldetector::Converter::operator(): There is no corresponding cass key for '" + string(TypeId::name(xtc->contains.id())) +
                        "'(" + toString(xtc->contains.id()) +
                        "), '" + DetInfo::name(reinterpret_cast<const DetInfo*>(&xtc->src)->detector()) +
                        "'(" + toString(reinterpret_cast<const DetInfo*>(&xtc->src)->detId()) +
                        "), '" + DetInfo::name(reinterpret_cast<const DetInfo*>(&xtc->src)->device()) +
                        "'(" + toString(reinterpret_cast<const DetInfo*>(&xtc->src)->detId()));
  }
  const idmap_t::mapped_type &casskey(lclsmapIt->second);

  switch( xtc->contains.id() )
  {
  case (Pds::TypeId::Id_pnCCDconfig) :
    {
      uint32_t version = xtc->contains.version();
      config_t config
          (make_pair(version,shared_ptr<PNCCD::ConfigV2>(new PNCCD::ConfigV2())));
      *(config.second) = *(reinterpret_cast<const Pds::PNCCD::ConfigV2*>(xtc->payload()));
      _pnccdConfigStore[casskey] = config;
    }
    break;


  case (Pds::TypeId::Id_pnCCDframe) :
    {
//      // Get a reference to the pnCCDDevice
//      pnCCDDevice &dev =
//          *dynamic_cast<pnCCDDevice*>(cassevent->devices()[cass::CASSEvent::pnCCD]);
//      //Get the frame from the xtc
//      const Pds::PNCCD::FrameV1* frameSegment =
//          reinterpret_cast<const Pds::PNCCD::FrameV1*>(xtc->payload());
//      //Get the the detecotor id //
//      const Pds::DetInfo& info = *(Pds::DetInfo*)(&xtc->src);
//      const size_t detectorId = info.devId();

//      //DEBUGOUT(std::cout << "pnCCD::Converter frame debug " << detectorId << " "<< cassevent->id() << std::endl);

//      //if necessary resize the detector container//
////      std::cout<<"pnCCDConverter::DataXTC: resizing container"<<std::endl;
//      if (detectorId >= dev.detectors()->size())
//        dev.detectors()->resize(detectorId+1);
////      std::cout<<"pnCCDConverter::DataXTC: done resizing"<<std::endl;

//      //only convert if we have a config for this detector
//      if (_pnccdConfig.size() > detectorId)
//      if (_pnccdConfig[detectorId].second)
//      {
//        //get a reference to the detector we are working on right now//
////        std::cout<<"pnCCDConverter::DataXTC: getting det from container"<<std::endl;
//        PixelDetector& det = (*dev.detectors())[detectorId];
////        std::cout<<"pnCCDConverter::DataXTC: done getting det from container"<<std::endl;
//        //get the pointer to the config for this detector//
//        const Pds::PNCCD::ConfigV2 *pnccdConfig = _pnccdConfig[detectorId].second;

//        size_t rowsOfSegment(0);
//        size_t columnsOfSegment(0);
//        //check the version and retrieve info of detector//
//        switch(_pnccdConfig[detectorId].first)
//        {
//        case 1:
//          //we need to set the rows and columns hardcoded since the information is not yet
//          //provided by LCLS in Version 1//
//          det.rows() = det.columns() = pnCCD::default_size;
//          det.originalrows() = det.originalcolumns() = pnCCD::default_size;
//          rowsOfSegment = pnCCD::default_size/2;
//          columnsOfSegment = pnCCD::default_size/2;
//          break;
//        case 2:
//          //get the rows and columns from config//
//          det.rows() = pnccdConfig->numRows();
//          det.columns() = pnccdConfig->numChannels();
//          det.originalrows() =  pnccdConfig->numRows();
//          det.originalcolumns() = pnccdConfig->numChannels();
//          det.info().assign(pnccdConfig->info());
//          det.timingFilename().assign(pnccdConfig->timingFName());
//          det.camaxMagic() = pnccdConfig->camexMagic();
//          rowsOfSegment = pnccdConfig->numSubmoduleRows();
//          columnsOfSegment = pnccdConfig->numSubmoduleChannels();
//          if(det.rows()>1024||det.columns()>1024||rowsOfSegment>512||columnsOfSegment>512)
//          {
//            std::cout<<"pnCCDConverter::DataXTC: rows:"<<det.rows()<<std::endl;
//            std::cout<<"pnCCDConverter::DataXTC: cols:"<<det.columns()<<std::endl;
//            std::cout<<"pnCCDConverter::DataXTC: info:"<<det.info()<<std::endl;
//            std::cout<<"pnCCDConverter::DataXTC: tfileName:"<<det.timingFilename()<<std::endl;
//            std::cout<<"pnCCDConverter::DataXTC: camaxMagic:"<<std::hex<<det.camaxMagic()<<std::dec<<std::endl;
//            std::cout<<"pnCCDConverter::DataXTC: SegRows:"<<rowsOfSegment<<std::endl;
//            std::cout<<"pnCCDConverter::DataXTC: SegCols:"<<columnsOfSegment<<std::endl;
//            //in this case I am overwriting the values
//            rowsOfSegment=512;
//            columnsOfSegment=512;
//            det.rows() = 1024;
//            det.columns() = 1024;
//            det.originalrows() = 1024;
//            det.originalcolumns() = 1024;
//          }
//          break;
//        default:
//          throw std::range_error("Unsupported pnCCD configuration version");
//        }

//        //find out the total size of this frame//
//        const size_t sizeOfOneSegment = frameSegment->sizeofData(*pnccdConfig);
//        const size_t NbrOfSegments = pnccdConfig->numLinks();
//        const size_t FrameSize = sizeOfOneSegment * NbrOfSegments;

//        //std::cout << "test " << sizeOfOneSegment << " " << NbrOfSegments
//        //          << " " << FrameSize<<std::endl;

//        //resize the frame to what we will receive//
//        det.frame().resize(FrameSize);

//        //create a container for pointers to the beginning of the data for all segments//
//        std::vector<const uint16_t*> xtcSegmentPointers(NbrOfSegments,0);
//        //go through all segments and get the pointers to the beginning//
//        for (size_t i=0; i<NbrOfSegments ;++i)
//        {
//          //pointer to first data element of segment//
//          xtcSegmentPointers[i] = frameSegment->data();
//          //iterate to the next frame segment//
//          frameSegment = frameSegment->next(*pnccdConfig);
//        }

//        //reorient the xtc segements to real frame segments//
//        std::vector<const uint16_t*> frameSegmentPointers(NbrOfSegments,0);
//        frameSegmentPointers[0] = xtcSegmentPointers[0];
//        frameSegmentPointers[1] = xtcSegmentPointers[3];
//        frameSegmentPointers[2] = xtcSegmentPointers[1];
//        frameSegmentPointers[3] = xtcSegmentPointers[2];

//        //go through each row of each element and align the data that it is//
//        //create a iterator for the raw frame//
//        cass::PixelDetector::frame_t::iterator it = det.frame().begin();
//        //we need to do the reordering of the segments here
//        //go through the all rows of the first two segments and //
//        //do first row , first row, second row , second row ...//
//        //The HLL People said the one needs to ignore the upper two bits//
//        //so ignore them//

//        for (size_t iRow=0; iRow<rowsOfSegment ;++iRow)
//        {
//          //copy the row of first segment//
//          for (size_t iCol=0; iCol<columnsOfSegment ;++iCol)
//            *it++ =  *frameSegmentPointers[0]++ & 0x3fff;
//          //copy the row of second segment//
//          for (size_t iCol=0; iCol<columnsOfSegment ;++iCol)
//            *it++ =  *frameSegmentPointers[1]++ & 0x3fff;
//        }
//        //go through the all rows of the next two segments and //
//        //do last row reversed, last row reversed, last row -1 reversed , last row -1 reversed...//
//        //therefore we need to let the datapointers of the last two segments//
//        //point to the end of the segement//
//        frameSegmentPointers[2] += rowsOfSegment*columnsOfSegment - 1;
//        frameSegmentPointers[3] += rowsOfSegment*columnsOfSegment - 1;
//        for (size_t iRow=0; iRow<rowsOfSegment ;++iRow)
//        {
//          //copy row of 3rd segment reversed
//          for (size_t iCol=0; iCol<columnsOfSegment ;++iCol)
//            *it++ = *frameSegmentPointers[2]-- & 0x3fff;
//          //copy row of 4th segement reversed
//          for (size_t iCol=0; iCol<columnsOfSegment ;++iCol)
//            *it++ = *frameSegmentPointers[3]--  & 0x3fff;
//        }
//      }
    }
    break;

  case (Pds::TypeId::Id_Frame) :
  {

//    //Get the the detector's device id //
//    const Pds::DetInfo& info = *(Pds::DetInfo*)(&xtc->src);
//    const size_t detectorId = info.detId();
//    //  cout<<"CCDConverter::XTCData: DetectorID:"<<info.detId()
//    //      <<" DeviceId:"<< info.devId()
//    //      <<" Detector:"<<info.detector()<<"("<<Pds::DetInfo::name(info.detector())<<")"
//    //      <<" Device:"<< info.device()<<"("<<Pds::DetInfo::name(info.device())<<")"
//    //      <<endl;
//    //retrieve a reference to the frame contained int the xtc//
//    const Pds::Camera::FrameV1 &frame =
//        *reinterpret_cast<const Pds::Camera::FrameV1*>(xtc->payload());
//    //retrieve a pointer to the ccd device we are working on//
//    cass::CCD::CCDDevice* dev = dynamic_cast<cass::CCD::CCDDevice*>(cassevent->devices()[cass::CASSEvent::CCD]);
//    //if necessary resize the detector container//
//    if (detectorId >= dev->detectors()->size())
//      dev->detectors()->resize(detectorId+1);
//    //retrieve a reference to the commercial ccd detector//
//    cass::PixelDetector& det = (*dev->detectors())[detectorId];
//    //  cout<< dev->detectors()->size()
//    //      <<" "<<detectorId()
//    //      <<" "<<frame.width()
//    //      <<" "<<frame.height()
//    //      <<" "<<frame.offset()
//    //      <<endl;

//    //copy the values status values from the frame to the detector//
//    det.columns()          = frame.width();
//    det.rows()             = frame.height();
//    det.originalcolumns()  = frame.width();
//    det.originalrows()     = frame.height();

//    //copy the frame data to this detector and do a type convertion implicitly//
//    const uint16_t* framedata (reinterpret_cast<const uint16_t*>(frame.data()));
//    const size_t framesize(frame.width()*frame.height());
//    //make frame big enough to take all data//
//    det.frame().resize(framesize);
//    det.frame().assign(framedata, framedata+framesize);
//    transform(det.frame().begin(),det.frame().end(),
//              det.frame().begin(),
//              bind2nd(minus<float>(),static_cast<float>(frame.offset())));
//    /* in the below how do I make sure that the casting from uint16 to float is done correctly */
//    // copy frame data to cass event, substract the offset from each pixel//
//    //  transform(framedata,framedata + framesize,
//    //            det.frame().begin(),
//    //            bind2nd(minus<float>(),static_cast<float>(frame.offset())));
//    //mark out the first 8 pixels since they store status info, that might mess up the picture
//    fill(det.frame().begin(),det.frame().begin()+8,*(det.frame().begin()+9));
  }
    break;

  default:
    break;
  }
}
