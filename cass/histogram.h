// Copyright (C) 2010 Lutz Foucar
// Copyright (C) 2010 Jochen Küpper

#ifndef HISTOGRAM_H
#define HISTOGRAM_H

#include <cassert>
#include <iostream>
#include <numeric>
#include <stdexcept>
#include <algorithm>
#include <vector>
#include <limits>

#include <QtCore/QMutex>
#include <QtCore/QReadWriteLock>
#include <QtCore/QWaitCondition>
#include <QtGui/QImage>

#include <stdint.h>

#include "serializer.h"
#include "serializable.h"

namespace cass
{

/** Axis properties for histograms.
 * This describes the properties of the axis of the histogram. And is
 * de / serializable.
 * @author Lutz Foucar
 */
class CASSSHARED_EXPORT AxisProperty : public Serializable
{
public:

    /** Constructor.
     * will set the properties in the initializtion list. Will also set the version
     * for the de / serialization.
     * @param nbrBins The Number of Bins the axis contains
     * @param lowerLimit The lower end of the axis
     * @param upperLimit The upper end of the axis
     */
    AxisProperty(size_t nbrBins, float lowerLimit, float upperLimit)
        : Serializable(1),
        _size(nbrBins), _low(lowerLimit), _up(upperLimit)
    {}

    /** read axis property from serializer.
     * This constructor will create a axis property that has been serialized to the
     * serializer. Will call @see deserialize(Serializer&).
     * @param[in] in The Serializer that we read the histogram from
     */
    AxisProperty(Serializer &in)
        :Serializable(1)
    {
        deserialize(in);
    }

    /** Serialize this class.
     * serializes this to the Serializer
     * @param out The Serializer that we will serialize this to
     */
    void serialize(Serializer& out);

    /** Deserialize this class.
     * deserializes this from the Serializer
     * @param in The Serializer that we will deserialize this from
     */
    void deserialize(Serializer& in);

    /*! @return size (nuber of bins) of axis */
    size_t size() const {return _size;}

    /** Convenience function.
    * @deprecated Use size() instead
    * @see size()
    */
    size_t nbrBins() const {return size();}

    /*! Lower limit of axis */
    float lowerLimit() const {return _low;};

    /*! Upper limit of axis */
    float upperLimit() const {return _up;}

    /*! bin-index for position x */
    size_t bin(float x);

