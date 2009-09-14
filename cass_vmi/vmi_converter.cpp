#include "vmi_converter.h"

#include "pdsdata/camera/FrameV1.hh"
#include "pdsdata/xtc/Xtc.hh"
#include "cassevent.h"
#include "vmi_event.h"


void cass::VMI::Converter::operator()(const Pds::Xtc* xtc, cass::CASSEvent* cassevent)
{
    const Pds::Camera::FrameV1 &frame = *reinterpret_cast<const Pds::Camera::FrameV1*>(xtc->payload());
    VMIEvent &vmievent = cassevent->VMIEvent();

    vmievent.isFilled()     = true;
    vmievent.columns()      = frame.width();
    vmievent.rows()         = frame.height();
    vmievent.bitsPerPixel() = frame.depth();
    vmievent.offset()       = frame.offset();

    //copy the frame data to this event ... initalize the size of the cutevent//
    const uint16_t* framedata = reinterpret_cast<const uint16_t*>(frame.data());
    vmievent.frame().assign(framedata, framedata + (frame.width()*frame.height()));
    vmievent.cutFrame().assign(vmievent.frame().size(),0);
}
