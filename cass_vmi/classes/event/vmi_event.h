#ifndef _VMIEVENT_H_
#define _VMIEVENT_H_

#include <vector>
#include <stdint.h>

#ifndef ROOT_Rtypes
#include "Rtypes.h"
#endif

#ifndef ROOT_TObject
#include "TObject.h"
#endif

namespace Pds
{
  namespace Camera
  {
    class FrameV1;
  }
}


namespace cass
{
  namespace VMI
  {
    class Coordinate
    {
    public:
      Coordinate(uint16_t X, uint16_t Y):x(X),y(Y){}
      Coordinate() {}
      ~Coordinate() {}
      uint16_t x;                 //x part of the coordinate
      uint16_t y;                 //y part of the coordinate
      ClassDefNV(Coordinate,1)    //A Coordinate in the Detector
   };


    class VMIEvent
    {
    public:
      VMIEvent():
          _isFilled(false),
          _columns(0),
          _rows(0),
          _bitsPerPixel(0),
          _offset(0),
          _integral(0),
          _maxPixelValue(0)
      {}
      ~VMIEvent() {}

    public:
      typedef std::vector<uint16_t> frame_t;
      typedef std::vector<Coordinate> coordinates_t;

    public:
      bool            &isFilled()              {return _isFilled;}
      bool             isFilled()const         {return _isFilled;}

      uint32_t        &integral()              {return _integral;}
      uint32_t         integral()const         {return _integral;}
      uint16_t        &maxPixelValue()         {return _maxPixelValue;}
      uint16_t         maxPixelValue()const    {return _maxPixelValue;}
      uint16_t        &columns()               {return _columns;}
      uint16_t         columns()const          {return _columns;}
      uint16_t        &rows()                  {return _rows;}
      uint16_t         rows()const             {return _rows;}
      uint16_t        &originalcolumns()       {return _originalcolumns;}
      uint16_t         originalcolumns()const  {return _originalcolumns;}
      uint16_t        &originalrows()          {return _originalrows;}
      uint16_t         originalrows()const     {return _originalrows;}
      uint16_t        &bitsPerPixel()          {return _bitsPerPixel;}
      uint16_t         bitsPerPixel()const     {return _bitsPerPixel;}
      uint32_t        &offset()                {return _offset;}
      uint32_t         offset()const           {return _offset;}

      const frame_t   &frame()const            {return _frame;}
      frame_t         &frame()                 {return _frame;}
      frame_t         &cutFrame()              {return _cutframe;}
      coordinates_t   &coordinatesOfImpact()   {return _coordinatesOfImpact;}

   private:
      bool             _isFilled;              //flag to tell whether this event has been filled

      //data comming from machine//
      frame_t         _frame;                  //the ccd frame
      uint16_t        _columns;                //Nbr of columns of the frame
      uint16_t        _rows;                   //Nbr of rows of the frame
      uint16_t        _originalcolumns;        //Nbr of columns of the frame before rebinning
      uint16_t        _originalrows;           //Nbr of rows of the frame before rebinning
      uint16_t        _bitsPerPixel;           //! number of bits per pixel
      uint32_t        _offset;                 //! the offset (need to find out what this value stands for)

      //data that gets calculated in Analysis//
      uint32_t        _integral;               //the sum of all pixelvalues
      uint16_t        _maxPixelValue;          //the highest pixelvalue
      coordinates_t   _coordinatesOfImpact;    //locations where something hit the detector are stored in this vector
      frame_t         _cutframe;               //new frame where only mcp is drawn (give maximum radius)
      ClassDefNV(VMIEvent,1)
    };
  }//end namespace vmi
}//end namespace cass

#endif