    /*! position for bin-index idx */
    float position(size_t idx) const { return _low + idx * (_up-_low)/(_size-1); };

protected:
    size_t _size; //!< the number of bins in this axis
    float _low;   //!< lower end of the axis
    float _up;    //!< upper end of the axis
};



/** histogram base class.
 * every type of histogram inherits from this. It contains information
 * about the dimesion of the histogram. Also it contains all of the properties
 * and information a histogram has. It does not contain the histograms memory.
 * We are serializable.
 * @author Lutz Foucar
 */
class CASSSHARED_EXPORT HistogramBackend
    : public Serializable
{
protected:

    /** constructor.
     * initializing the properties and sets the Serialization version
     * @param dim the Dimension of the histogram ie. 1d,2d
     * @param ver The version for de / serializing.
     */
    explicit HistogramBackend(size_t dim, uint16_t ver)
        : Serializable(ver),lock(QReadWriteLock::Recursive), _dimension(dim), _nbrOfFills(0)
    {};

public:

    /** destructor.
     * virtual destructor, since it is a baseclass. Does nothing
     */
    virtual ~HistogramBackend() {};

    /** Read-write lock for internal memory/data

    This is a public property of every histogram. It must be locked for all access to the histogram.
    Mostly, this is done internally, but you have to use it if you directly access memory of a
    histogram (appropriately for reading and writing).
    */
    QReadWriteLock lock;

    /** serialize this object to a serializer.
     * This function is pure virtual since it overwrites the
     * serializables serialize function. Needs to be implemented by the classes
     * inheriting from here
     * @param out The Serializer we serialize this to
     */
    virtual void serialize(Serializer &out)=0;

    /** serialize this object from a serializer.
     * This function is pure virtual since it overwrites the
     * serializables deserialize function. Needs to be implemented by the
     * classes inheriting from this
     * @param in The Serializer we serialize this from
     */
    virtual void deserialize(Serializer &in)=0;
    /** clear the histogram*/
    virtual void clear()=0;


    /** typedef for more readable code*/
    typedef std::vector<AxisProperty> axis_t;

    /** setter*/
    size_t  &nbrOfFills()      {return _nbrOfFills;}
    //@{
    /** getter*/
    size_t   nbrOfFills()const {return _nbrOfFills;}
    size_t   dimension()const  {return _dimension;}
    const axis_t  &axis()const {return _axis;}
    //@}

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

protected:

    //!< dimension of the histogram
    size_t    _dimension;
    //!< the axis of this histogram
    axis_t    _axis;
    //!< how many times has this histogram been filled
    size_t    _nbrOfFills;
};




/** base class for float histograms.
 * from this all float histograms should inherit
 * @todo check whether the wait until fill mechanism works
 * @author Lutz Foucar
 */
class CASSSHARED_EXPORT HistogramFloatBase
    : public HistogramBackend
{
public:
    /** typedef describing the type of the values stored in memory*/
    typedef float value_t;
    /** typedef describing the storage type*/
    typedef std::vector<value_t> storage_t;

    /** constructor.
     * @param dim The dimension of the histogram
     * @param memory_size size of the memory, used for special cases
     * @param ver the serialization version
     */
    HistogramFloatBase(size_t dim, size_t memory_size, uint16_t ver)
        :HistogramBackend(dim,ver),
        _memory(memory_size, 0.),
        _fillwhenserialized(false),
        _shouldbefilled(!_fillwhenserialized)
        {}

    /** read histogram from serializer.
     * This constructor will create a histogram that has been serialized to the
     * serializer. Will call @see deserialize(Serializer&).
     * @param[in] in The Serializer that we read the histogram from
     */
    HistogramFloatBase(Serializer& in)
        : HistogramBackend(0,1)
    {
        deserialize(in);
    }
    /** virtual desctructor, since this a base class*/
    virtual ~HistogramFloatBase()      {}

    /** serialize this histogram to the serializer*/
    virtual void serialize(Serializer&);

    /** deserialize this histogram from the serializer*/
    virtual void deserialize(Serializer&);

    /** return const reference to histogram data */
    const storage_t& memory() const {return _memory;}

    /** return reference to histogram data, so that one can manipulate the data */
    storage_t& memory() { return _memory; };

    /** Minimum value in current histogram.
     * needs to be implemented in the histogram, since we need to mask out over /  underflow bins
     */
    virtual value_t min() const {return std::numeric_limits<value_t>::min();}

    /** Maximum value in current histogram.
     * needs to be implemented in the histogram, since we need to mask out over /  underflow bins
     */
    virtual value_t max() const {return std::numeric_limits<value_t>::max();}

    /** return whether the histogram should be filled.
     *this means that someone wants to have the histogram serialized
     */
    bool shouldBeFilled() {return _shouldbefilled;}

    /** clear the histogram memory*/
    virtual void clear() {lock.lockForWrite();std::fill(_memory.begin(),_memory.end(),0);}

    /** notify histogram that is has been filled*/
    void notify(){_fillcondition.wakeAll();}

protected:

    /** histogram storage.
     * The memory contains the histogram in range nbins, after that there are some reservered spaces
     * for over/underflow statistics
     */
    storage_t _memory;

    /** flag to tell whether histogram needs to only be filled when serialized*/
    bool _fillwhenserialized;

    /** flag to signal the postprocessor to fill the histogram*/
    mutable bool _shouldbefilled;

    /** mutex for waiting until we are filled*/
    mutable QMutex _waitMutex;

    /** condition that we will wait on until we were filled by the postprocessor*/
    mutable QWaitCondition _fillcondition;
};




/** "0D Histogram" (scalar value).

@author Jochen Kuepper
*/
class CASSSHARED_EXPORT Histogram0DFloat : public HistogramFloatBase
{
public:

    /*! Create a 0d histogram of a single float */
    explicit Histogram0DFloat()
        : HistogramFloatBase(0, 1, 1)
    {};

    /*! Constructor for reading a histogram from a stream */
    Histogram0DFloat(Serializer &in)
        : HistogramFloatBase(in)
    {};

