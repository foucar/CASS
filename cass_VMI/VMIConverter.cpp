#include "VMIConverter.h"


void cass::VMI::Converter::operator()(const Pds::Camera::FrameV1& frame, VMIEvent& vmievent)
{
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
