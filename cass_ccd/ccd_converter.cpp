//Copyright (C) 2009,2010, 2011 Lutz Foucar

#include "ccd_converter.h"

#include <algorithm>
#include <iostream>
#include <functional>


#include "pdsdata/camera/FrameV1.hh"
#include "pdsdata/xtc/Xtc.hh"
#include "pdsdata/xtc/DetInfo.hh"
#include "cass_event.h"
#include "ccd_device.h"
#include "pixel_detector.h"

using namespace cass::CCD;
using namespace std;

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
  _pdsTypeList.push_back(Pds::TypeId::Id_Frame);
}

void cass::CCD::Converter::operator()(const Pds::Xtc* xtc, cass::CASSEvent* cassevent)
{
  using namespace std;
  //Get the the detector's device id //
  const Pds::DetInfo& info = *(Pds::DetInfo*)(&xtc->src);
  const size_t detectorId = info.detId();
//  cout<<"CCDConverter::XTCData: DetectorID:"<<info.detId()
//      <<" DeviceId:"<< info.devId()
//      <<" Detector:"<<info.detector()<<"("<<Pds::DetInfo::name(info.detector())<<")"
//      <<" Device:"<< info.device()<<"("<<Pds::DetInfo::name(info.device())<<")"
//      <<endl;
  //retrieve a reference to the frame contained int the xtc//
  const Pds::Camera::FrameV1 &frame =
    *reinterpret_cast<const Pds::Camera::FrameV1*>(xtc->payload());
  //retrieve a pointer to the ccd device we are working on//
  cass::CCD::CCDDevice* dev = dynamic_cast<cass::CCD::CCDDevice*>(cassevent->devices()[cass::CASSEvent::CCD]);
  //if necessary resize the detector container//
  if (detectorId >= dev->detectors()->size())
    dev->detectors()->resize(detectorId+1);
  //retrieve a reference to the commercial ccd detector//
  cass::PixelDetector& det = (*dev->detectors())[detectorId];
//  cout<< dev->detectors()->size()
//      <<" "<<detectorId()
//      <<" "<<frame.width()
//      <<" "<<frame.height()
//      <<" "<<frame.offset()
//      <<endl;

  //copy the values status values from the frame to the detector//
  det.columns()          = frame.width();
  det.rows()             = frame.height();
  det.originalcolumns()  = frame.width();
  det.originalrows()     = frame.height();

  //copy the frame data to this detector and do a type convertion implicitly//
  const uint16_t* framedata (reinterpret_cast<const uint16_t*>(frame.data()));
  const size_t framesize(frame.width()*frame.height());
  //make frame big enough to take all data//
  det.frame().resize(framesize);
  det.frame().assign(framedata, framedata+framesize);
  transform(det.frame().begin(),det.frame().end(),
            det.frame().begin(),
            bind2nd(minus<float>(),static_cast<float>(frame.offset())));
  /* in the below how do I make sure that the casting from uint16 to float is done correctly */
  // copy frame data to cass event, substract the offset from each pixel//
//  transform(framedata,framedata + framesize,
//            det.frame().begin(),
//            bind2nd(minus<float>(),static_cast<float>(frame.offset())));
  //mark out the first 8 pixels since they store status info, that might mess up the picture
  fill(det.frame().begin(),det.frame().begin()+8,*(det.frame().begin()+9));
}