    void fill(value_t value=0.) {lock.lockForWrite(); _memory[0] = value; lock.unlock(); };

    /*! Simple assignment ot the single value */
    Histogram0DFloat& operator=(value_t val) { fill(val); return *this; };
};



/** 1D Histogram.
 * can be used for Graphs, ToF's etc.
 * @author Lutz Foucar
 * @author Jochen Küpper
 */
class CASSSHARED_EXPORT Histogram1DFloat : public HistogramFloatBase
{
public:

    /** constructor.
     * Create a 1d histogram of floats
     * @param nbrXBins, xLow, xUp The properties of the x-axis
     */
    Histogram1DFloat(size_t nbrXBins, float xLow, float xUp)
        : HistogramFloatBase(1,nbrXBins+2,1)
    {
      //set up the axis
      _axis.push_back(AxisProperty(nbrXBins,xLow,xUp));
    }

    /** read histogram from serializer while creating.
     * This constructor will create a histogram that has been serialized to the
     * serializer. Serialization is done in the baseclass.
     * @param[in] in The Serializer that we read the histogram from
     */
    Histogram1DFloat(Serializer &in)
        : HistogramFloatBase(in)
    {}

    /** Add datum to histogram.
     * This operation will lock the memory before attempting to fill the right bin.
     * It will find the right bin for the x-value. If the histogram the bin should not
     * be increased by one, but by a user defined value, then this can be given as the
     * second paramenter.
     * @param x x value that should be histogrammed
     * @param weight value of datum
     */
    void fill(float x, value_t weight=1.);

    /** Minimum value in current histogram.
     * Avoid checking over / underflow bins.
     */
    virtual value_t min() const { return *(std::min_element(_memory.begin(), _memory.end()-2)); };

    /** Maximum value in current histogram.
     * Avoid checking over / underflow bins.
     */
    virtual value_t max() const { return *(std::max_element(_memory.begin(), _memory.end()-2)); };

    /*! Return histogram bin that contains x */
    value_t& operator()(float x) { return _memory[_axis[0].bin(x)]; };

    /*! Return histogram bin */
    value_t& bin(size_t bin) { return _memory[bin]; };

    /** center of histogram.
    * @todo check and improve
    * @todo check the warning that the compiler at slac give: "/usr/lib/gcc/x86_64-redhat-linux/4.1.2/../../../../include/c++/4.1.2/bits/stl_numeric.h:89: warning: passing 'const float' for argument 1 to '__gnu_cxx::__normal_iterator<_Iterator, _Container> __gnu_cxx::__normal_iterator<_Iterator, _Container>::operator+(const typename std::iterator_traits<_Iter>::difference_type&) const [with _Iterator = float*, _Container = std::vector<float, std::allocator<float> >]'"
    */
    float center() const
    {
        using namespace std;
        storage_t partial(_memory.size());
        accumulate(_memory.begin(), _memory.end(), partial.begin());
        storage_t::const_iterator center(find_if(partial.begin(), partial.end(), bind2nd(greater_equal<value_t>(), sum()/2)));
        return _axis[0].position((center - partial.begin()));
    }

    /** Sum of all values */
    value_t sum() const { value_t sum; std::accumulate(_memory.begin(), _memory.end(), sum); return sum; };

    /** Reduce the 1D histogram to a scalar (integrate/sum all values).
    * @see sum()
    */
    value_t reduce() const { return sum(); };
};



/** 2D Histogram.
 * can be used for detector images, i.e., pnCCD, VMI CCD, etc...
 * @author Lutz Foucar
 * @author Jochen Küpper
 */
class CASSSHARED_EXPORT Histogram2DFloat : public HistogramFloatBase
{
public:

    /** create a 2d histogram.
     * This is used when constructing a histogram
     * @param nbrXBins, xLow, xUp The properties of the x-axis
     * @param nbrYBins, yLow, yUp The properties of the y-axis
     */
    Histogram2DFloat(size_t nbrXBins, float xLow, float xUp,
                     size_t nbrYBins, float yLow, float yUp)
        : HistogramFloatBase(2,nbrXBins*nbrYBins+8,1)
    {
        //set up the two axis of the 2d hist
        _axis.push_back(AxisProperty(nbrXBins,xLow,xUp));
        _axis.push_back(AxisProperty(nbrYBins,yLow,yUp));
    }

