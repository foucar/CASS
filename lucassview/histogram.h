// Copyright (C) 2010 Lutz Foucar
// Copyright (C) 2010 Jochen Küpper

/**
 * @file histogram.h file contains histogram classes declarations and some
 *                   definitions
 *
 * @author Lutz Foucar
 */

#ifndef HISTOGRAM_H
#define HISTOGRAM_H

#include <cassert>
#include <iostream>
#include <numeric>
#include <stdexcept>
#include <algorithm>
#include <vector>
#include <limits>
#include <cmath>
#include <sstream>

#include <stdint.h>

#include "serializer.h"
#include "serializable.h"

namespace cass
{
  /** Axis properties for histograms
   *
   * This describes the properties of the axis of the histogram. And is
   * de / serializable.
   *
   * @author Lutz Foucar
   */
  class AxisProperty : public Serializable
  {
  public:
    /** Constructor
     *
     * will set the properties in the initializtion list. Will also set the version
     * for the de / serialization.
     *
     * @param nbrBins The Number of Bins the axis contains
     * @param lowerLimit The lower end of the axis
     * @param upperLimit The upper end of the axis
     */
    AxisProperty(size_t nbrBins, float lowerLimit, float upperLimit, std::string title="")
      : Serializable(2),
       _size(nbrBins),
       _low(lowerLimit),
       _up(upperLimit),
       _title(title)
    {}

    /** read axis property from serializer
     *
     * This constructor will create a axis property that has been serialized to the
     * serializer. Will call @see deserialize(Serializer&).
     *
     * @param[in] in The Serializer that we read the histogram from
     */
    AxisProperty(SerializerBackend &in)
      :Serializable(2)
    {
      deserialize(in);
    }

    /** Serialize this class.
     *
     * serializes this to the Serializer
     *
     * @param out The Serializer that we will serialize this to
     * @todo add title to serialization
     */
    void serialize(SerializerBackend& out)const;

    /** Deserialize this class
     *
     * deserializes this from the Serializer
     *
     * @param in The Serializer that we will deserialize this from
     * @todo add title to serialization
     */
    bool deserialize(SerializerBackend& in);

    /** @return title of the axis */
    const std::string& title()const {return _title;}

    /** retrieve number of bins */
    size_t nbrBins() const {return _size;}

    /** Lower limit of axis */
    float lowerLimit() const {return _low;};

    /** Upper limit of axis */
    float upperLimit() const {return _up;}

    /** bin-index for position x */
    size_t bin(float x)const;

    /** position for bin-index idx */
    float position(size_t idx) const { return _low + idx * (_up-_low)/(_size-1); };

    /** convert user distance to distance in histogram memory coordinates*/
    size_t user2hist(float user)const {return static_cast<size_t>(user*_size / (_up-_low));}

    /** convert distance in histogram memory coordinates to user distance */
    float hist2user(size_t hist)const {return hist*(_up-_low)/_size;}

    /** get the possible rebinfactors for this nbr of bins */
    std::vector<size_t> rebinfactors()const;

  protected:
    size_t _size;       //!< the number of bins in this axis
    float _low;         //!< lower end of the axis
    float _up;          //!< upper end of the axis
    std::string _title; //!< the title of the axis
  };






  /** histogram base class.
   *
   * every type of histogram inherits from this. It contains information
   * about the dimesion of the histogram. Also it contains all of the properties
   * and information a histogram has. It does not contain the histograms memory.
   * We are serializable.
   *
   * @author Lutz Foucar
   */
  class HistogramBackend : public Serializable
  {
  protected:
    /** constructor
     *
     * initializing the properties and sets the Serialization version
     *
     * @param dim the Dimension of the histogram ie. 1d,2d
     * @param ver The version for de / serializing.
     */
    HistogramBackend(size_t dim, uint16_t ver)
      :Serializable(ver),
       _dimension(dim),
       _nbrOfFills(0)
    {}

    /** copy constructor */
    HistogramBackend(const cass::HistogramBackend& in)
      : Serializable(in),
        _dimension(in._dimension),
        _axis(in._axis),
        _nbrOfFills(in._nbrOfFills),
        _mime(in._mime),
        _key(in._key)
    {}

  public:
    /** destructor */
    virtual ~HistogramBackend() {}

    /** typedef for more readable code*/
    typedef std::vector<AxisProperty> axis_t;

    /** possible axes */
    enum Axis {xAxis=0, yAxis, zAxis};

