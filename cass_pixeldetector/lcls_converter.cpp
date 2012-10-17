// Copyright (C) 2009, 2010, 2011 Lutz Foucar

/**
 * @file lcls_converter.cpp contains the converters to convert ccd and pnccd data
 *                          to CASSEvent
 *
 * @author Lutz Foucar
 */

#include <iostream>
#include <stdexcept>
#include <algorithm>

#include "pdsdata/xtc/Xtc.hh"
#include "pdsdata/xtc/TypeId.hh"
#include "pdsdata/xtc/DetInfo.hh"
#include "pdsdata/pnCCD/ConfigV1.hh"
#include "pdsdata/pnCCD/ConfigV2.hh"
#include "pdsdata/pnCCD/FrameV1.hh"
#include "pdsdata/camera/FrameV1.hh"

#include "lcls_converter.h"

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
 * This will take the different information from the xtc and transform it to
 * a unique key by having a comparison smaller than applied.
 *
 * Unfortunately the xtc differentiats between the detector and the detectorId
 * the detector is an enum described in Pds::DetInfo::Detector and the detector
 * id is different. The same is true for the device. Both, the detector ids and
 * the device ids are encoded in a single unsigned 32 bit integer. To identify
 * a single detector it is therefore necessary to give all the information.
 *
 * @author Lutz Foucar
 */
class Key
{
public:
  /** constructor
   *
   * will create the physical from the detector and device inputs. The
   * detectorType can just be copied.
   *
   * @param detectorType the type of data contained in the xtc
   * @param det enum of the detector itself
   * @param detId the id of the detector
   * @param dev enum of the device of the data
   * @param devId the id of the device
   */
  Key(const TypeId::Type& detectorType,
      const DetInfo::Detector& det, uint32_t detId,
      const DetInfo::Device& dev, uint32_t devId)
    :_detectorType(detectorType),
     _detector(((det&0xff)<<24) | ((detId&0xff)<<16) | ((dev&0xff)<<8) |(devId&0xff))
  {}

  /** constructor
   *
   * one can retrieve the physical id and the type id directly from the xtc.
   * Therefore a direct assigmnet is possible.
   *
   * @param detectorType the type of data contained in the xtc
   * @param physicalId the id that has the info about device and detector encoded
   */
  Key(const TypeId::Type& detectorType, uint32_t physicalId)
    :_detectorType(detectorType),_detector(physicalId)
  {}

  /** check whether this is less than other
   *
   * will compare for less first the _detectorType. If this is the same it will
   * compare for less the _detector value. This makes sure that the lcls Id is
   * unique.
   *
   * @param other the other key that one compares this key to
   */
  bool operator <(const Key& other) const
  {
    if (_detectorType != other._detectorType)
      return _detectorType < other._detectorType;
    return _detector < other._detector;
  }

private:
  /** type of the detector contained in the xtc */
  uint32_t _detectorType;

  /** the physical value of the detectors data. */
  uint32_t _detector;
};
}//end namespace Id

/** extract the right detector from the CASSEvent
 *
 * check whether the device that contains the detector is present in the CASSevent
 * if not throw and runtime error
 *
 * @return reference to the requested detector
 * @param evt the cassvent containing the detector
 * @param key key of the detector within the map of the device holding the devices
 *
 * @author Lutz Foucar
 */