    /** create default histogram.

    @overload

    This constructor creates a histogram with integer positions, i.e., it will have positions
    (0,0), (0,1), ... (0,cols-1), (1,0), ... (rows-1,cols-1)
    @param rows, cols Number of bins per row and column.
    */
    Histogram2DFloat(size_t rows, size_t cols)
        : HistogramFloatBase(2,rows * cols + 8,1)
    {
        // set up the two axis of the 2d hist
        _axis.push_back(AxisProperty(rows, 0., float(rows-1.)));
        _axis.push_back(AxisProperty(cols, 0., float(cols-1.)));
    }

    /** read histogram from serializer.
     * This constructor will create a histogram that has been serialized to the
     * serializer. Serialization is done in the baseclass.
     * @param[in] in The Serializer that we read the histogram from
     */
    Histogram2DFloat(Serializer &in)
        : HistogramFloatBase(in)
    {}

    /*! @return shape of histogram (rows, columns) */
    std::pair<size_t, size_t> shape() const {return std::make_pair(_axis[0].size(), _axis[1].size()); };

    /*! Return histogram bin that contains (x,y) */
    value_t& operator()(float x, float y) { return _memory[_axis[0].bin(x) + _axis[1].bin(y) * _axis[0].size()]; };

    /*! Return histogram bin (row,col) */
    const value_t& bin(size_t row, size_t col) const { return _memory[row + col * _axis[0].size()]; };

    /** Minimum value in current histogram.
     * Avoid checking over / underflow bins.
     */
    virtual value_t min() const { return *(std::min_element(_memory.begin(), _memory.end()-8)); };

    /** Maximum value in current histogram.
     * Avoid checking over / underflow bins.
     */
    virtual value_t max() const { return *(std::max_element(_memory.begin(), _memory.end()-8)); };

    /** Return histogram bin (row,col).
    * @overload
    */
    value_t& bin(size_t row, size_t col) { return _memory[row + col * _axis[0].size()]; };

    /** center of histogram.
    * @todo check and improve
    */
    std::pair<float, float> center() const { return std::make_pair(reduce(xAxis).center(), reduce(yAxis).center()); };

    /** Add datum to histogram.
     * This operation will lock the memory before attempting to fill the right bin.
     * @param x, y Position of datum
     * @param weight value of datum
     */
    void fill(float x, float y, value_t weight=1.);

    /** Reduce the 2D histogram to a 1D integral along a specified axis.
    * @param axis Reduce along x/rows (axis=xAxis) or y/columns (axis=yAxis)
    * @note we should rename this to procject, since it more or less does just a
    *       projection of the 2d hist to onto one axis. Then we should also include
    *       a range on the other axis.
    */
    Histogram1DFloat reduce(Axis axis) const;

