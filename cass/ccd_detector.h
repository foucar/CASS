//Copyright (C) 2010 Lutz Foucar

#ifndef _CCD_DETECTOR_H_
#define _CCD_DETECTOR_H_

#include <iostream>
#include <vector>
#include <stdint.h>
#include "cass.h"
#include "serializer.h"

namespace cass
{
  /** Pixel of a CCD Detector.
   * This class defines a pixel in a ccd detector.
   * @author Lutz Foucar
   */
  class CASSSHARED_EXPORT Pixel
  {
  public:
    Pixel(uint16_t X, uint16_t Y, pixel_t Z)
        :_x(X),_y(Y),_z(Z)
    {}
    Pixel()       {}
    virtual ~Pixel()      {}

  public:
    uint16_t &x()       {return _x;}
    uint16_t  x()const  {return _x;}
    uint16_t &y()       {return _y;}
    uint16_t  y()const  {return _y;}
    pixel_t  &z()       {return _z;}
    pixel_t   z()const  {return _z;}

  private:
    uint16_t _x;        //!< x coordinate of the pixel
    uint16_t _y;        //!< y coordinate of the pixel
    pixel_t  _z;        //!< the pixel value
  };


  /** A CCD Detector.
   * This class will be used by the commercial ccd and the
   * pnCCD Devices.
   * @author Lutz Foucar
   */
  class CASSSHARED_EXPORT CCDDetector
  {
  public:
    CCDDetector():
        _columns(0),
        _rows(0),
        _originalcolumns(0),
        _originalrows(0),
        _version(1),
        _isDarkframe(false),
        _integral(0),
        _maxPixelValue(0)
    {}
    virtual ~CCDDetector() {}

  public:

    /** Linear array of CCD data
     * For definition of frame_t format see
     * - pnCCDConverter
     * - Pulnix -> ask Chris O'Grady
     */
    typedef std::vector<pixel_t> frame_t;

    typedef std::vector<Pixel> pixelList_t;

  public:
    void serialize(cass::SerializerBackend&)const;
    void deserialize(cass::SerializerBackend&);



  public:
    uint32_t        &integral()              {return _integral;}
    uint32_t         integral()const         {return _integral;}
    pixel_t         &maxPixelValue()         {return _maxPixelValue;}
    pixel_t          maxPixelValue()const    {return _maxPixelValue;}
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
    frame_t         _frame;                  //!< the ccd frame
    uint16_t        _columns;                //!< Nbr of columns of the frame
    uint16_t        _rows;                   //!< Nbr of rows of the frame
    uint16_t        _originalcolumns;        //!< Nbr of columns of the frame before rebinning
    uint16_t        _originalrows;           //!< Nbr of rows of the frame before rebinning
    uint16_t        _version;                //!< the version for de/serializing

    //status that is set by analysis, derived by cass.ini//
    bool            _isDarkframe;            //!< is this ccd frame a darkframe

    //data that gets calculated in Analysis//
    uint32_t        _integral;               //!< the sum of all pixelvalues
    pixel_t         _maxPixelValue;          //!< the highest pixelvalue
    pixelList_t     _pixellist;              //!< list of pixels above a given threshold
  };
}//end namespace cass

inline void cass::CCDDetector::serialize(cass::SerializerBackend &out) const
{
  //the version//
  out.addUint16(_version);
  //serialize the columns rows and then the frame//
  out.addUint16(_columns);
  out.addUint16(_rows);
  for (frame_t::const_iterator it=_frame.begin();it!=_frame.end();++it)
     out.addFloat(*it);
}

inline void cass::CCDDetector::deserialize(cass::SerializerBackend &in)
{
  //check whether the version fits//
  uint16_t ver = in.retrieveUint16();
  if(ver!=_version)
  {
    std::cerr<<"version conflict in ccd-detector: "<<ver<<" "<<_version<<std::endl;
    return;
  }
  //get the number of columns and rows. This defines the size of//
  _columns = in.retrieveUint16();
  _rows    = in.retrieveUint16();
  //make the frame the right size//
  _frame.resize(_columns*_rows);
  //retrieve the frame//
  for (frame_t::iterator it=_frame.begin();it!=_frame.end();++it)
     *it = in.retrieveFloat();
}

#endif
