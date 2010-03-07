#ifndef _CCD_DETECTOR_H_
#define _CCD_DETECTOR_H_

#include <vector>
#include <stdint.h>
#include "cass.h"
#include "serializer.h"

namespace cass
{
  class CASSSHARED_EXPORT Pixel
  {
  public:
    Pixel(uint16_t X, uint16_t Y, pixel_t Z)
        :_x(X),_y(Y),_z(Z)
    {
    }
    Pixel()       {}
    ~Pixel()      {}

  public:
    uint16_t &x()       {return _x;}
    uint16_t  x()const  {return _x;}
    uint16_t &y()       {return _y;}
    uint16_t  y()const  {return _y;}
    pixel_t  &z()       {return _z;}
    pixel_t   z()const  {return _z;}

  private:
    uint16_t _x;        //x coordinate of the coordinate
    uint16_t _y;        //y part of the coordinate
    pixel_t  _z;        //the pixel value
  };


  class CASSSHARED_EXPORT CCDDetector
  {
  public:
    CCDDetector():
        _columns(0),
        _rows(0),
        _originalcolumns(0),
        _originalrows(0),
        _isDarkframe(false),
        _integral(0),
        _maxPixelValue(0)
    {}
    ~CCDDetector() {}

  public:
    typedef std::vector<pixel_t> frame_t;
    typedef std::vector<Pixel> pixelList_t;

  public:
    void serialize(cass::Serializer&)const;
    void deserialize(cass::Serializer&);



  public:
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

    const frame_t     &frame()const          {return _frame;}
    frame_t           &frame()               {return _frame;}
    const pixelList_t &pixellist()const      {return _pixellist;}
    pixelList_t       &pixellist()           {return _pixellist;}

  private:
    //data//
    frame_t         _frame;                  //the ccd frame
    uint16_t        _columns;                //Nbr of columns of the frame
    uint16_t        _rows;                   //Nbr of rows of the frame
    uint16_t        _originalcolumns;        //Nbr of columns of the frame before rebinning
    uint16_t        _originalrows;           //Nbr of rows of the frame before rebinning

    //status that is set by analysis, derived by cass.ini//
    bool            _isDarkframe;            //is this ccd frame a darkframe

    //data that gets calculated in Analysis//
    uint32_t        _integral;               //the sum of all pixelvalues
    uint16_t        _maxPixelValue;          //the highest pixelvalue
    pixelList_t     _pixellist;              //list of pixels above a given threshold
  };
}//end namespace cass

inline void cass::CCDDetector::serialize(cass::Serializer &out) const
{
//  //serialize the columns rows and then the frame//
//  std::copy( reinterpret_cast<const char*>(&_columns),
//             reinterpret_cast<const char*>(&_columns)+sizeof(uint16_t),
//             out);
//  std::copy( reinterpret_cast<const char*>(&_rows),
//             reinterpret_cast<const char*>(&_rows)+sizeof(uint16_t),
//             out);
//
//  std::copy( reinterpret_cast<const char*>(&_frame[0]),
//             reinterpret_cast<const char*>(&_frame[0])+sizeof(pixel_t)*_frame.size(),
//             out);
}

inline void cass::CCDDetector::deserialize(cass::Serializer &in)
{
//  //get the number of columns and rows. This defines the size of//
//  //the frame that comes after it//
//  std::copy(in,
//            in+sizeof(uint16_t),
//            reinterpret_cast<char*>(&_columns));
//  in += sizeof(uint16_t);
//  std::copy(in,
//            in+sizeof(uint16_t),
//            reinterpret_cast<char*>(&_rows));
//  in += sizeof(uint16_t);
//
//  //make the frame the right size//
//  _frame.resize(_columns*_rows);
//  std::copy(in,
//            in+sizeof(pixel_t)*_frame.size(),
//            reinterpret_cast<char*>(&_frame[0]));
//  in += sizeof(pixel_t)*_frame.size();
}

#endif