Detector& retrieveDet(CASSEvent& evt, const Device::detectors_t::key_type& key)
{
  CASSEvent::devices_t &devices(evt.devices());
  CASSEvent::devices_t::iterator devIt(devices.find(CASSEvent::PixelDetectors));
  if(devIt == devices.end())
  {
    throw runtime_error("pixeldetector::retrieveDet: There is no  pixeldetector device within the CASSEvent");
  }
  Device &dev (*dynamic_cast<Device*>(devIt->second));
  return (dev.dets()[key]);
}

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
                 DetInfo::Camp, 0,
                 DetInfo::pnCCD, 0);
  Key FrontPnCCDConfig(TypeId::Id_pnCCDconfig,
                       DetInfo::Camp, 0,
                       DetInfo::pnCCD, 0);
  Key BackPnCCD(TypeId::Id_pnCCDframe,
                DetInfo::Camp, 0,
                DetInfo::pnCCD, 1);
  Key BackPnCCDConfig(TypeId::Id_pnCCDconfig,
                      DetInfo::Camp, 0,
                      DetInfo::pnCCD, 1);
  Key commCCD1(TypeId::Id_Frame,
               DetInfo::AmoBps, 0,
               DetInfo::Opal1000, 0);
  Key commCCD2(TypeId::Id_Frame,
               DetInfo::Camp, 0,
               DetInfo::Opal1000, 0);
  Key commCCD3(TypeId::Id_Frame,
               DetInfo::AmoEndstation, 1,
               DetInfo::Opal1000, 0);
  Key xppCCD(TypeId::Id_Frame,
             DetInfo::XppSb3Pim, 1,
             DetInfo::TM6740, 1);




  _LCLSToCASSId[FrontPnCCD] = 0;
  _LCLSToCASSId[FrontPnCCDConfig] = 0;
  _LCLSToCASSId[BackPnCCD] = 1;  
  _LCLSToCASSId[BackPnCCDConfig] = 1;
  _LCLSToCASSId[commCCD1] = 2;
  _LCLSToCASSId[commCCD2] = 3;
  _LCLSToCASSId[commCCD3] = 4;
  _LCLSToCASSId[xppCCD] = 5;
}

