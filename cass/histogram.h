// Copyright (C) 2010-2011 Lutz Foucar
// Copyright (C) 2010 Jochen Küpper
// Copyright (C) 2011 Stephan kassemeyer

/**
 * @file cass/histogram.h file contains histogram classes declarations and some
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

#include <QtCore/QMutex>
#include <QtCore/QReadWriteLock>
#include <QtCore/QWriteLocker>
#include <QtCore/QWaitCondition>
#include <QtGui/QColor>
#include <QtGui/QImage>

#ifdef JPEG_CONVERSION
#include <jpeglib.h>
#endif //JPEG_CONVERSION

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

    /** Constructor.
     *
     * will set the properties in the initializtion list. Will also set the version
     * for the de / serialization.
     *
     * @param nbrBins The Number of Bins the axis contains
     * @param lowerLimit The lower end of the axis
     * @param upperLimit The upper end of the axis
     * @param title The title of the axis
     */
  AxisProperty(size_t nbrBins, float lowerLimit, float upperLimit, std::string title="")
        : Serializable(2),
          _size(nbrBins),
          _low(lowerLimit),
          _up(upperLimit),
          _title(title)
    {}

    /** read axis property from serializer.
     * This constructor will create a axis property that has been serialized to the
     * serializer. Will call @see deserialize(Serializer&).
     * @param[in] in The Serializer that we read the histogram from
     */
    AxisProperty(SerializerBackend &in)
        :Serializable(2)
    {
        deserialize(in);
    }

    /** Serialize this class.
     * serializes this to the Serializer
     * @param out The Serializer that we will serialize this to
     */
    void serialize(SerializerBackend& out)const;

    /** Deserialize this class.
     * deserializes this from the Serializer
     * @param in The Serializer that we will deserialize this from
     */
    bool deserialize(SerializerBackend& in);

    /** @return size (nuber of bins) of axis */
    size_t size() const {return _size;}

    /** set the size of the axis
     *
     * @param size the new size
     * @param fix if true this will change the lower and upper limit to fit the size
     */
    void setSize(size_t size, bool fix=false);

    /** @return title of the axis */
    const std::string& title()const {return _title;}

    /** Convenience function.
     * @deprecated Use size() instead
     * @see size()
     */
    size_t nbrBins() const {return size();}

    /** Lower limit of axis */
    float lowerLimit() const {return _low;}

    /** Upper limit of axis */
    float upperLimit() const {return _up;}

    /** bin-index for position x */
    size_t bin(float x)const;

    /** position for bin-index idx */
    float position(size_t idx) const { return _low + idx * (_up-_low)/(_size-1); }

    /** convert user distance to distance in histogram memory coordinates*/
    size_t user2hist(float user)const {return static_cast<size_t>(user*_size / (_up-_low));}

    /** convert distance in histogram memory coordinates to user distance */
    float hist2user(size_t hist)const {return hist*(_up-_low)/_size;}

    /** get the possible rebinfactors for this nbr of bins */
//    std::vector<size_t> rebinfactors()const;

protected:
    size_t _size;       //!< the number of bins in this axis
    float _low;         //!< lower end of the axis
    float _up;          //!< upper end of the axis
    std::string _title; //!< the title of the axis
};






/** histogram base class.
 * every type of histogram inherits from this. It contains information
 * about the dimesion of the histogram. Also it contains all of the properties
 * and information a histogram has. It does not contain the histograms memory.
 * We are serializable.
 * @author Lutz Foucar
 */
