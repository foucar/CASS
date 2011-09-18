//Copyright (C) 2011 Lutz Foucar

/**
 * @file pixeldetector.hpp contains container for simple pixel detector data
 *
 * @author Lutz Foucar
 */


#ifndef _PIXEL_DETECTOR_HPP_
#define _PIXEL_DETECTOR_HPP_

#include <QtCore/QMutex>
#include <iostream>
#include <vector>
#include <map>
#include <stdint.h>
#include "serializer.h"
#include "serializable.h"
#include "device_backend.h"

namespace cass
{
namespace pixeldetector
{
/** Pixel definition
 *
 * Defines a pixel within a pixel detector.
 *
 * @author Lutz Foucar
 */
struct Pixel
{
  /** constructor
     *
     * @param X the x coordinate of the pixel
     * @param Y the y coordinate of the pixel
     * @param Z the value of the pixel
     */
  Pixel(uint16_t X, uint16_t Y, pixel_t Z)
    :_x(X),_y(Y),_z(Z),_used(false)
  {}

  /** default constructor.*/
  Pixel()
    :_x(0),_y(0),_z(0),_used(false)
  {}

  /** x coordinate of the pixel */
  uint16_t _x;

  /** x coordinate of the pixel */
  uint16_t _y;  //!< y coordinate of the pixel

  /** x coordinate of the pixel */
  pixel_t _z;  //!< the pixel value

  /** value to mark pixel any analysis of the pixels */
  uint32_t _used;
};

/** A Hit on a pixel detector.
 *
 * This class defines a hit on a pixel detector that might consist of more
 * one pixel.
 *
 * @author Lutz Foucar
 */
struct Hit
{
  /** default constructor.*/
  Hit()
    :_x(0),_y(0),_z(0),_nbrPixels(0)
  {}

  /** the x coordinate of hit */
  float _x;

  /** the x coordinate of hit */
  float _y;

  /** the value of the hit */
  uint64_t _z;

  /** number of pixels that this hit consists of */
  size_t   _nbrPixels;
};

/** Detector containing a ccd camera image.
 *
 * This class represents a ccd camera image with all its properties.
 *
 * @author Lutz Foucar
 * @author Nicola Coppola
 */
class Detector : public cass::Serializable
{
public:
  /** constructor */
  Detector():
    Serializable(1),
    _columns(0),
    _rows(0)
  {}

  /** constructor
   *
   * constructs the Detector from data contained in the serialzer
   *
   * @param in the serializer object to read the data from
   */
  Detector(SerializerBackend &in)
    : Serializable(1)
  {
    deserialize(in);
  }

public:
  /** definition of the frame*/
  typedef std::vector<pixel_t> frame_t;

public:
  /** serialize the data to the Serializer
   *
   * serializes the frame, the info about colums and rows to the serializer.
   *
   * @param out the serializer object that the data will be serialzed to
   */
  void serialize(SerializerBackend &out)const;

  /** deserialize the data from the Serializer
   *
   * reads the frame the info about columns and rows from the serialzer
   *
   * @return true when de serialization was successfull
   * @param in the serializer object to read the data from
   */
  bool deserialize(SerializerBackend &in);

public:
  //@{
  /** setter */
  uint16_t       &columns()                {return _columns;}
  uint16_t       &rows()                   {return _rows;}
  frame_t        &frame()                  {return _frame;}
  uint32_t       &camaxMagic()             {return _camaxMagic;}
  std::string    &info()                   {return _info;}
  std::string    &timingFilename()         {return _timingFilename;}
  //@}
  //@{
  /** getter */
  uint16_t           columns()const        {return _columns;}
  uint16_t           rows()const           {return _rows;}
  const frame_t     &frame()const          {return _frame;}
  uint32_t           camaxMagic()const     {return _camaxMagic;}
  const std::string &info()const           {return _info;}
  const std::string &timingFilename()const {return _timingFilename;}
  //@}

private:
  /** Linear array of CCD data.
   *
   * how the data is aligned with respect to the laboratory see:
   * - pnCCDConverter
   * - Pulnix -> ask Chris O'Grady
   */
  frame_t _frame;

