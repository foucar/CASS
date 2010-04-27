//Copyright (C) 2010 lmf
//Modified by Nicola Coppola 25.03.2010 introduced ROI(s)

#ifndef _PIXEL_DETECTOR_H_
#define _PIXEL_DETECTOR_H_

#include <QtCore/QMutex>
#include <iostream>
//#include <map>
//#include <string>
#include <vector>
#include <stdint.h>
#include "cass.h"
#include "serializer.h"
#include "analysis_backend.h"
#include "parameter_backend.h"

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

  class CASSSHARED_EXPORT ROIsimple
  {
    /* inserting the "definition" of a ROI
       each ROI need the following "attributes": shape, xsize, ysize, xcenter, ycenter
       shapes:=circ(==circle),triangle(isosceles),square <=Do I need many squares per frame?
       AAAA there is a problem with a triangular shape... the orientation!!!
       the orientation is used only in the case of a triangular shape

       I think also a "double triangle bottle-like shape could be helpful

       xsize,ysize and center are in pixel units

       Do I need to shrink the ROI if I am rebinning??
    */
    /* I do not care it has only public...
       public:
       ROIsimple(std::string name,uint32_t xsize,uint32_t ysize,uint32_t xcentre,
       uint32_t ycentre,int32_t orientation)
    */
  public:
    // the shape name(s)
    // or   enum NameTypes {circ=0, triangle=1, square=2};
    std::string name;
    uint32_t xsize;// the size(s) along the x axis
    uint32_t ysize;// the size(s) along the y axis
    uint32_t xcentre;// the centre(s) along the x axis
    uint32_t ycentre;// the centre(s) along the y axis
    /*
      the orientation is used only in the case of a triangular shape
      /\           ----         |\           /|
     /  \  ==+1    \  /  ==-1   | \  ==+2   / | == -2
     ----           \/          | /         \ |
                                |/           \|
      if I rotate the plane by -pi/2: -2=>+1 1=>+2 -1=>-2  +2=>-1
      please remember to use the rotated frame wrt standard-natural frame
      orientation!!
    */
    // the orientation
    int32_t orientation;
    ROIsimple()       {}
    ~ROIsimple()      {}
    //I want something like void ROIload(detROI_t *_detROI);
    //void load(cass::PixelDetector::detROI_t&);
    //void save(cass::PixelDetector::detROI_t *_detROI);
  };

  class CASSSHARED_EXPORT detROI_ //: public cass::ParameterBackend
  {
  public:
    detROI_() {}
    ~detROI_() {}
    //void load();
    //void save();
  public:
    std::vector<ROIsimple> _ROI;
    // to which detector the ROI belongs
    uint32_t _detID;
  };

  /** functor creating a region of interest.
   * a region of interest is created from a list of simple shapes.
   * @see cass::ROIsimple
   *
   * this function will create
   * - a ROI Mask
   * - a ROI Iterator, which is a list of indices of the frame that are not
   *   masked as uniteresting
   * - a ROI Mask Converter, which is the transformed (in the input original
   *   shape) ROI mask for each detector
   * - a ROI Iterator Converter, which is the transformed (in the input
   *   original shape) ROI index-pointer-mask for each detector.
   * All of these entities are vectors of unsigned integers.
   *
   * Example usage (deprecated: the create should create everything):
   * @code
   * //create "everything"//
   * //for the first pnccd detector//
   * cass::ROI::roi_t roi = cass::ROI::create("pnCCD01",cass::ROI::ROIIterator);
   * @endcode
   *
   * @note Do I need to shrink the ROI if I am rebinning??
   * @todo explain what the different kind of entities are and what they do.
   * @todo add examples how to iterate over the frame
   * @todo cleanup the documentation make it more clear
   * @todo this, most probably, should be a class and not a struct...
   * @author Nicola Coppola
   */
  class CASSSHARED_EXPORT ROI : public cass::ParameterBackend
  {
  public:
    ROI(const std::string detectorname) {}
    ~ROI() {}
    /** an region of interest entity */
    //typedef std::vector<uint32_t> roi_t;
    //typedef std::vector<detROI_>  detROI_t;//the vector containin of elementary ROI(s)
    typedef detROI_ detROI_t;
    typedef std::vector<uint16_t> ROImask_t;//the ROI mask for each detector//
    typedef std::vector<uint32_t> ROIiterator_t;//the ROI index-pointer-mask for each detector//
    //the transformed (in the input original shape) ROI mask for each detector//
    typedef std::vector<uint16_t> ROImask_converter_t;
    //the transformed (in the input original shape) ROI index-pointer-mask for each detector//
    typedef std::vector<uint32_t> ROIiterator_converter_t;
    /** creating the roi entity.
     * This functor will return the requested entity.
     * @return the requested entity
     * @param detectorname a unique name for which detector we create the mask
     *        we need this info, to be able to extract the right info from cass.ini
     * @param entity tells the functor what type of entity we want it to return
     */
    detROI_t        _detROI;                 //the vector with the ROI(s) "inside"
    //detROI_         _detROI;                 //the vector with the ROI(s) "inside"
    ROImask_t       _ROImask;
    ROIiterator_t   _ROIiterator;
    ROImask_converter_t       _ROImask_converter;
    ROIiterator_converter_t   _ROIiterator_converter;
    //static detROI_t create(const std::string detectorname)const;
    //static ROImask_t create(const std::string detectorname)const;
    //static ROIiterator_t create(const std::string detectorname)const;

    /*static*/ void load(detROI_*);
    void save();

  };


  //CCDDetector -> TwoDDetector??
  class CASSSHARED_EXPORT PixelDetector // : public cass::DeviceBackend
  {
  public:
    PixelDetector():
      _columns(0),
      _rows(0),
      _originalcolumns(0),
      _originalrows(0),
      _version(1),
      _isDarkframe(false),
      _integral(0),
      _integral_overthres(0),
      _maxPixelValue(0)
        {}
    virtual ~PixelDetector() {}

    void loadSettings(QSettings);
    void saveSettings(QSettings);

  public:

    /** Linear array of CCD data

    For definition of frame_t format see
    - pnCCDConverter
    - Pulnix -> ask Chris O'Grady
    */
    typedef std::vector<pixel_t> frame_t;
    
    typedef std::vector<Pixel> pixelList_t;

  public:
    void serialize(cass::Serializer&)const;
    void deserialize(cass::Serializer&);

  public:
    uint64_t        &integral()              {return _integral;}
    uint64_t         integral()const         {return _integral;}
    uint64_t        &integral_overthres()      {return _integral_overthres;}
    uint64_t         integral_overthes()const  {return _integral_overthres;}
    pixel_t        &maxPixelValue()         {return _maxPixelValue;}
    pixel_t         maxPixelValue()const    {return _maxPixelValue;}
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

    const cass::ROI::detROI_t   &detROI()const        {return _detROI;}
    //cass::ROI::detROI_t        &detROI()             {return _detROI;}
    //typedef std::vector<detROI_> detROI_t;
    // //      _detROI detROI;
    //typedef std::vector<uint16_t> ROImask_t;//the ROI mask for each detector//
    //typedef std::vector<uint32_t> ROIiterator_t;//the ROI index-pointer-mask for each detector//

  private:
    //data//
    frame_t         _frame;                  //the ccd frame
    uint16_t        _columns;                //Nbr of columns of the frame
    uint16_t        _rows;                   //Nbr of rows of the frame
    uint16_t        _originalcolumns;        //Nbr of columns of the frame before rebinning
    uint16_t        _originalrows;           //Nbr of rows of the frame before rebinning
    uint16_t        _version;                //the version for de/serializing

    //status that is set by analysis, derived by cass.ini//
    bool            _isDarkframe;            //is this ccd frame a darkframe
    cass::ROI::detROI_t _detROI;             //the vector with the ROI(s) "inside"

    //data that gets calculated in Analysis//
    uint64_t        _integral;               //the sum of all pixelvalues
    uint64_t        _integral_overthres;     //the sum of all pixelvalues above a certain threshold
    pixel_t         _maxPixelValue;          //the highest pixelvalue
    pixelList_t     _pixellist;              //list of pixels above a given threshold
  };
}//end namespace cass


//CCDDetector -> TwoDDetector??
inline void cass::PixelDetector::serialize(cass::Serializer &out) const
{
  //the version//
  out.addUint16(_version);
  //serialize the columns rows and then the frame//
  out.addUint16(_columns);
  out.addUint16(_rows);
  for (frame_t::const_iterator it=_frame.begin();it!=_frame.end();++it)
     out.addFloat(*it);
}

inline void cass::PixelDetector::deserialize(cass::Serializer &in)
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