void Converter::operator()(const Pds::Xtc* xtc, CASSEvent* evt)
{
//  cout<< "XTC: '"<<TypeId::name(xtc->contains.id())
//      <<"'("<<xtc->contains.id()
//      <<"), '"<<DetInfo::name(reinterpret_cast<const DetInfo*>(&xtc->src)->detector())
//      <<"'("<<reinterpret_cast<const DetInfo*>(&xtc->src)->detId()
//      <<"), '"<<DetInfo::name(reinterpret_cast<const DetInfo*>(&xtc->src)->device())
//      <<"'("<<reinterpret_cast<const DetInfo*>(&xtc->src)->devId()
//      <<")"<<endl;

  idmap_t::key_type lclskey(xtc->contains.id(), xtc->src.phy());
  idmap_t::iterator lclsmapIt(_LCLSToCASSId.find(lclskey));
  if (lclsmapIt == _LCLSToCASSId.end())
  {
    throw runtime_error("pixeldetector::Converter::operator(): There is no corresponding cass key for '"+
                         string(TypeId::name(xtc->contains.id())) +
                        "'(" + toString(xtc->contains.id()) +
                        "), '" + DetInfo::name(reinterpret_cast<const DetInfo*>(&xtc->src)->detector()) +
                        "'(" + toString(reinterpret_cast<const DetInfo*>(&xtc->src)->detId()) +
                        "), '" + DetInfo::name(reinterpret_cast<const DetInfo*>(&xtc->src)->device()) +
                        "'(" + toString(reinterpret_cast<const DetInfo*>(&xtc->src)->devId()) +
                        ")");
  }
  const idmap_t::mapped_type &casskey(lclsmapIt->second);

  switch( xtc->contains.id() )
  {

  case (Pds::TypeId::Id_pnCCDconfig) :
  {
    uint32_t version = xtc->contains.version();
    if (2 < version)
    {
      throw runtime_error("pixeldetector::Converter::operator: pnCCD Config version" +
                          toString(version) + "is not supported");
    }
    config_t config
        (make_pair(version,shared_ptr<PNCCD::ConfigV2>(new PNCCD::ConfigV2())));
    *(config.second) = *(reinterpret_cast<const Pds::PNCCD::ConfigV2*>(xtc->payload()));
    _pnccdConfigStore[casskey] = config;
  }
    break;


  case (Pds::TypeId::Id_pnCCDframe) :
  {
    map<int32_t,config_t>::const_iterator storeIt(_pnccdConfigStore.find(casskey));
    if(storeIt == _pnccdConfigStore.end())
      break;
    const config_t config(storeIt->second);

    Detector &det(retrieveDet(*evt,casskey));
    const PNCCD::FrameV1* frameSegment
        (reinterpret_cast<const Pds::PNCCD::FrameV1*>(xtc->payload()));

    size_t rowsOfSegment(0);
    size_t columnsOfSegment(0);
    switch(config.first)
    {
    case 1:
      det.rows() = det.columns() = 1024;
      rowsOfSegment = 512;
      columnsOfSegment = 512;
      break;
    case 2:
      det.rows() = config.second->numRows();
      det.columns() = config.second->numChannels();
      det.info().assign(config.second->info());
      det.timingFilename().assign(config.second->timingFName());
      det.camaxMagic() = config.second->camexMagic();
      rowsOfSegment = config.second->numSubmoduleRows();
      columnsOfSegment = config.second->numSubmoduleChannels();
      if(det.rows()>1024||det.columns()>1024||rowsOfSegment>512||columnsOfSegment>512)
      {
        std::cout<<"pnCCDConverter::DataXTC: rows:"<<det.rows()<<std::endl;
        std::cout<<"pnCCDConverter::DataXTC: cols:"<<det.columns()<<std::endl;
        std::cout<<"pnCCDConverter::DataXTC: info:"<<det.info()<<std::endl;
        std::cout<<"pnCCDConverter::DataXTC: tfileName:"<<det.timingFilename()<<std::endl;
        std::cout<<"pnCCDConverter::DataXTC: camaxMagic:"<<std::hex<<det.camaxMagic()<<std::dec<<std::endl;
        std::cout<<"pnCCDConverter::DataXTC: SegRows:"<<rowsOfSegment<<std::endl;
        std::cout<<"pnCCDConverter::DataXTC: SegCols:"<<columnsOfSegment<<std::endl;
        //in this case I am overwriting the values
        rowsOfSegment=512;
        columnsOfSegment=512;
        det.rows() = 1024;
        det.columns() = 1024;
      }
      break;
    default:
      break;
    }
    const size_t sizeOfOneSegment = frameSegment->sizeofData(*(config.second));
    if (sizeOfOneSegment != rowsOfSegment*columnsOfSegment)
    {
      throw runtime_error("pixeldetector::Converter::operator: size of one segment '" +
                          toString(sizeOfOneSegment) +
                          "' is inconsistent with number of rows '" +
                          toString(rowsOfSegment) + "' colums '" +
                          toString(columnsOfSegment) + "' of the segments");
    }
    const size_t NbrOfSegments = config.second->numLinks();
    const size_t FrameSize = sizeOfOneSegment * NbrOfSegments;
    det.frame().resize(FrameSize);

    vector<const uint16_t*> xtcSegmentPointers(NbrOfSegments,0);
    for (size_t i=0; i<NbrOfSegments ;++i)
    {
      //pointer to first data element of segment//
      xtcSegmentPointers[i] = frameSegment->data();
      frameSegment = frameSegment->next(*config.second);
    }
    const uint16_t * tileA = xtcSegmentPointers[0];
    const uint16_t * tileB = xtcSegmentPointers[3];
    const uint16_t * tileC = xtcSegmentPointers[1]+sizeOfOneSegment-1;
    const uint16_t * tileD = xtcSegmentPointers[2]+sizeOfOneSegment-1;

    frame_t::iterator pixel = det.frame().begin();

    for (size_t iRow=0; iRow<rowsOfSegment ;++iRow)
    {
      for (size_t iCol=0; iCol<columnsOfSegment ;++iCol)
        *pixel++ =  *tileA++ & 0x3fff;
      for (size_t iCol=0; iCol<columnsOfSegment ;++iCol)
        *pixel++ =  *tileB++ & 0x3fff;
    }
    for (size_t iRow=0; iRow<rowsOfSegment ;++iRow)
    {
      for (size_t iCol=0; iCol<columnsOfSegment ;++iCol)
        *pixel++ = *tileC-- & 0x3fff;
      for (size_t iCol=0; iCol<columnsOfSegment ;++iCol)
        *pixel++ = *tileD--  & 0x3fff;
    }
  }
    break;

  case (Pds::TypeId::Id_Frame) :
  {
    Detector &det(retrieveDet(*evt,casskey));
    const Camera::FrameV1 &frame
        (*reinterpret_cast<const Camera::FrameV1*>(xtc->payload()));
    det.columns() = frame.width();
    det.rows() = frame.height();
    const uint16_t* framedata (reinterpret_cast<const uint16_t*>(frame.data()));
    const size_t framesize(frame.width()*frame.height());
    det.frame().resize(framesize);
    transform(framedata,framedata + framesize,
              det.frame().begin(),
              bind2nd(minus<float>(),static_cast<float>(frame.offset())));
    fill(det.frame().begin(),det.frame().begin()+8,*(det.frame().begin()+9));
  }
    break;

  default:
    break;
  }
}
