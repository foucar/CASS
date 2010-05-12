//Copyright (C) 2010 Lutz Foucar
//Modified by Nicola Coppola 25.03.2010 introduced ROI(s)

#ifndef _PIXEL_DETECTOR_H_
#define _PIXEL_DETECTOR_H_

#include <QtCore/QMutex>
#include <iostream>
#include <vector>
#include <stdint.h>
#include "cass.h"
#include "serializer.h"
#include "analysis_backend.h"
#include "parameter_backend.h"
#include "serializable.h"

namespace cass
{
  /** Pixel definition
   *
   * This class defines a pixel in a pixel detector.
   *
   * @author Lutz Foucar
   */
  class CASSSHARED_EXPORT Pixel
  {
  public:
    /** constructor. Sets the pixel information*/
    Pixel(uint16_t X, uint16_t Y, pixel_t Z)
      :_x(X),_y(Y),_z(Z)
    {}
    /** default constructor.*/
    Pixel()       {}
    /** default destructor. */
    ~Pixel()      {}

  public:
    //@{
    /** setter */
    uint16_t &x()       {return _x;}
    uint16_t &y()       {return _y;}
    pixel_t  &z()       {return _z;}
    //@}
    //@{
    /** getter */
    uint16_t  x()const  {return _x;}
    uint16_t  y()const  {return _y;}
    pixel_t   z()const  {return _z;}
    //@}

  private:
    uint16_t _x;  //!< x coordinate of the pixel
    uint16_t _y;  //!< y coordinate of the pixel
    pixel_t  _z;  //!< the pixel value
  };



  /** simple ROI definition.
   *
   *  each ROI need the following "attributes": shape, xsize, ysize, xcenter, ycenter
   *  shapes:=circ(==circle),triangle(isosceles),square and  the orientation!!!\n
   *  the orientation is used only in the case of a triangular shape
   *
   *  I think also a "double triangle bottle-like shape could be helpful
   *
   *  xsize,ysize and center are in pixel units.
   *
   * The orientation is used only in the case of a triangular shape.\n
   * @verbatim
  /\           ----         |\           /|
 /  \  ==+1    \  /  ==-1   | \  ==+2   / | == -2
 ----           \/          | /         \ |
                            |/           \|
     @endverbatim
   * if I rotate the plane by -pi/2: -2=>+1 1=>+2 -1=>-2  +2=>-1.
   * Please remember to use the rotated frame wrt standard-natural frame
   * orientation!!
   *
   * @author Nicola Coppola
   */
  class CASSSHARED_EXPORT ROIsimple
  {
//  I do not care it has only public...
//  public:
//  ROIsimple(std::string name,uint32_t xsize,uint32_t ysize,uint32_t xcentre,
//  uint32_t ycentre,int32_t orientation)
  public:
    // the shape name(s)
    // or   enum NameTypes {circ=0, triangle=1, square=2};
    std::string name; //!< the shape of the simpleROI
    uint32_t xsize;   //!< the size(s) along the x axis
    uint32_t ysize;   //!< the size(s) along the y axis
    uint32_t xcentre; //!< the centre(s) along the x axis
    uint32_t ycentre; //!< the centre(s) along the y axis

    int32_t orientation;  //!< the orientation of the triangle
    ~ROIsimple()      {}
    //The following is a "wish", that for the moment is not fullfilled (or needed)!
    //I want something like void ROIload(detROI_t *_detROI);
    //void load(cass::PixelDetector::detROI_t&);
    //void save(cass::PixelDetector::detROI_t *_detROI);
  };



  /** Region of Interest
   *
   * The region of interest is composed from a list of simple ROIsimple
   *
   * @author Nicola Coppola
   */
  class CASSSHARED_EXPORT detROI_ //: public cass::ParameterBackend
  {
  public:
    detROI_() {}
    ~detROI_() {}
    //void load();
    //void save();
  public:
    /** the list of simple roi shapes*/
    std::vector<ROIsimple> _ROI;
    /** Id to which detector the ROI belongs to*/
    uint32_t _detID;
  };

  /** class creating a region of interest.
   *
   * a region of interest is created from a list of simple shapes.
   * @see cass::ROIsimple
   *
   * this function will define/create
   * - a ROI Mask (the values are:\n
   *          =1 pixel is to be used,\n
   *          =2 pixel is declared BAD,\n
   *          =0 pixel is masked\n
   * - a ROI Iterator, which is a list of indices of the frame that are not
   *   masked as uniteresting\n
   * - a ROI Mask Converter, which is the transformed (in the input original
   *   shape) ROI mask for each detector\n
   * - a ROI Iterator Converter, which is the transformed (in the input
   *   original shape) ROI index-pointer-mask for each detector.\n
   * All of these entities are vectors of unsigned integers.
   *
   * I have decided that I do not need to shrink the ROI if I am rebinning, \n
   * the rebinned frame is calculated from the uncorrected one, via the ROI Mask anyway
   *
   * @todo add examples how to iterate over the frame (in principle pnccd_analysis.cpp is full thereof)
   * @author Nicola Coppola
   */
  class CASSSHARED_EXPORT ROI : public cass::ParameterBackend
  {
  public:
    /** default constructor */
    ROI(/*const std::string detectorname*/) {}
    /** default destructor*/
    ~ROI() {}
    /** an region of interest entity */
    typedef detROI_ detROI_t;
    /** the ROI mask for each detector */
    typedef std::vector<uint16_t> ROImask_t;
    /** the ROI index-pointer-mask for each detector */
    typedef std::vector<uint32_t> ROIiterator_t;
    /** the transformed (in the input original shape) ROI mask for each detector */
    typedef std::vector<uint16_t> ROImask_converter_t;
    /** the transformed (in the input original shape) ROI index-pointer-mask for each detector */
    typedef std::vector<uint32_t> ROIiterator_converter_t;
    /** creating the roi entities.
     * This functor will calculate the requested entities.
     * 
     * @param detectorname a unique name for which detector we create the mask
     *        we need this info, to be able to extract the right info from cass.ini
     *        (for the moment this is not needed, as the way that would allow to use it
     *         is clearly NOT nice)
     */
    /** the vector with the ROI(s) "inside" */
    detROI_t                  _detROI;
    ROImask_t                 _ROImask;
    ROIiterator_t             _ROIiterator;
    ROIiterator_t             _ROIiterator_pp;
    ROImask_converter_t       _ROImask_converter;
    ROIiterator_converter_t   _ROIiterator_converter;
    //static detROI_t create(const std::string detectorname)const;
    //static ROImask_t create(const std::string detectorname)const;
    //static ROIiterator_t create(const std::string detectorname)const;
    /** this function creates the region of interest */
    /*static*/ void load(/*detROI_**/);
    void save();

  };




