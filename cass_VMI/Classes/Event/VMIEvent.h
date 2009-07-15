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
            Coordinate(uint16_t X, uint16_t Y):x(X),y(Y){}
            uint16_t x;
            uint16_t y;
        };
        class VMIEvent
        {
        public:
           // VMIEvent(Pds::Camera::FrameV1 &frame);	//frame from LCLS

            uint32_t&   integral()                   {return _integral;}
            uint32_t    integral()const              {return _integral;}
            uint16_t&   maxPixelValue()              {return _maxPixelValue;}
            uint16_t    maxPixelValue()const         {return _maxPixelValue;}
            uint16_t&   columns()                    {return _columns;}
            uint16_t    columns()const               {return _columns;}
            uint16_t&   rows()                       {return _rows;}
            uint16_t    rows()const                  {return _rows;}

            const std::vector<uint16_t>& frame()const               {return _frame;}
            std::vector<uint16_t>&       frame()                    {return _frame;}
            std::vector<Coordinate>&     coordinatesOfImpact()      {return _coordinatesOfImpact;}
            std::vector<uint16_t>&       cutFrame()                 {return _cutframe;}

		private:
            //data comming from machine//
            std::vector<uint16_t>   _frame;		//the ccd frame
            uint16_t                _columns;
            uint16_t                _rows;
            uint16_t                _bitsPerPixel;
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