    /** possible over/underflow quadrants for 2d histograms */
    enum Quadrant{UpperLeft=0, UpperMiddle, UpperRight,
                  Left,                     Right,
                  LowerLeft  , LowerMiddle, LowerRight};

    /** the over/underflow bin for 1d histograms */
    enum OverUnderFlow{Overflow=0, Underflow};

    /** serialize this object to a serializer.
     *
     * This function is pure virtual since it overwrites the
     * serializables serialize function. Needs to be implemented by the classes
     * inheriting from here
     *
     * @param out The Serializer we serialize this to
     */
    virtual void serialize(SerializerBackend &out)const=0;

    /** serialize this object from a serializer
     *
     * This function is pure virtual since it overwrites the
     * serializables deserialize function. Needs to be implemented by the
     * classes inheriting from this
     *
     * @param in The Serializer we serialize this from
     */
    virtual bool deserialize(SerializerBackend &in)=0;

    //@{
    /** setter */
    size_t      &nbrOfFills()         {return _nbrOfFills;}
    std::string &MimeType()           {return _mime;}
    std::string &key()                {return _key;}
    //@}
    //@{
    /** getter*/
    size_t             nbrOfFills()const  {return _nbrOfFills;}
    size_t             dimension()const   {return _dimension;}
    const axis_t      &axis()const        {return _axis;}
    const std::string &mimeType()const    {return _mime;}
    const std::string &key()const         {return _key;}
    //@}

  protected:
    /** dimension of the histogram */
    size_t _dimension;

    /** the axis of this histogram */
    axis_t _axis;

    /** how many times has this histogram been filled */
    size_t _nbrOfFills;

    /** mime type of the histogram */
    std::string _mime;

    /** the id of the histogram */
    std::string _key;
  };




  /** base class for float histograms.
   *
   * from this all float histograms should inherit
   *
   * @author Lutz Foucar
   */
  class HistogramFloatBase : public HistogramBackend
  {
  public:
    /** constructor
     *
     * @param dim The dimension of the histogram
     * @param memory_size size of the memory, used for special cases
     */
    HistogramFloatBase(size_t dim, size_t memory_size)
      :HistogramBackend(dim,2),
      _memory(memory_size, 0.)
    {}

    /** read histogram from serializer
     *
     * This constructor will create a histogram that has been serialized to the
     * serializer. Will call @see deserialize(Serializer&).
     *
     * @param[in] in The Serializer that we read the histogram from
     */
    HistogramFloatBase(SerializerBackend& in)
      : HistogramBackend(0,2)
    {
      deserialize(in);
    }

    /** typedef describing the type of the values stored in memory*/
    typedef float value_t;

    /** typedef describing the storage type*/
    typedef std::vector<value_t> storage_t;

    /** virtual desctructor, since this a base class*/
    virtual ~HistogramFloatBase(){}

    /** serialize this histogram to the serializer*/
    virtual void serialize(SerializerBackend&)const;

    /** deserialize this histogram from the serializer*/
    virtual bool deserialize(SerializerBackend&);

    /** return const reference to histogram data */
    const storage_t& memory()const {return _memory;}

    /** return reference to histogram data, so that one can manipulate the data */
    storage_t& memory() { return _memory; };

  protected:
    /** histogram storage
     *
     * The memory contains the histogram in range nbins, after that there are some reservered spaces
     * for over/underflow statistics
     */
    storage_t _memory;
  };




  /** "0D Histogram" (scalar value).
   *
   * @author Jochen Kuepper
   */
  class Histogram0DFloat : public HistogramFloatBase
  {
  public:
    /** Create a 0d histogram of a single float */
    explicit Histogram0DFloat()
      : HistogramFloatBase(0, 1)
    {
      _mime = "application/cass0Dhistogram";
    }

    /** Create a 0d histogram with bool value */
    explicit Histogram0DFloat(bool state)
      : HistogramFloatBase(0, 1)
    {
      _mime = "application/cass0Dhistogram";
      _memory[0] = state;
    }

    /** Constructor for reading a histogram from a stream */
    Histogram0DFloat(SerializerBackend &in)
      : HistogramFloatBase(in)
    {}

    /** getter for the 0d value */
    value_t getValue()const { return _memory[0]; };
  };



