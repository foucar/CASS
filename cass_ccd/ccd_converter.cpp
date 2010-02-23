#include "ccd_converter.h"
#include <iostream>

#include "pdsdata/camera/FrameV1.hh"
#include "pdsdata/xtc/Xtc.hh"
#include "cass_event.h"
#include "ccd_device.h"
#include "ccd_detector.h"



cass::CCD::Converter::Converter()
{
  //this converter should react on a ccd frame//
  _types.push_back(Pds::TypeId::Id_Frame);
}

void cass::CCD::Converter::operator()(const Pds::Xtc* xtc, cass::CASSEvent* cassevent)
{
  //retrieve a reference to the frame contained int the xtc//
  const Pds::Camera::FrameV1 &frame = 
    *reinterpret_cast<const Pds::Camera::FrameV1*>(xtc->payload());
  //retrieve a pointer to the ccd device we are working on//
  cass::CCD::CCDDevice* dev = 
    dynamic_cast<cass::CCD::CCDDevice*>(cassevent->devices()[cass::CASSEvent::Pulnix]);
  //retrieve a reference to the pulnix detector//
  cass::CCDDetector& det = dev->detector();

  //copy the values status values from the frame to the detector//
  det.isFilled()         = true;
  det.columns()          = frame.width();
  det.rows()             = frame.height();
  det.originalcolumns()  = frame.width();
  det.originalrows()     = frame.height();
  det.bitsPerPixel()     = frame.depth();
  det.offset()           = frame.offset();

  //copy the frame data to this detector and do a type convertion implicitly//
  const uint16_t* framedata = reinterpret_cast<const uint16_t*>(frame.data());
  det.frame().assign(framedata, framedata + (frame.width()*frame.height()));
}
