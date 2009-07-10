#ifndef _VMIEVENT_H_
#define _VMIEVENT_H_

#include <vector>

#include "pdsdata/camera/FrameV1.hh"

namespace cass
{
    namespace VMI
    {
        class VMIEvent
        {
        public:
            VMIEvent(Pds::Camera::FrameV1 &frame);	//frame from LCLS
		private:
            std::vector<uint16_t>   _frame;		//the ccd frame
			uint32_t                _columns;
            uint32_t                _rows;
            uint32_t                _bitsPerPixel;
            uint32_t                _offset;
            uint32_t                _integral;
            uint16_t                _maxPixelValue;
            //std::vector<coordinate> _coordinatesOfImpact;
            //new frame where only mcp is drawn (give maximum radius)
        };
    }//end namespace vmi
}//end namespace cass

#endif