  /** number of columns of the frame */
  uint16_t _columns;

  /** number of rows of the frame */
  uint16_t _rows;

  /** magic camax info, encodes ie. the gain of the ccd (pnCCD specific)*/
  uint32_t _camaxMagic;

  /** infostring of the detector, telling the name of the detector (pnCCD specific) */
  std::string _info;

  /** filename of the file containing the timing info of the sequenzer  (pnCCD specific)*/
  std::string _timingFilename;
};

/** the device containing pixel detector data
 *
 * @author Lutz Foucar
 */
class Device :public cass::DeviceBackend
{
public:
  /** a map of all pixel detectors available */
  typedef std::map<int32_t,Detector> detectors_t;

  /** constructor.*/
  Device();

public:
  /** instrument setter*/
  detectors_t &dets() {return _detectors;}

  /** instrument getter*/
  const detectors_t &dets()const {return _detectors;}

  /** serialize the data to the Serializer
   *
   * serializes the the key and then the detector to the serializer.
   *
   * @param out the serializer object that the data will be serialzed to
   */
  void serialize(cass::SerializerBackend &out)const;

  /** deserialize the data from the Serializer
   *
   * reads the frame the the key and then the detctor from the serializer
   *
   * @return true when de serialization was successfull
   * @param in the serializer object to read the data from
   */
  bool deserialize(cass::SerializerBackend &in);

private:
  /** Container for all pixel detectors */
  detectors_t _detectors;
};

}//end namespace pixeldetectors
}//end namespace cass


void cass::pixeldetector::Detector::serialize(cass::SerializerBackend &out)const
{
  //the version//
  out.addUint16(_version);
  //serialize the columns rows and then the frame//
  out.addUint16(_columns);
  out.addUint16(_rows);
  for (frame_t::const_iterator it=_frame.begin();it!=_frame.end();++it)
    out.addFloat(*it);
}

bool cass::pixeldetector::Detector::deserialize(cass::SerializerBackend &in)
{
  //check whether the version fits//
  uint16_t ver = in.retrieveUint16();
  if(ver!=_version)
  {
    std::cerr<<"version conflict in pixel detector: "<<ver<<" "<<_version<<std::endl;
    return false;
  }
  //get the number of columns and rows. This defines the size of//
  _columns = in.retrieveUint16();
  _rows    = in.retrieveUint16();
  //make the frame the right size//
  _frame.resize(_columns*_rows);
  //retrieve the frame//
  for (frame_t::iterator it=_frame.begin();it!=_frame.end();++it)
    *it = in.retrieveFloat();
  return true;
}

void cass::pixeldetector::Device::serialize(cass::SerializerBackend &out) const
{
  //the version
  out.addUint16(_version);
  // how many detectors
  out.addUint32(_detectors.size());
  //the detectors
  detectors_t::const_iterator it(_detectors.begin());
  for(; it != _detectors.end(); ++it)
  {
    out.addInt32(it->first);
    it->second.serialize(out);
  }
}

bool cass::pixeldetector::Device::deserialize(cass::SerializerBackend &in)
{
  //check whether the version fits//
  uint16_t ver = in.retrieveUint16();
  if(ver!=_version)
  {
    std::cerr<<"version conflict in pixel detector device: "<<ver
            <<" "<<_version<<std::endl;
    return false;
  }
  //how many detectors//
  uint32_t nbrDetectors (in.retrieveUint32());
  //the detectors//
  for(uint32_t i(0); i < nbrDetectors; ++nbrDetectors)
  {
    int32_t key(in.retrieveInt32());
    Detector det(in);
    _detectors[key] = det;
  }
  return true;
}

#endif
