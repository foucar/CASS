#include "VMIEvent.h"

cass::VMI::VMIEvent::VMIEvent(Pds::Camera::FrameV1 &frame)
{
    _columns        = frame.width();
    _rows           = frame.height();
    _bitsPerPixel   = frame.depth();
    _offset         = frame.offset();

    //copy the frame data to this event somehow...
}
