#ifndef _CCD_DETECTOR_H_
#define _CCD_DETECTOR_H_

#include <vector>
#include <stdint.h>
#include "cass.h"

namespace cass
{
  class CASSSHARED_EXPORT Pixel
  {
  public:
    Pixel(uint16_t X, uint16_t Y, Pixel_t Z)
        :x(X),y(Y),z(Z)
    {
    }
    Pixel()       {}
    ~Pixel()      {}
  public:
    uint16_t &x()       {return _x;}
    uint16_t  x()const  {return _x;}
    uint16_t &y()       {return _y;}
    uint16_t  y()const  {return _y;}
    Pixel_t  &z()       {return _z;}
    Pixel_t   z()const  {return _z;}

  private:
    uint16_t _x;        //x coordinate of the coordinate
    uint16_t _y;        //y part of the coordinate
    Pixel_t  _z;        //the pixel value
  };


  class CASSSHARED_EXPORT CCDDetector
  {
  public:
    CCDDetector():
        _columns(0),
        _rows(0),
        _bitsPerPixel(0),
        _offset(0),
        _integral(0),
        _maxPixelValue(0)
    {}
    ~CCDDetector() {}

  public:
    typedef std::vector<Pixel_t> Frame_t;
    typedef std::vector<Pixel> PixelList_t;

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

    const Frame_t   &frame()const            {return _frame;}
    Frame_t         &frame()                 {return _frame;}
    const PixelList_t &pixellist()const      {return _coordinatesOfImpact;}
    PixelList_t       &pixellist()           {return _coordinatesOfImpact;}

  private:
    bool            _isFilled;               //flag to tell whether this event has been filled

    //data comming from machine//
    frame_t         _frame;                  //the ccd frame
    uint16_t        _columns;                //Nbr of columns of the frame
    uint16_t        _rows;                   //Nbr of rows of the frame
    uint16_t        _originalcolumns;        //Nbr of columns of the frame before rebinning
    uint16_t        _originalrows;           //Nbr of rows of the frame before rebinning
    uint16_t        _bitsPerPixel;           //number of bits per pixel
    uint32_t        _offset;                 //the offset (need to find out what this value stands for)

    //data that gets calculated in Analysis//
    uint32_t        _integral;               //the sum of all pixelvalues
    uint16_t        _maxPixelValue;          //the highest pixelvalue
    PixelList_t     _pixellist;              //list of pixels above a given threshold
    frame_t         _cutframe;               //new frame where only mcp is drawn (give maximum radius)
  };
}//end namespace cass

#endif