  /** 1D Histogram
   *
   * can be used for Graphs, ToF's etc.
   *
   * @author Lutz Foucar
   * @author Jochen Küpper
   */
  class Histogram1DFloat : public HistogramFloatBase
  {
  public:
    /** constructor
     *
     * Create a 1d histogram of floats, sets up the axis and mimetype
     *
     * @param nbrXBins, xLow, xUp The properties of the x-axis
     * @param xtitle The title of the x-axis
     */
    Histogram1DFloat(size_t nbrXBins, float xLow, float xUp, std::string xtitle="")
      : HistogramFloatBase(1,nbrXBins+2)
    {
      _axis.push_back(AxisProperty(nbrXBins,xLow,xUp,xtitle));
      _mime = "application/cass1Dhistogram";
    }

    /** read histogram from serializer while creating
     *
     * This constructor will create a histogram that has been serialized to the
     * serializer. Serialization is done in the baseclass.
     *
     * @param[in] in The Serializer that we read the histogram from
     */
    Histogram1DFloat(SerializerBackend &in)
      : HistogramFloatBase(in)
    {}

    /** return number of bins. */
    size_t size() const {return _axis[0].nbrBins();}

    /** Return histogram bin */
    value_t& bin(size_t bin) { return _memory[bin]; };
  };

  /** 2D Histogram
   *
   * can be used for detector images, i.e., pnCCD, VMI CCD, etc...
   *
   * @author Lutz Foucar
   * @author Jochen Küpper
   */
  class Histogram2DFloat : public HistogramFloatBase
  {
  public:
    /** read histogram from serializer.
     * This constructor will create a histogram that has been serialized to the
     * serializer. Serialization is done in the baseclass.
     * @param[in] in The Serializer that we read the histogram from
     */
    Histogram2DFloat(SerializerBackend &in)
      : HistogramFloatBase(in)
    {}

    /** Return histogram bin (row,col)
     *
     * @return the value of the requested row and column
     * @param row the requested row
     * @param col the requestd column
     */
    value_t& bin(size_t row, size_t col)
    {
      return _memory[col + row * _axis[HistogramBackend::xAxis].nbrBins()];
    }
  };
}



inline void cass::AxisProperty::serialize(cass::SerializerBackend &out)const
{
  out.addUint16(_version);
  out.addSizet(_size);
  out.addFloat(_low);
  out.addFloat(_up);
  out.addString(_title);
}

inline bool cass::AxisProperty::deserialize(cass::SerializerBackend &in)
{
  using namespace std;
  //check whether the version fits//
  uint16_t ver = in.retrieveUint16();
  if(ver!=_version)
  {
    stringstream ss;
    ss <<"AxisProperty::deserialize(): Version conflict is  '"<<ver<<"' should be '"<<_version<<"'";
    throw runtime_error(ss.str());
  }
  _size     = in.retrieveSizet();
  _low      = in.retrieveFloat();
  _up       = in.retrieveFloat();
  _title    = in.retrieveString();
  return true;
}

inline void cass::HistogramFloatBase::serialize(cass::SerializerBackend &out)const
{
  out.addUint16(_version);
  out.addSizet(_nbrOfFills);
  out.addSizet(_dimension);
  for (axis_t::const_iterator it=_axis.begin(); it !=_axis.end();++it)
    it->serialize(out);
  size_t size = _memory.size();
  out.addSizet(size);
  for (storage_t::const_iterator it (_memory.begin()); it!=_memory.end();++it)
    out.addFloat(*it);
  out.addString(_key);
}

inline bool cass::HistogramFloatBase::deserialize(cass::SerializerBackend &in)
{
  using namespace std;
  //check whether the version fits//
  uint16_t ver = in.retrieveUint16();
  if(ver!=_version)
  {
    stringstream ss;
    ss <<"HistogramFloatBase::deserialize(): Version conflict is '"<<ver<<"' should be '"<<_version<<"'";
    throw runtime_error(ss.str());
  }
  _nbrOfFills = in.retrieveSizet();
  _dimension = in.retrieveSizet();
  for (size_t i=0; i<_dimension;++i)
    _axis.push_back(AxisProperty(in));
  size_t size = in.retrieveSizet();
  _memory.resize(size);
  for (storage_t::iterator it=_memory.begin(); it!=_memory.end();++it)
    *it = in.retrieveFloat();
  _key = in.retrieveString();
  return true;
}

#endif // HISTOGRAM_H



// Local Variables:
// coding: utf-8
// mode: C++
// c-file-style: "gnu"
// c-file-offsets: ((c . 0) (innamespace . 0))
// fill-column: 100
// End:
