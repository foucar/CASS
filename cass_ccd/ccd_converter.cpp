//Copyright (C) 2009,2010 Lutz Foucar

#include "ccd_converter.h"
#include <iostream>

#include "pdsdata/camera/FrameV1.hh"
#include "pdsdata/xtc/Xtc.hh"
#include "pdsdata/xtc/DetInfo.hh"
#include "cass_event.h"
#include "ccd_device.h"
//#include "ccd_detector.h"
#include "pixel_detector.h"



void cass::CCD::Converter::operator()(const Pds::Xtc* xtc, cass::CASSEvent* cassevent)
{
  //Get the the detector's device id //
  const Pds::DetInfo& info = *(Pds::DetInfo*)(&xtc->src);
  const size_t detectorId = info.devId();
  //retrieve a reference to the frame contained int the xtc//
  const Pds::Camera::FrameV1 &frame =
    *reinterpret_cast<const Pds::Camera::FrameV1*>(xtc->payload());
  //retrieve a pointer to the ccd device we are working on//
  cass::CCD::CCDDevice* dev = dynamic_cast<cass::CCD::CCDDevice*>(cassevent->devices()[cass::CASSEvent::CCD]);
  //if necessary resize the detector container//
  if (detectorId >= dev->detectors()->size())
    dev->detectors()->resize(detectorId+1);
  //retrieve a reference to the pulnix detector//
  cass::PixelDetector& det = dev->detector();

  //copy the values status values from the frame to the detector//
  det.columns()          = frame.width();
  det.rows()             = frame.height();
  det.originalcolumns()  = frame.width();
  det.originalrows()     = frame.height();

  //copy the frame data to this detector and do a type convertion implicitly//
  const uint16_t* framedata = reinterpret_cast<const uint16_t*>(frame.data());
  det.frame().assign(framedata, framedata + (frame.width()*frame.height()));
}