class HistogramBackend
    : public Serializable
{
public:
  /** define a shared pointer of this */
  typedef std::tr1::shared_ptr<HistogramBackend> shared_pointer;

protected:
    /** constructor.
     * initializing the properties and sets the Serialization version
     * @param dim the Dimension of the histogram ie. 1d,2d
     * @param ver The version for de / serializing.
     */
    explicit HistogramBackend(size_t dim, uint16_t ver)
        : Serializable(ver), _dimension(dim), _nbrOfFills(0)
    {}

    /** copy constructor.
     * We need to implement this ourselves, since it is not possitble to copy
     * construct the lock
     */
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

    /** possible axes.
     * convenience type to allow for easier choosing of the axis
     */
    enum Axis {xAxis=0, yAxis, zAxis};

    /** possible over/underflow quadrants.
     * convenience type to allow for easier choosing of the
     * over-/underflow quadrant (2D histograms)
     */
    enum Quadrant{UpperLeft=0, UpperMiddle, UpperRight,
                  Left,                     Right,
                  LowerLeft  , LowerMiddle, LowerRight};

    /** the over/underflow bin.
     * convenience type for easier chosing the over and
     * underflow bins (1D hist).
     */
    enum OverUnderFlow{Overflow=0, Underflow};

    /** serialize this object to a serializer.
     * This function is pure virtual since it overwrites the
     * serializables serialize function. Needs to be implemented by the classes
     * inheriting from here
     * @param out The Serializer we serialize this to
     */
    virtual void serialize(SerializerBackend &out)const=0;

    /** serialize this object from a serializer.
     * This function is pure virtual since it overwrites the
     * serializables deserialize function. Needs to be implemented by the
     * classes inheriting from this
     * @param in The Serializer we serialize this from
     */
    virtual bool deserialize(SerializerBackend &in)=0;

    /** clone the histogram.
     * This function will create a new copy of this on the heap. It will not copy
     * the data in memory but rather set it to 0.
     * Needs to be implemented by the different flavors of histograms.
     */
//    virtual HistogramBackend* clone()const=0;

    /** create a copy of the histogram
     *
     * creates a copy that will copy everything (including data) except for the lock
     */
    virtual HistogramBackend* copyclone()const=0;

    /** returns a shared pointer to a copy of this histogram
     *
     * @return shared pointer to this
     */
    virtual shared_pointer copy_sptr()const=0;

    /** clear the histogram*/
    virtual void clear()=0;

    /** evaluate whether value is non zero
     * @note this should only be implemented by Histogram0DFloat
     */
    virtual bool isTrue() const {assert(false);return false;}


    //@{
    /** setter */
    size_t      &nbrOfFills()         {return _nbrOfFills;}
    axis_t      &axis()               {return _axis;}
    std::string &MimeType()           {return _mime;}
    std::string &key()                {return _key;}
    uint64_t    &id()                 {return _id;}
    //@}
    //@{
    /** getter*/
    size_t             nbrOfFills()const  {return _nbrOfFills;}
    size_t             dimension()const   {return _dimension;}
    const axis_t      &axis()const        {return _axis;}
    const std::string &mimeType()const    {return _mime;}
    const std::string &key()const         {return _key;}
    uint64_t           id()const          {return _id;}
    //@}

public:
    /** Read-write lock for internal memory/data
     *
     * This is a public property of every histogram. It must be locked for all access to the histogram.
     * Mostly, this is done internally, but you have to use it if you directly access memory of a
     * histogram (appropriately for reading and writing).
     */
    mutable QReadWriteLock lock;

#ifdef JPEG_CONVERSION
    /** render histogram into jpeg image
     * @author Stephan Kassemeyer
     *
     */
     virtual std::vector<JOCTET>* jpegImage() const {return new std::vector<JOCTET>;}  // TODO: turn into pure virtual function without implementation in base class?
#endif //JPEG_CONVERSION

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
    /** the id of the event that make the contents */
    uint64_t _id;
};




/** base class for float histograms.
 * from this all float histograms should inherit
 * @author Lutz Foucar
 */
