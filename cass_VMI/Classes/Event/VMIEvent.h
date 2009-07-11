#ifndef _VMIEVENT_H_
#define _VMIEVENT_H_

#include <vector>

#include "pdsdata/camera/FrameV1.hh"

namespace cass
{
    namespace VMI
    {
        class Coordinate
        {
        public:
            uint16_t x;
            uint16_t y;
        };
        class VMIEvent
        {
        public:
            VMIEvent(Pds::Camera::FrameV1 &frame);	//frame from LCLS

            void        integral(uint32_t in)        {_integral=in;}
            uint32_t    integral()const              {return _integral;}
            void        maxPixelValue(uint16_t in)   {_maxPixelValue=in;}
            uint32_t    maxPixelValue()const         {return _maxPixelValue;}
            const std::vector<uint16_t>& frame()const{return _frame;}
            uint32_t    columns()const               {return _columns;}
            uint32_t    rows()const                  {return _rows;}

		private:
            //data comming from machine//
            std::vector<uint16_t>   _frame;		//the ccd frame
			uint32_t                _columns;
            uint32_t                _rows;
            uint32_t                _bitsPerPixel;
            uint32_t                _offset;

            //data that get calculated in Analysis//
            uint32_t                _integral;
            uint16_t                _maxPixelValue;
            std::vector<Coordinate> _coordinatesOfImpact;
            std::vector<uint16_t>   _cutframe;            //new frame where only mcp is drawn (give maximum radius)
        };
    }//end namespace vmi
}//end namespace cass

#endif
