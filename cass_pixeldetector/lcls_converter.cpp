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
#include "pdsdata/cspad/ConfigV1.hh"
#include "pdsdata/cspad/ConfigV2.hh"
#include "pdsdata/cspad/ConfigV3.hh"
#include "pdsdata/cspad/ConfigV4.hh"
#include "pdsdata/cspad/ElementHeader.hh"
#include "pdsdata/cspad/ElementIterator.hh"

#include "lcls_converter.h"

#include "cass_event.h"
#include "pixeldetector.hpp"
#include "log.h"

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

/** copy payload of xtc into a bytearray
 *
 * create a byte array of the right size and copy the payload of the xtc to it.
 *
 * @return vector containing the payload of the xtc
 * @param xtc the xtc whos payload should be copied.
 *
 * @author Lutz Foucar
 */
vector<uint8_t> extractPayload(const Pds::Xtc* xtc)
{
  vector<uint8_t> payload(xtc->sizeofPayload());
  copy(xtc->payload(),xtc->payload()+xtc->sizeofPayload(),payload.begin());

  return payload;
}

/** copy additional info about the pnCCD
 *
 * No info is available, just set default values.
 *
 * @param cfg reference to the configuration
 * @param det reference to the detector that the info will be copied to
 * @param rowsOfSegment the number of rows in each segment
 * @param columnsOfSegment the number of columns in each segment
 *
 * @author Lutz Foucar
 */
void copyAdditionalPnccdInfo(const Pds::PNCCD::ConfigV1& /*cfg*/, Detector &det,
                             size_t & rowsOfSegment, size_t &columnsOfSegment)
{
  det.rows() = det.columns() = 1024;
  rowsOfSegment = 512;
  columnsOfSegment = 512;
}

/** copy additional info about the pnCCD
 *
 * Copy all info that one can get from version 2.
 *
 * @param cfg reference to the configuration
 * @param det reference to the detector that the info will be copied to
 * @param rowsOfSegment the number of rows in each segment
 * @param columnsOfSegment the number of columns in each segment
 *
 * @author Lutz Foucar
 */
void copyAdditionalPnccdInfo(const Pds::PNCCD::ConfigV2& cfg, Detector &det,
                             size_t & rowsOfSegment, size_t &columnsOfSegment)
{
  det.rows() = cfg.numRows();
  det.columns() = cfg.numChannels();
  det.info().assign(cfg.info());
  det.timingFilename().assign(cfg.timingFName());
  det.camaxMagic() = cfg.camexMagic();
  rowsOfSegment = cfg.numSubmoduleRows();
  columnsOfSegment = cfg.numSubmoduleChannels();
  if(det.rows()>1024||det.columns()>1024||rowsOfSegment>512||columnsOfSegment>512)
  {
    Log::add(Log::ERROR,"pnCCDConverter::DataXTC: rows:" + toString(det.rows()));
    Log::add(Log::ERROR,"pnCCDConverter::DataXTC: cols:" + toString(det.columns()));
    Log::add(Log::ERROR,"pnCCDConverter::DataXTC: info:" + det.info());
    Log::add(Log::ERROR,"pnCCDConverter::DataXTC: tfileName:" + det.timingFilename());
    Log::add(Log::ERROR,"pnCCDConverter::DataXTC: camaxMagic:" + toString(det.camaxMagic()));
    Log::add(Log::ERROR,"pnCCDConverter::DataXTC: SegRows:" + toString(rowsOfSegment));
    Log::add(Log::ERROR,"pnCCDConverter::DataXTC: SegCols:" + toString(columnsOfSegment));
  }
}

/** copy the pnCCD frame into the detector
 *
 * @tparam ConfigType the type of configuration
 * @param xtc the raw data of the detector
 * @param cfg the configuration that tells what is where in the configuration
 * @param det the place where the frame data should be copied to
 *
 * @author Lutz Foucar
 */