    /** Create a QImage of this histogram.

    This method does the necessary locking itself!
    (This is the oly reason this method is not const.)

    @todo Provide good useable scaling mechanism, i.e., incluing passing it here.
    @return QImage of this histogram
    */
    QImage qimage();
};




//--inlined functions---//

//---------------Axis-------------------------------

inline void cass::AxisProperty::serialize(cass::Serializer &out)
{
  //the version//
  out.addUint16(_version);

  //number of bins, lower & upper limit
  out.addSizet(_size);
  out.addFloat(_low);
  out.addFloat(_up);
}



inline void cass::AxisProperty::deserialize(cass::Serializer &in)
{
  //check whether the version fits//
  uint16_t ver = in.retrieveUint16();
  if(ver!=_version)
  {
    std::cerr<<"version conflict in Axisproperty of Histogram: "<<ver<<" "<<_version<<std::endl;
    return;
  }
  //number of bins, lower & upper limit
  _size     = in.retrieveSizet();
  _low      = in.retrieveFloat();
  _up       = in.retrieveFloat();
}



inline size_t AxisProperty::bin(float pos)
{
    if(pos < _low)
        throw std::out_of_range("Requested position to low");
    else if(pos > _up)
        throw std::out_of_range("Requested position to high");
    size_t index(static_cast<size_t>(_size * ((pos - _low) / (_up - _low))));
    if(size() == index)
        // be forgiving and compensate for rounding errors
        --index;
    assert(index < size());
    return index;
}



//-----------------Base class-----------------------
inline void cass::HistogramFloatBase::serialize(cass::Serializer &out)
{
  //if we need to wait until the histogram is filled before serialization//
  //wait here and set the flag that this histogram needs to be filled//
  if(_fillwhenserialized)
  {
    //tell that we should be filled//
    _shouldbefilled = true;
    //wait until we have been filled an can proceed//
    _fillcondition.wait(&_waitMutex);
  }
  lock.lockForRead();
  //the version//
  out.addUint16(_version);
  //the dimension//
  out.addSizet(_dimension);
  //the axis properties//
  for (axis_t::iterator it=_axis.begin(); it !=_axis.end();++it)
    it->serialize(out);
  //size of the memory//
  size_t size = _memory.size();
  out.addSizet(size);
  //the memory//
  for (storage_t::const_iterator it=_memory.begin(); it!=_memory.end();++it)
    out.addFloat(*it);
  //we have been filled and serialized so we need to tell that we don't want //
  //to be filled again//
  if(_fillwhenserialized)
    _shouldbefilled=false;
  lock.unlock();
}



inline void cass::HistogramFloatBase::deserialize(cass::Serializer &in)
{
  //check whether the version fits//
  uint16_t ver = in.retrieveUint16();
  if(ver!=_version)
  {
    std::cerr<<"version conflict in histogram: "<<ver<<" "<<_version<<std::endl;
    return;
  }
  lock.lockForWrite();
  //the dimension//
  _dimension = in.retrieveSizet();
  //initialize axis for all dimensions//
  for (size_t i=0; i<_dimension;++i)
    _axis.push_back(AxisProperty(in));
  //the memory//
  size_t size = in.retrieveSizet();
  _memory.resize(size);
  for (storage_t::iterator it=_memory.begin(); it!=_memory.end();++it)
    *it = in.retrieveFloat();
  lock.unlock();
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
  lock.lockForWrite();
  // if in range fill the memory otherwise figure out whether over of underflow occured//
  if (xInRange)
    _memory[xBin] += weight;
  else if (xBin >= nxBins)
    _memory[nxBins+Overflow] += 1;
  else if (xBin < 0)
    _memory[nxBins+Underflow] += 1;
  lock.unlock();
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
//  std::cout
//      <<std::boolalpha
//      <<" X:"<<x
//      <<" NbrXbins"<<nxBins
//      <<" Xbin:"<<xBin
//      <<" XinRange:"<<xInRange
//      <<"    "
//      <<" Y:"<<y
//      <<" NbrYbins:"<<nyBins
//      <<" Ybin:"<<yBin
//      <<" YinRange:"<<yInRange
//      <<std::endl;

  lock.lockForRead();
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
  lock.unlock();
  // increase the number of fills
  ++_nbrOfFills;
}



inline Histogram1DFloat Histogram2DFloat::reduce(Histogram2DFloat::Axis axis) const
{
    Histogram1DFloat hist(_axis[axis].size(), _axis[axis].lowerLimit(), _axis[axis].upperLimit());
    size_t columns(_axis[1].size()), rows(_axis[0].size());
    switch(axis)
    {
    case xAxis: // reduce along rows (integrate rows)
        for(size_t col=0; col<columns; ++col)
            for(size_t row=0; row<rows; ++row)
                hist.bin(col) += bin(row, col);
        break;
    case yAxis: // reduce along columns (integrate rows)
        for(size_t row=0; row<rows; ++row)
            for(size_t col=0; col<columns; ++col)
                hist.bin(row) += bin(row, col);
        break;
    case zAxis:
        throw std::out_of_range("Cannot reduce 2D histogram along 3rd (z) axis!");
    }
}


} //end namespace cass


#endif // HISTOGRAM_H



// Local Variables:
// coding: utf-8
// mode: C++
// c-file-style: "gnu"
// c-file-offsets: ((c . 0) (innamespace . 0))
// fill-column: 100
// End:
