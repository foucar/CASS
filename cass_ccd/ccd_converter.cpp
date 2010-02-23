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
    const Pds::Camera::FrameV1 &frame = *reinterpret_cast<const Pds::Camera::FrameV1*>(xtc->payload());
    CCDDetector &det = cassevent->devices()[cass::DeviceBackend::Pulnix].detector();

    det.isFilled()         = true;
    det.columns()          = frame.width();
    det.rows()             = frame.height();
    det.originalcolumns()  = frame.width();
    det.originalrows()     = frame.height();
    det.bitsPerPixel()     = frame.depth();
    det.offset()           = frame.offset();

    //copy the frame data to this event ... initalize the size of the cutevent//
    const uint16_t* framedata = reinterpret_cast<const uint16_t*>(frame.data());
    det.frame().assign(framedata, framedata + (frame.width()*frame.height()));

    //make the cutframe as big as the vmievent, but fill with 0//
    vmievent.cutFrame().assign(vmievent.frame().size(),0);
}