template <typename ConfigType>
void copyPnCCDFrame(const Pds::Xtc* xtc, const ConfigType& cfg, Detector& det)
{
  const PNCCD::FrameV1* frameSegment
      (reinterpret_cast<const Pds::PNCCD::FrameV1*>(xtc->payload()));

  size_t rowsOfSegment(0);
  size_t columnsOfSegment(0);

  copyAdditionalPnccdInfo(cfg,det,rowsOfSegment,columnsOfSegment);

  const size_t sizeOfOneSegment = frameSegment->sizeofData(cfg);
  if (sizeOfOneSegment != rowsOfSegment*columnsOfSegment)
  {
    throw runtime_error("copyPnCCDFrame: size of one segment '" +
                        toString(sizeOfOneSegment) +
                        "' is inconsistent with number of rows '" +
                        toString(rowsOfSegment) + "' colums '" +
                        toString(columnsOfSegment) + "' of the segments");
  }
  const size_t NbrOfSegments = cfg.numLinks();
  const size_t FrameSize = sizeOfOneSegment * NbrOfSegments;
  det.frame().resize(FrameSize);

  vector<const uint16_t*> xtcSegmentPointers(NbrOfSegments,0);
  for (size_t i=0; i<NbrOfSegments ;++i)
  {
    //pointer to first data element of segment//
    xtcSegmentPointers[i] = frameSegment->data();
    frameSegment = frameSegment->next(cfg);
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

/** copy the cspad frame to the detector
 *
 * @tparam ConfigType the type of configuration
 * @param xtc the raw data of the detector
 * @param cfg the configuration that tells what is where in the configuration
 * @param det the place where the frame data should be copied to
 *
 * @author Lutz Foucar
 */
template <typename ConfigType>
void copyCsPadFrame(const Pds::Xtc* xtc, const ConfigType& cfg, Detector& det)
{
  //Get the frame from the xtc
  const int asic_nx(Pds::CsPad::MaxRowsPerASIC);
  const int asic_ny(Pds::CsPad::ColumnsPerASIC);
  Pds::CsPad::ElementIterator iter(cfg, *xtc);
  const Pds::CsPad::ElementHeader* element;
  /**  2 asics per segment. 8 segments per quadrant. */
  const int pixelsPerQuadrant(2*asic_nx*8*asic_ny);
  const int pixelsPerSegment(2*asic_nx*asic_ny);
  /** 4 quadrants */
  const int FrameSize(4*pixelsPerQuadrant);
  det.frame().resize(FrameSize);
  // loop  over quadrants (elements)
  while( (element=iter.next() ))
  {
    const size_t quad(element->quad());
    pixel_t* rawframe((&det.frame().front()) + quad * pixelsPerQuadrant);
    const Pds::CsPad::Section* section;
    unsigned int section_id;
    while(( section=iter.next(section_id) ))
    {
      const uint16_t* pixels = section->pixel[0];
      for (int ii=0; ii<pixelsPerSegment; ++ii)
      {
        *rawframe = *pixels;
        ++rawframe;
        ++pixels;
      }
    }
  }
  /** all sections above each other */
  det.columns() = 2*asic_nx;
  det.rows() = 4*8*asic_ny;
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
  _pdsTypeList.push_back(TypeId::Id_CspadConfig);
  _pdsTypeList.push_back(TypeId::Id_CspadElement);

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
  Key commCCD4(TypeId::Id_Frame,
               DetInfo::AmoEndstation, 0,
               DetInfo::Opal1000, 0);
  Key xppCCD(TypeId::Id_Frame,
             DetInfo::XppSb3Pim, 1,
             DetInfo::TM6740, 1);

  Key CXIFrontCsPadConfig(TypeId::Id_CspadConfig,
                          DetInfo::CxiDs1, 0,
                          DetInfo::Cspad, 0);
  Key CXIFrontCsPad(TypeId::Id_CspadElement,
                    DetInfo::CxiDs1, 0,
                    DetInfo::Cspad, 0);
  Key CXIBackCsPadConfig(TypeId::Id_CspadConfig,
                         DetInfo::CxiDsd, 0,
                         DetInfo::Cspad, 0);
  Key CXIBackCsPad(TypeId::Id_CspadElement,
                   DetInfo::CxiDsd, 0,
                   DetInfo::Cspad, 0);




  _LCLSToCASSId[FrontPnCCD] = 0;
  _LCLSToCASSId[FrontPnCCDConfig] = 0;
  _LCLSToCASSId[BackPnCCD] = 1;  
  _LCLSToCASSId[BackPnCCDConfig] = 1;
  _LCLSToCASSId[commCCD1] = 2;
  _LCLSToCASSId[commCCD2] = 3;
  _LCLSToCASSId[commCCD3] = 4;
  _LCLSToCASSId[commCCD4] = 6;
  _LCLSToCASSId[xppCCD] = 5;

  _LCLSToCASSId[CXIFrontCsPadConfig] = 7;
  _LCLSToCASSId[CXIFrontCsPad] = 7;
  _LCLSToCASSId[CXIBackCsPadConfig] = 8;
  _LCLSToCASSId[CXIBackCsPad] = 8;

}

void Converter::operator()(const Pds::Xtc* xtc, CASSEvent* evt)
{
  Log::add(Log::DEBUG4, string("XTC: '") +
           TypeId::name(xtc->contains.id()) + "'(" + toString(xtc->contains.id()) +
           "), '" + DetInfo::name(reinterpret_cast<const DetInfo*>(&xtc->src)->detector()) +
           "'(" + toString(reinterpret_cast<const DetInfo*>(&xtc->src)->detId()) +
           "), '" + DetInfo::name(reinterpret_cast<const DetInfo*>(&xtc->src)->device()) +
           "'(" + toString(reinterpret_cast<const DetInfo*>(&xtc->src)->devId()) +
           ")");

  /** skip if there is no corresponding cass key for that xtc */
  idmap_t::key_type lclskey(xtc->contains.id(), xtc->src.phy());
  idmap_t::iterator lclsmapIt(_LCLSToCASSId.find(lclskey));
  if (lclsmapIt == _LCLSToCASSId.end())
  {
    Log::add(Log::ERROR, string("pixeldetector::Converter::operator(): There is no corresponding cass key for : '") +
             TypeId::name(xtc->contains.id()) + "'(" + toString(xtc->contains.id()) +
             "), '" + DetInfo::name(reinterpret_cast<const DetInfo*>(&xtc->src)->detector()) +
             "'(" + toString(reinterpret_cast<const DetInfo*>(&xtc->src)->detId()) +
             "), '" + DetInfo::name(reinterpret_cast<const DetInfo*>(&xtc->src)->device()) +
             "'(" + toString(reinterpret_cast<const DetInfo*>(&xtc->src)->devId()) +
             ")");
    return;
  }
  const idmap_t::mapped_type &casskey(lclsmapIt->second);


  switch( xtc->contains.id() )
  {

  case (Pds::TypeId::Id_CspadConfig) :
  case (Pds::TypeId::Id_pnCCDconfig) :
  {
    if (xtc->contains.id() == Pds::TypeId::Id_pnCCDconfig && 2 < xtc->contains.version())
      throw runtime_error("pixeldetector::Converter::operator: pnCCD Config version" +
                          toString(xtc->contains.version()) + "is not supported");
    if (xtc->contains.id() == Pds::TypeId::Id_CspadConfig && 4 < xtc->contains.version())
      throw runtime_error("pixeldetector::Converter::operator: csPad Config version" +
                          toString(xtc->contains.version()) + "is not supported");
    _configStore[casskey] = make_pair(xtc->contains.version(),extractPayload(xtc));
  }
    break;


  case (Pds::TypeId::Id_pnCCDframe) :
  {
    configStore_t::const_iterator storeIt(_configStore.find(casskey));
    if(storeIt == _configStore.end())
      break;
    const config_t config(storeIt->second);
    Detector &det(retrieveDet(*evt,casskey));
    switch (config.first)
    {
    case 1:
    {
      const Pds::PNCCD::ConfigV1 &cfg
          (reinterpret_cast<const Pds::PNCCD::ConfigV1&>(config.second.front()));
      copyPnCCDFrame(xtc,cfg,det);
    }
      break;

    case 2:
    {
      const Pds::PNCCD::ConfigV2 &cfg
          (reinterpret_cast<const Pds::PNCCD::ConfigV2&>(config.second.front()));
      copyPnCCDFrame(xtc,cfg,det);
    }
      break;

    default:
      throw runtime_error("LCLSConverter:pnCCDFrame: this should not be happening");
    }
  }
    break;

  case (Pds::TypeId::Id_CspadElement) :
  {
    /** get the configuration for this element and return when there is no config */
    configStore_t::const_iterator storeIt(_configStore.find(casskey));
    if(storeIt == _configStore.end())
      break;
    const config_t config(storeIt->second);
    Detector &det(retrieveDet(*evt,casskey));
    switch (config.first)
    {
    case 1:
    {
      const Pds::CsPad::ConfigV1 &cfg
          (reinterpret_cast<const Pds::CsPad::ConfigV1&>(config.second.front()));
      copyCsPadFrame(xtc,cfg,det);
    }
      break;

    case 2:
    {
      const Pds::CsPad::ConfigV2 &cfg
          (reinterpret_cast<const Pds::CsPad::ConfigV2&>(config.second.front()));
      copyCsPadFrame(xtc,cfg,det);
    }
      break;

    case 3:
    {
      const Pds::CsPad::ConfigV3 &cfg
          (reinterpret_cast<const Pds::CsPad::ConfigV3&>(config.second.front()));
      copyCsPadFrame(xtc,cfg,det);
    }
      break;

    case 4:
    {
      const Pds::CsPad::ConfigV4 &cfg
          (reinterpret_cast<const Pds::CsPad::ConfigV4&>(config.second.front()));
      copyCsPadFrame(xtc,cfg,det);
    }
      break;

    default:
      throw runtime_error("LCLSConverter:csPad: this should not be happening");
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
