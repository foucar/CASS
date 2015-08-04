//Copyright (C) 2011, 2015 Lutz Foucar

/**
 * @file pixeldetector.hpp contains container for simple pixel detector data
 *
 * @author Lutz Foucar
 */


#ifndef _PIXEL_DETECTOR_HPP_
#define _PIXEL_DETECTOR_HPP_

#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <stdint.h>

#include "serializable.hpp"
#include "device_backend.hpp"

namespace cass
{
namespace pixeldetector
{

/** Detector containing a ccd camera image.
 *
 * This class represents a ccd camera image with all its properties.
 *
 * @author Lutz Foucar
 * @author Nicola Coppola
 */
class Detector : public Serializable
{
public:
  /** constructor */
  Detector()
    : Serializable(1),
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
/** define a pixel of the pixel detector */
typedef float pixel_t;

/** a frame is a vector of pixels */
typedef std::vector<pixel_t> frame_t;

/** define a shape of an image columnsxrows */
typedef std::pair<size_t,size_t> shape_t;

public:
  /** serialize the data to the Serializer
   *
   * serializes the frame, the info about colums and rows to the serializer.
   *
   * @param out the serializer object that the data will be serialzed to
   */
  void serialize(SerializerBackend &out)const
  {
    writeVersion(out);
    /** write the columns rows and then the frame */
    out.add(_columns);
    out.add(_rows);
    for (frame_t::const_iterator it=_frame.begin();it!=_frame.end();++it)
      out.add(*it);
  }

  /** deserialize the data from the Serializer
   *
   * reads the frame the info about columns and rows from the serialzer
   *
   * @return true when de serialization was successfull
   * @param in the serializer object to read the data from
   */
  bool deserialize(SerializerBackend &in)
  {
    checkVersion(in);
    /**  get the number of columns and rows */
    _columns = in.retrieve<uint16_t>();
    _rows    = in.retrieve<uint16_t>();
    /** clear the frame and read it from the stream */
    _frame.clear();
    for (size_t i(0); i < _columns*_rows;++i)
      _frame.push_back(in.retrieve<frame_t::value_type>());
    return true;
  }

public:
  //@{
  /** setter */
  uint16_t       &columns()                {return _columns;}
  uint16_t       &rows()                   {return _rows;}
  frame_t        &frame()                  {return _frame;}
  uint32_t       &camaxMagic()             {return _camaxMagic;}
  std::string    &info()                   {return _info;}
  std::string    &timingFilename()         {return _timingFilename;}
  uint64_t       &id()                     {return _eventID;}
  //@}
  //@{
  /** getter */
  uint16_t           columns()const        {return _columns;}
  uint16_t           rows()const           {return _rows;}
  shape_t            shape()const          {return std::make_pair(_columns,_rows);}
  const frame_t     &frame()const          {return _frame;}
  uint32_t           camaxMagic()const     {return _camaxMagic;}
  const std::string &info()const           {return _info;}
  const std::string &timingFilename()const {return _timingFilename;}
  uint64_t           id()const             {return _eventID;}
  //@}

private:
  /** Linear array of CCD data.
   *
   * see cass::pixeldetector::Converter for layout
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

  /** the eventid that this detector belongs to (can be used for crosschecks */
  uint64_t _eventID;
};

/** the device containing pixel detector data
 *
 * @author Lutz Foucar
 */
class Device : public DeviceBackend
{
public:
  /** define the detector container */
  typedef std::map<int32_t,Detector> detectors_t;

  /** constructor.*/
  Device()
    : DeviceBackend(1)
  {}

public:
  /** instrument setter*/
  detectors_t &dets() {return _detectors;}

  /** instrument getter*/
  const detectors_t &dets()const {return _detectors;}

  /** serialize the data to the Serializer
   *
   * serializes the key within the map and then the detector to the serializer.
   *
   * @param out the serializer object that the data will be serialzed to
   */
  void serialize(SerializerBackend &out)const
  {
    writeVersion(out);
    /** write how many items in the container there are */
    out.add(static_cast<size_t>(_detectors.size()));
    /** write the keys of the detector and the detector itself */
    detectors_t::const_iterator it(_detectors.begin());
    for(; it != _detectors.end(); ++it)
    {
      out.add(it->first);
      it->second.serialize(out);
    }
  }

  /** deserialize the data from the Serializer
   *
   * reads the frame the the key and then the detctor from the serializer
   *
   * @return true when de serialization was successfull
   * @param in the serializer object to read the data from
   */
  bool deserialize(SerializerBackend &in)
  {
    checkVersion(in);
    /** read the number of detectors */
    size_t nbrDetectors(in.retrieve<size_t>());
    /** read the key of the detector and then deserialze the detector */
    for(uint32_t i(0); i < nbrDetectors; ++nbrDetectors)
    {
      detectors_t::key_type key(in.retrieve<detectors_t::key_type>());
      _detectors[key] = Detector(in);
    }
    return true;
  }

private:
  /** Container for all pixel detectors */
  detectors_t _detectors;
};

}//end namespace pixeldetectors
}//end namespace cass
#endif