  /** Detector containing a ccd camera image.
   *
   * This class represents a ccd camera image with all its properties.
   *
   * @author Lutz Foucar
   * @author Nicola Coppola
   */
  class CASSSHARED_EXPORT PixelDetector  : public cass::Serializable// : public cass::DeviceBackend
  {
  public:
    PixelDetector():
        Serializable(1),
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
    /** virtual destructor so that one can derive from this class */
    virtual ~PixelDetector() {}
    /** load the settings of the pixeldetector */
    void loadSettings(QSettings);
    /** save the settings of the pixeldetector */
    void saveSettings(QSettings);

  public:
    /** definition of the frame*/
    typedef std::vector<pixel_t> frame_t;
    /** definition of the pixel list.*/
    typedef std::vector<Pixel> pixelList_t;

  public:
    /** serialize the pixeldetector to the Serializer*/
    void serialize(cass::Serializer&);
    /** deserialize the pixeldetector from the Serializer*/
    void deserialize(cass::Serializer&);

  public:
    //@{
    /** setter */
    uint64_t       &integral()               {return _integral;}
    uint64_t       &integral_overthres()     {return _integral_overthres;}
    pixel_t        &maxPixelValue()          {return _maxPixelValue;}
    uint16_t       &columns()                {return _columns;}
    uint16_t       &rows()                   {return _rows;}
    uint16_t       &originalcolumns()        {return _originalcolumns;}
    uint16_t       &originalrows()           {return _originalrows;}
    frame_t        &frame()                  {return _frame;}
    pixelList_t    &pixellist()              {return _pixellist;}
    cass::ROI::detROI_t  &detROI()           {return _detROI;}

    cass::ROI::ROIiterator_t &ROIiterator_pp() {return _ROIiterator_pp;}
    uint32_t       &camaxMagic()             {return _camaxMagic;}
    std::string    &info()                   {return _info;}
    std::string    &timingFilename()         {return _timingFilename;}
    //@}
    //@{
    /** getter */
    uint64_t        integral()const          {return _integral;}
    uint64_t        integral_overthes()const {return _integral_overthres;}
    pixel_t         maxPixelValue()const     {return _maxPixelValue;}
    uint16_t        columns()const           {return _columns;}
    uint16_t        rows()const              {return _rows;}
    uint16_t        originalcolumns()const   {return _originalcolumns;}
    uint16_t        originalrows()const      {return _originalrows;}
    const frame_t  &frame()const             {return _frame;}
    const pixelList_t &pixellist()const      {return _pixellist;}
    const cass::ROI::detROI_t &detROI()const {return _detROI;}
    const cass::ROI::ROIiterator_t &ROIiterator_pp()const{return _ROIiterator_pp;}
    uint32_t          camaxMagic()const      {return _camaxMagic;}
    const std::string info()const            {return _info;}
    const std::string timingFilename()const  {return _timingFilename;}
    //@}

  protected:
    //data//
    /** Linear array of CCD data.
     *
     * how the data is aligned with respect to the laboratory see:
     * - pnCCDConverter
     * - Pulnix -> ask Chris O'Grady
     */
    frame_t         _frame;
    uint16_t        _columns;         //!< Nbr of columns of the frame
    uint16_t        _rows;            //!< Nbr of rows of the frame
    uint16_t        _originalcolumns; //!< Nbr of columns of the frame before rebinning
    uint16_t        _originalrows;    //!< Nbr of rows of the frame before rebinning
    uint16_t        _version;         //!< the version for de/serializing
    bool            _isDarkframe;     //!< status that is set by analysis, derived by cass.ini

    cass::ROI::detROI_t   _detROI;          //!< the vector with the ROI(s) "inside"
    cass::ROI::ROIiterator_t _ROIiterator_pp; //!< the iterator for the postprocessors

    //data specific for pnccd detectors. might be reused for other detectors//
    /** magic camax info, encodes ie. the gain of the ccd*/
    uint32_t _camaxMagic;
    /** infostring of the detector, telling the name of the detector*/
    std::string _info;
    /** filename of the file containing the timing info of the sequenzer*/
    std::string _timingFilename;


    //data that gets calculated in Analysis//
    uint64_t        _integral;            //!< the sum of all pixelvalues
    uint64_t        _integral_overthres;  //!< the sum of all pixelvalues above a certain threshold
    pixel_t         _maxPixelValue;       //!< the highest pixelvalue
    /** the pixellist.
     * The pixel list contains a list of pixel that were found using diffrent
     * kind of analysis
     * @see cass::PNCCD::Analyzer, cass::CCD::Analyzer
     */
    pixelList_t     _pixellist;
  };
}//end namespace cass


inline void cass::PixelDetector::serialize(cass::Serializer &out)
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