class HistogramFloatBase
    : public HistogramBackend
{
public:
    /** constructor.
     * @param dim The dimension of the histogram
     * @param memory_size size of the memory, used for special cases
     */
    HistogramFloatBase(size_t dim, size_t memory_size)
        :HistogramBackend(dim,3),
        _memory(memory_size, 0.)
    {}

    /** read histogram from serializer.
     * This constructor will create a histogram that has been serialized to the
     * serializer. Will call @see deserialize(Serializer&).
     * @param[in] in The Serializer that we read the histogram from
     */
    HistogramFloatBase(SerializerBackend& in)
        : HistogramBackend(0,3)
    {
        deserialize(in);
    }

    /** clone the histogram.
     * This function will create a new copy of this on the heap. It will not copy
     * the data in memory but rather set it to 0.
     */
//    virtual HistogramBackend* clone()const
//    {
//      return new HistogramFloatBase(dimension(),memory().size());
//    }

    /** clone the histogram completly
     *
     * @return pointer to the copied histogram
     */
    virtual HistogramBackend* copyclone()const
    {
      HistogramFloatBase* copy(new HistogramFloatBase(dimension(),memory().size()));
      copy->memory() = memory();
      copy->nbrOfFills() = nbrOfFills();
      copy->key() = key();
      return copy;
    }

    /** clone the histogram completly
     *
     * @return shared pointer to the copied histogram
     */
    virtual shared_pointer copy_sptr()const
    {
      std::tr1::shared_ptr<HistogramFloatBase> copy(new HistogramFloatBase(dimension(),memory().size()));
      copy->memory() = memory();
      copy->nbrOfFills() = nbrOfFills();
      copy->key() = key();
      return copy;
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
    storage_t& memory() { return _memory; }

    /** Minimum value in current histogram.
     * needs to be implemented in the histogram, since we need to mask out over /  underflow bins
     */
    virtual value_t min() const {return std::numeric_limits<value_t>::min();}

    /** Maximum value in current histogram.
     * needs to be implemented in the histogram, since we need to mask out over /  underflow bins
     */
    virtual value_t max() const {return std::numeric_limits<value_t>::max();}

    /** clear the histogram memory */
    virtual void clear()
    {
      std::fill(_memory.begin(),_memory.end(),0);
      _nbrOfFills = 0;
    }

    /** assignment operator. will copy axis properties and memory */
//    void operator=(const HistogramFloatBase& rhs);


protected:
    /** histogram storage.
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
    {_mime = "application/cass0Dhistogram";}

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

    /** clone the histogram.
     * This function will create a new copy of this on the heap. It will not copy
     * the data in memory but rather set it to 0.
     */
//    virtual HistogramBackend* clone()const
//    {
//      return new Histogram0DFloat();
//    }

    /** clone the histogram completly
     *
     * @return pointer to the cloned histogram
     */
    virtual HistogramBackend* copyclone()const
    {
      Histogram0DFloat* copy(new Histogram0DFloat());
      copy->memory() = memory();
      copy->nbrOfFills() = nbrOfFills();
      copy->key() = key();
      return copy;
    }

    /** clone the histogram completly
     *
     * @return shared pointer to the cloned histogram
     */
    virtual shared_pointer copy_sptr()const
    {
      std::tr1::shared_ptr<Histogram0DFloat> copy(new Histogram0DFloat());
      copy->memory() = memory();
      copy->nbrOfFills() = nbrOfFills();
      copy->key() = key();
      return copy;
    }

    /** fill the 0d histogram with a value */
    void fill(value_t value=0.)
    {
      _memory[0] = value;
      ++_nbrOfFills;
    }

    /** getter for the 0d value*/
    value_t getValue()const { return _memory[0]; }

    /** evaluate whether value is non zero */
    bool isTrue() const
    {
      return (std::abs(_memory[0]) > std::sqrt(std::numeric_limits<value_t>::epsilon()));
    }

    /** Simple assignment ot the single value */
    Histogram0DFloat& operator=(value_t val) { fill(val); return *this; }
};



/** 1D Histogram.
 * can be used for Graphs, ToF's etc.
 * @todo add rebinning
 * @author Lutz Foucar
 * @author Jochen Küpper
 */
class Histogram1DFloat : public HistogramFloatBase
{
public:
    /** constructor.
     * Create a 1d histogram of floats
     * @param nbrXBins, xLow, xUp The properties of the x-axis
     * @param xtitle The title of the x-axis
     */
    Histogram1DFloat(size_t nbrXBins, float xLow, float xUp, std::string xtitle="X")
        : HistogramFloatBase(1,nbrXBins+2)
    {
      //set up the axis
      _axis.push_back(AxisProperty(nbrXBins,xLow,xUp,xtitle));
      _mime = "application/cass1Dhistogram";
    }

    /** read histogram from serializer while creating.
     * This constructor will create a histogram that has been serialized to the
     * serializer. Serialization is done in the baseclass.
     * @param[in] in The Serializer that we read the histogram from
     */
    Histogram1DFloat(SerializerBackend &in)
        : HistogramFloatBase(in)
    {}

    /** constructor.
     *
     * Create a row of a table
     *
     * @param rowSize the number of columns that the row has
     * @param rowTitle The title of the row
     */
    Histogram1DFloat(size_t rowSize, std::string rowTitle="RowName")
        : HistogramFloatBase(1,rowSize)
    {
      //set up the axis
      _axis.push_back(AxisProperty(rowSize,0,rowSize-1,rowTitle));
      _mime = "application/cass1Dhistogram";
    }

    /** constructor.
     *
     * Create an appendable vector
     */
    Histogram1DFloat()
        : HistogramFloatBase(1,0)
    {
      //set up the axis
      _axis.push_back(AxisProperty(0,0,0));
      _mime = "application/cass1Dhistogram";
    }

    /** clone the histogram.
     * This function will create a new copy of this on the heap. It will not copy
     * the data in memory but rather set it to 0.
     */
//    virtual HistogramBackend* clone()const
//    {
//      return new Histogram1DFloat(_axis[xAxis].nbrBins(),
//                                  _axis[xAxis].lowerLimit(),
//                                  _axis[xAxis].upperLimit(),
//                                  _axis[xAxis].title());
//    }

    /** clone the histogram completly
     *
     * @return pointer to the copied histogram
     */
    virtual HistogramBackend* copyclone()const
    {
     Histogram1DFloat*copy(new Histogram1DFloat(_axis[xAxis].nbrBins(),
                                  _axis[xAxis].lowerLimit(),
                                  _axis[xAxis].upperLimit(),
                                  _axis[xAxis].title()));
      copy->memory() = memory();
      copy->nbrOfFills() = nbrOfFills();
      copy->key() = key();
      return copy;
    }

    /** clone the histogram completly
     *
     * @return shared pointer to the copied histogram
     */
    virtual shared_pointer copy_sptr()const
    {
      std::tr1::shared_ptr<Histogram1DFloat> copy
          (new Histogram1DFloat(_axis[xAxis].nbrBins(),
                                _axis[xAxis].lowerLimit(),
                                _axis[xAxis].upperLimit(),
                                _axis[xAxis].title()));
      copy->memory() = memory();
      copy->nbrOfFills() = nbrOfFills();
      copy->key() = key();
      return copy;
    }

    /** resize histogram.
     * will drop all memory and resize axis and memory to the newly requsted size
     */
//    void resize(size_t nbrXBins, float xLow, float xUp);

    /** Add datum to histogram.
     * This operation will lock the memory before attempting to fill the right bin.
     * It will find the right bin for the x-value. If the histogram the bin should not
     * be increased by one, but by a user defined value, then this can be given as the
     * second paramenter.
     * @param x x value that should be histogrammed
     * @param weight value of datum
     */
    void fill(float x, value_t weight=1.);

    /** return number of bins. */
//    size_t size() const {return _axis[0].size();}

    /** Minimum value in current histogram.
     * Avoid checking over / underflow bins.
     */
//    virtual value_t min() const { return *(std::min_element(_memory.begin(), _memory.end()-2)); }

    /** Maximum value in current histogram.
     * Avoid checking over / underflow bins.
     */
//    virtual value_t max() const { return *(std::max_element(_memory.begin(), _memory.end()-2)); }

    /** Return histogram bin that contains x */
//    value_t& operator()(float x) { return _memory[_axis[0].bin(x)]; }

    /** Return histogram bin that contains x */
//    const value_t& operator()(float x) const { return _memory[_axis[0].bin(x)]; }

    /** Return histogram bin */
//    value_t& bin(size_t bin) { return _memory[bin]; }

    /** Return histogram bin */
//    const value_t& bin(size_t bin) const { return _memory[bin]; }

    /** return the bin that the value would be placed into */
    size_t binForVal(float val)const
    {
      const int nxBins(static_cast<const int>(_axis[xAxis].nbrBins()));
      const float xlow(_axis[xAxis].lowerLimit());
      const float xup(_axis[xAxis].upperLimit());
      const int xBin(static_cast<int>( nxBins * (val - xlow) / (xup-xlow)));

      //check whether the fill is in the right range//
      if (0<=xBin && xBin<nxBins)
        return xBin;
      else if (xBin >= nxBins)
        return nxBins+Overflow;
      else if (xBin < 0)
        return nxBins+Underflow;
      else
        throw std::logic_error("Histogram1DFloat::binforval(): This should not happen");
    }

//    /** center of histogram */
//    float center() const
//    {
//        using namespace std;
//        storage_t partial(_memory.size());
//        partial_sum(_memory.begin(), _memory.end(), partial.begin());
//        storage_t::const_iterator center(find_if(partial.begin(), partial.end(), bind2nd(greater_equal<value_t>(), sum()/2)));
//        return _axis[0].position((center - partial.begin()));
//    }

    /** Sum of all values */
//    value_t sum() const {return std::accumulate(_memory.begin(), _memory.end(), 0.f);}

    /** Reduce the 1D histogram to a scalar (integrate/sum all values).
     * @see sum()
     */
//    value_t reduce() const { return sum(); }

//    /** integral of a region in the trace */
//    value_t integral(const std::pair<float,float> &area) const
//    {
//      return std::accumulate(_memory.begin()+_axis[0].bin(std::min(area.first,area.second)),
//                             _memory.begin()+_axis[0].bin(std::max(area.first,area.second)),
//                             0.f);
//    }

    /** append a value to the vector
     *
     * appends the value to the end of the storage and sets the size of the
     * axisproperty accordingly.
     *
     * @param value the value that should be appended to the histograms storage
     */
    void append(const storage_t::value_type &value);

    /** clear the vector
     *
     * clears the storage of this histogram and sets the axisproperties
     * accordingly.
     */
    void clearline();

#ifdef JPEG_CONVERSION
    /** render 1d histogram into jpeg image
     *  @author Stephan kassemeyer
     */
    std::vector<JOCTET>* jpegImage() const;
#endif //JPEG_CONVERSION
};



/** 2D Histogram.
 * can be used for detector images, i.e., pnCCD, VMI CCD, etc...
 * @todo add rebinning
 * @author Lutz Foucar
 * @author Jochen Küpper
 */
class Histogram2DFloat : public HistogramFloatBase
{
public:

    /** create a 2d histogram.
     * This is used when constructing a histogram
     * @param nbrXBins, xLow, xUp The properties of the x-axis
     * @param nbrYBins, yLow, yUp The properties of the y-axis
     * @param xtitle, ytitle The titles of the x and y axis
     */
    Histogram2DFloat(size_t nbrXBins, float xLow, float xUp,
                     size_t nbrYBins, float yLow, float yUp,
                     std::string xtitle="X",std::string ytitle="Y")
        : HistogramFloatBase(2,nbrXBins*nbrYBins+8)  // +8 for under/overflow bits
    {
        //set up the two axis of the 2d hist
        _axis.push_back(AxisProperty(nbrXBins,xLow,xUp,xtitle));
        _axis.push_back(AxisProperty(nbrYBins,yLow,yUp,ytitle));
        // for time beeing, export 2d histograms as image.
        _mime = "image/";
    }

    /** create default histogram.
     *
     * @overload
     *
     * This constructor creates a histogram with integer positions, i.e., it will have positions
     * (0,0), (0,1), ... (0,cols-1), (1,0), ... (rows-1,cols-1)
     * @param rows, cols Number of bins per row and column.
     */
    Histogram2DFloat(size_t rows, size_t cols)
        : HistogramFloatBase(2,rows * cols + 8) // +8 for under/overflow bits
    {
        // set up the two axis of the 2d hist
        _axis.push_back(AxisProperty(rows, 0., float(rows-1.)));
        _axis.push_back(AxisProperty(cols, 0., float(cols-1.)));
        //setMimeType(std::string("application/cass2Dhistogram"));
        _mime = "image/";     // for time beeing, export 2d histograms as image.
    }

    /** create a histogram that stores a list of lists
     *
     * this constructs a 2d histogram that can contain a table, where
     * in x it will store a single list and with increasing y the lists will
     * increase.
     *
     * @param listSize the size of the list in x
     */
    Histogram2DFloat(size_t listSize)
      : HistogramFloatBase(2,0)
    {
      // set up the two axis of the 2d hist
      _axis.push_back(AxisProperty(listSize, 0., float(listSize-1.)));
      _axis.push_back(AxisProperty(0, 0., 0));
      _mime = "table/";     // for time beeing, export 2d histograms as image.
    }

    /** read histogram from serializer.
     * This constructor will create a histogram that has been serialized to the
     * serializer. Serialization is done in the baseclass.
     * @param[in] in The Serializer that we read the histogram from
     */
    Histogram2DFloat(SerializerBackend &in)
        : HistogramFloatBase(in)
    {}

    /** clone the histogram.
     * This function will create a new copy of this on the heap. It will not copy
     * the data in memory but rather set it to 0.
     */
    virtual HistogramBackend* clone()const
    {
      return new Histogram2DFloat(_axis[xAxis].nbrBins(),
                                  _axis[xAxis].lowerLimit(),
                                  _axis[xAxis].upperLimit(),
                                  _axis[yAxis].nbrBins(),
                                  _axis[yAxis].lowerLimit(),
                                  _axis[yAxis].upperLimit(),
                                  _axis[xAxis].title(),
                                  _axis[yAxis].title());
    }

    /** clone the histogram completly
     *
     * @return pointer to the copied histogram
     */
    virtual HistogramBackend* copyclone()const
    {
      Histogram2DFloat* copy(new Histogram2DFloat(_axis[xAxis].nbrBins(),
                                  _axis[xAxis].lowerLimit(),
                                  _axis[xAxis].upperLimit(),
                                  _axis[yAxis].nbrBins(),
                                  _axis[yAxis].lowerLimit(),
                                  _axis[yAxis].upperLimit(),
                                  _axis[xAxis].title(),
                                  _axis[yAxis].title()));
      copy->memory() = memory();
      copy->nbrOfFills() = nbrOfFills();
      copy->key() = key();
      return copy;
    }

    /** clone the histogram completly
     *
     * @return shared pointer to the copied histogram
     */
    virtual shared_pointer copy_sptr()const
    {
      std::tr1::shared_ptr<Histogram2DFloat>
          copy(new Histogram2DFloat(_axis[xAxis].nbrBins(),
                                    _axis[xAxis].lowerLimit(),
                                    _axis[xAxis].upperLimit(),
                                    _axis[yAxis].nbrBins(),
                                    _axis[yAxis].lowerLimit(),
                                    _axis[yAxis].upperLimit(),
                                    _axis[xAxis].title(),
                                    _axis[yAxis].title()));
      copy->memory() = memory();
      copy->nbrOfFills() = nbrOfFills();
      copy->key() = key();
      return copy;
    }

    /** resize histogram.
     * will drop all memory and resize axis and memory to the newly requsted size
     */
//    void resize(size_t nbrXBins, float xLow, float xUp,
//                size_t nbrYBins, float yLow, float yUp);

    /** @return shape of histogram (rows, columns) */
    std::pair<size_t, size_t> shape() const {return std::make_pair(_axis[0].size(), _axis[1].size()); }

    /** Return histogram bin that contains (x,y) */
    value_t& operator()(float x, float y) { return _memory[_axis[0].bin(x) + _axis[1].bin(y) * _axis[0].size()]; }

    /** Return histogram bin (row,col) */
    const value_t& bin(size_t row, size_t col) const { return _memory[col + row * _axis[0].size()]; }

    /** Minimum value in current histogram.
     * Avoid checking over / underflow bins.
     */
//    virtual value_t min() const { return *(std::min_element(_memory.begin(), _memory.end()-8)); }

    /** Maximum value in current histogram.
     * Avoid checking over / underflow bins.
     */
//    virtual value_t max() const { return *(std::max_element(_memory.begin(), _memory.end()-8)); }

    /** Return histogram bin (row,col).
     * @overload
     */
    value_t& bin(size_t row, size_t col) { return _memory[col + row * _axis[0].size()]; }

    /** center of histogram.
     * @todo check and improve
     */
//    std::pair<float, float> center() const { return std::make_pair(reduce(xAxis).center(), reduce(yAxis).center()); }

    /** Add datum to histogram.
     * This operation will lock the memory before attempting to fill the right bin.
     * @param x, y Position of datum
     * @param weight value of datum
     */
    void fill(float x, float y, value_t weight=1.);

    /** Reduce the 2D histogram to a 1D integral along a specified axis.
     * @deprecated use project instead
     * @param axis Reduce along x/rows (axis=xAxis) or y/columns (axis=yAxis)
     */
//    Histogram1DFloat reduce(Axis axis) const;

    /** Reduce the 2D histogram to a 1D integral along a specified axis.
     * @param[in] range the range on the other axis that you want to project
     * @param[in] axis Reduce along x/rows (axis=xAxis) or y/columns (axis=yAxis)
     */
    Histogram1DFloat project(std::pair<float,float> range, Axis axis) const;

    /** Reduce the 2D histogram to a 1D radial average/projection around a specified center.
     * @param[in] center center in coordinates of this 2d histograms memroy
     * @param[in] maxRadius the maximal possible radius in histogram memory coordinates
     */
    Histogram1DFloat radial_project(const std::pair<size_t,size_t> &center,
                                    size_t maxRadius) const;

    /** Reduce the 2D histogram to a 1D radar plot around a specified center.
     * @param[in] center reference to pair x and y center in histogram memory coordinates
     * @param[in] range reference to min and max radii in histogram memory coordinates
     * @param[in] nbrBins the number of bins that the resulting histogram has.
     *                    Range will be 0 ... 360
     */
    Histogram1DFloat radar_plot(const std::pair<size_t,size_t> &center,
                                const std::pair<size_t,size_t> &range,
                                size_t nbrBins) const;

    /** convert histogram to Radius - \f$ \phi \f$ representation
     * @param[in] center reference to x and y cetner in histogram memory coordinates
     * @param[in] maxRadius the maximal possible radius in histogram memory coordinates
     * @param[in] nbrAngleBins the number of bins that the resulting histogram has.
     *                         Range will be 0 ... 360
     */
    Histogram2DFloat convert2RPhi(const std::pair<size_t,size_t> &center,
                                  const size_t maxRadius,
                                  const size_t nbrAngleBins) const;

    /** append row(s) to the histogram that is beeing used as a table
     *
     * append a number of rows to then end of the table. Checks if the size of
     * the appended vector is modulo the x-size of the table.
     *
     * @param rows the row(s) to be added
     */
    void appendRows(const HistogramFloatBase::storage_t &rows);

    /** clear the table */
    void clearTable();

    /** Create a QImage of this histogram.
     *
     * This method does the necessary locking itself!
     * (This is the oly reason this method is not const.)
     *
     * @todo Provide good useable scaling mechanism, i.e., incluing passing it here.
     * @return QImage of this histogram
     */
//    QImage qimage();

#ifdef JPEG_CONVERSION
    /** render 2d histogram into jpeg image
     *  @author Stephan kassemeyer
     */
    std::vector<JOCTET>* jpegImage() const;
#endif //JPEG_CONVERSION
};




//--inlined functions---//

//---------------Axis-------------------------------

inline void cass::AxisProperty::serialize(cass::SerializerBackend &out)const
{
  writeVersion(out);
  //number of bins, lower & upper limit
  out.addSizet(_size);
  out.addFloat(_low);
  out.addFloat(_up);
  out.addString(_title);
}



inline bool cass::AxisProperty::deserialize(cass::SerializerBackend &in)
{
  checkVersion(in);
  //number of bins, lower & upper limit
  _size     = in.retrieveSizet();
  _low      = in.retrieveFloat();
  _up       = in.retrieveFloat();
  _title    = in.retrieveString();
  return true;
}



inline size_t AxisProperty::bin(float pos) const
{
    if(pos < _low)
    {
        std::stringstream ss;
        ss<<"AxisProperty::bin(): Requested position '"<<pos
            <<"' to low, the lowest can be '"<<_low<<"'";
        throw std::out_of_range(ss.str());
    }
    else if(pos > _up)
    {
        std::stringstream ss;
        ss<<"AxisProperty::bin(): Requested position '"<<pos
            <<"' to high, the highest can be '"<<_up<<"'";
        throw std::out_of_range(ss.str());
    }
    size_t index(static_cast<size_t>(_size * ((pos - _low) / (_up - _low))));
    /** @todo check where this function gets called and find out whether the
     *        one needs to check if the size is smaller than the index calculated
     */
    if(size() == index)
        // be forgiving and compensate for rounding errors
        --index;
    assert(index < size());
    return index;
}



//-----------------Base class-----------------------
inline void cass::HistogramFloatBase::serialize(cass::SerializerBackend &out)const
{
  QReadLocker rlock(&lock);
  writeVersion(out);
  //the number of fills//
  out.addSizet(_nbrOfFills);
  //the dimension//
  out.addSizet(_dimension);
  //the event id//
  out.addUint64(_id);
  //the axis properties//
  for (axis_t::const_iterator it=_axis.begin(); it !=_axis.end();++it)
    it->serialize(out);
  //size of the memory//
  size_t size = _memory.size();
  out.addSizet(size);
  //the memory//
  for (storage_t::const_iterator it=_memory.begin(); it!=_memory.end();++it)
    out.addFloat(*it);
  out.addString(_key);
}



inline bool cass::HistogramFloatBase::deserialize(cass::SerializerBackend &in)
{
  checkVersion(in);
  QWriteLocker wlock(&lock);
  //the number of fills//
  _nbrOfFills = in.retrieveSizet();
  //the dimension//
  _dimension = in.retrieveSizet();
  //the event id//
  _id = in.retrieveUint64();
  //initialize axis for all dimensions//
  for (size_t i=0; i<_dimension;++i)
    _axis.push_back(AxisProperty(in));
  //the memory//
  size_t size = in.retrieveSizet();
  _memory.resize(size);
  for (storage_t::iterator it=_memory.begin(); it!=_memory.end();++it)
    *it = in.retrieveFloat();
  _key = in.retrieveString();
  return true;
}


//-----------------1D Hist--------------------------
inline void cass::Histogram1DFloat::fill(float x, float weight)
{
  //calc the bin//
  const int nxBins    = static_cast<const int>(_axis[xAxis].nbrBins());
  const float xlow    = _axis[xAxis].lowerLimit();
  const float xup     = _axis[xAxis].upperLimit();
  const int xBin      = static_cast<int>( nxBins * (x - xlow) / (xup-xlow));

  //check whether the fill is in the right range//
  const bool xInRange = 0<=xBin && xBin<nxBins;
  // if in range fill the memory otherwise figure out whether over of underflow occured//
  if (xInRange)
    _memory[xBin] += weight;
  else if (xBin >= nxBins)
    _memory[nxBins+Overflow] += 1;
  else if (xBin < 0)
    _memory[nxBins+Underflow] += 1;
  //increase the number of fills//
  ++(_nbrOfFills);
}

//-----------------2D Hist--------------------------
inline void Histogram2DFloat::fill(float x, float y, float weight)
{
  // calc the xbin
  const long nxBins   = static_cast<long>(_axis[xAxis].size());
  const float xlow    = _axis[xAxis].lowerLimit();
  const float xup     = _axis[xAxis].upperLimit();
  const long xBin     = static_cast<long>(nxBins * (x - xlow) / (xup-xlow));
  // calc the ybin
  const long nyBins   = static_cast<long>(_axis[yAxis].size());
  const float ylow    = _axis[yAxis].lowerLimit();
  const float yup     = _axis[yAxis].upperLimit();
  const long yBin     = static_cast<long>(nyBins * (y - ylow) / (yup-ylow));
  // fill the memory or the over/underflow quadrant bins
  const long maxSize  = nyBins*nxBins;
  const bool xInRange = 0<=xBin && xBin<nxBins;
  const bool yInRange = 0<=yBin && yBin<nyBins;
  //lock the write operation//

  // if both bin coordinates are in range, fill the memory,//
  //otherwise figure out which quadrant needs to be filled//
  if (xInRange && yInRange)
    _memory[yBin*nxBins + xBin]+= weight;
  if (xBin <0 && yBin <0)
    _memory[maxSize+LowerLeft]+=1;
  else if (xInRange && yBin <0)
    _memory[maxSize+LowerMiddle]+=1;
  else if (xBin >= nxBins && yBin >=nyBins)
    _memory[maxSize+LowerRight]+=1;
  else if (xBin < 0 && yInRange)
    _memory[maxSize+Left]+=1;
  else if (xBin >= nxBins && yInRange)
    _memory[maxSize+Right]+=1;
  else if (xBin < 0 && yBin >= nyBins)
    _memory[maxSize+UpperLeft]+=1;
  else if (xInRange && yBin >= nyBins)
    _memory[maxSize+UpperMiddle]+=1;
  else if (xBin >= nxBins && yBin >= nyBins)
    _memory[maxSize+UpperRight]+=1;
  // increase the number of fills
  ++_nbrOfFills;
}



///** Convert Histogram2DFloat::value_t to uint8_t
//*
//* @author Jochen Küpper
//*/
//class value2pixel
//{
//public:
//
//    value2pixel(Histogram2DFloat::value_t min, Histogram2DFloat::value_t max)
//        : _min(min), _max(max)
//        {};
//
//    uint8_t operator()(Histogram2DFloat::value_t val) {
//        return uint8_t((val - _min) / (_max - _min) * 0xff);
//    };
//
//protected:
//
//    Histogram2DFloat::value_t _min, _max;
//};



//inline QImage Histogram2DFloat::qimage() {
//    QImage qi(shape().first, shape().second, QImage::Format_Indexed8);
//    qi.setColorCount(256);
//    for(unsigned i=0; i<256; ++i)
//        qi.setColor(i, QColor(i, i, i).rgb());
//    qi.fill(0);
//    uint8_t *data(qi.bits());
//    //    value2pixel converter(0,1);
//    value2pixel converter(min(), max());
//    lock.lockForRead();
//    // Subtract 8 to get the size of the buffer excluding over/underflow flags
//    std::transform(_memory.begin(), _memory.end()-8, data, converter);
//    lock.unlock();
//    return qi;
//}


} //end namespace cass


#endif // HISTOGRAM_H

