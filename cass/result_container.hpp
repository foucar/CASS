// Copyright (C) 2011, 2012 Lutz Foucar

/**
 * @file result_container.hpp result container classes
 *
 * @todo make sure it compiles / needs creating appropriate forward declaration
 *       and most likely moving the function definition after the class definition
 *
 * @author Lutz Foucar
 */

#ifndef __RESULTCONTAINER__
#define __RESULTCONTAINER__

#include <vector>
#include <string>
#include <stdexcept>
#include <cmath>
#include <limits>
#include <cassert>
#include <algorithm>
#include <tr1/memory>

#include <QtCore/QReadWriteLock>
#include <QtCore/QWriteLocker>

#include "serializable.h"
#include "serializer.h"

namespace cass
{
//forward declarations
template<typename T>struct Axis;
template <typename T> class Result;


/** add an Axis to a stream
 *
 * @todo add the type info to the stream and check it
 *
 * @tparam T the precision of the axis values
 * @param serializer the serializer to serialize the axis to
 * @param axis the axis to serialize
 *
 * @author Lutz Foucar
 */
template <typename T>
SerializerBackend& operator<<(SerializerBackend& serializer, const Axis<T>& axis)
{
  serializer.addUint16(Axis<T>::serializationVersion);
  serializer.addSizet(axis.nBins);
  serializer.add<T>(axis.low);
  serializer.add<T>(axis.up);
  serializer.addString(axis.title);
  return serializer;
}

/** read an Axis from a stream
 *
 * @todo add the type info to the stream and check it
 *
 * @tparam T the precision of the axis values
 * @param serializer the serializer to serialize the axis to
 * @param axis the axis to serialize
 *
 * @author Lutz Foucar
 */
template <typename T>
SerializerBackend& operator>>(SerializerBackend& serializer, Axis<T>& axis)
{
  uint16_t version(serializer.retrieveUint16());
  assert(version == Axis<T>::serializationVersion);
  axis.nBins = serializer.retrieveSizet();
  axis.low = serializer.retrieve<T>();
  axis.up = serializer.retrieve<T>();
  axis.title = serializer.retrieveString();
  return serializer;
}

/** add a Result to a stream
 *
 * @todo add the type info to the stream and check it
 *
 * @tparam T the precision of the axis values
 * @param serializer the serializer to serialize the axis to
 * @param result the Result to serialize
 *
 * @author Lutz Foucar
 */
template <typename T>
SerializerBackend& operator<<(SerializerBackend& serializer,
                              const Result<T>& result)
{
  typedef Result<T,precision,world2hist> result_t;
  serializer.addUint16(result_t::serializationVersion);
  serializer.addSizet(result._axis.size());
  for (typename result_t::axis_t::const_iterator it=result._axis.begin(), end = result._axis.end(); it != end; ++it)
    serializer << *it;
  serializer.addSizet(result._storage.size());
  for (typename result_t::const_iterator it=result.begin(),end=result.end(); it != end; ++it)
    serializer.add<T>(*it);
  return serializer;
}

/** read a Result from a stream
 *
 * @todo add the type info to the stream and check it
 *
 * @tparam T the precision of the axis values
 * @param serializer the serializer to serialize the axis to
 * @param result the Result to write to
 *
 * @author Lutz Foucar
 */
template <typename T>
SerializerBackend& operator>>(SerializerBackend& serializer,
                              Result<T>& result)
{
  typedef Result<T> result_t;
  uint16_t version(serializer.retrieveUint16());
  assert(version == result_t::serializationVersion);
  result._axis.resize(serializer.retrieveSizet());
  for (typename result_t::axis_t::iterator it=result._axis.begin(), end = result._axis.end(); it != end; ++it)
    serializer >> (*it);
  result._storage.resize(serializer.retrieveSizet());
  for (typename result_t::iterator it=result.begin(),end = result.end(); it != end; ++it)
    *it = serializer.retrieve<result_t::value_t>();
  return serializer;
}


/** an axis of a more than 0 dimensional container
 *
 * @tparam the type in which the bin limits are given
 *
 * @author Lutz Foucar
 */
template<typename T>
struct Axis
{
  /** the serialization version of this class */
  enum {serializationVersion=1};

  /** the presision type of the axis boundaries */
  typedef T precision_t;

  /** Constructor
   *
   * will set the properties in the initializtion list. Will also set the version
   * for the de / serialization.
   *
   * @param nbrBins The Number of Bins the axis contains
   * @param lowerLimit The lower end of the axis
   * @param upperLimit The upper end of the axis
   */
  Axis(size_t nbrBins, precision_t lowerLimit, precision_t upperLimit, std::string title="Axis Title")
    : Serializable(serializationVersion),
      nBins(nbrBins),
      low(lowerLimit),
      up(upperLimit),
      title(title)
  {}

  /** friend for serialization
   *
   * @note make one single instance (called "specialization" in generic terms)
   *       of the template a friend. Because the compiler knows from the
   *       parameter list that the template arguments one does not have to put
   *       those between <...>, so they can be left empty
   */
  friend SerializerBackend& operator<< <> (SerializerBackend&, const Axis<T>&);

  /** friend for deserialization
   *
   * @note make one single instance (called "specialization" in generic terms)
   *       of the template a friend. Because the compiler knows from the
   *       parameter list that the template arguments one does not have to put
   *       those between <...>, so they can be left empty
   */
  friend SerializerBackend& operator>> <> (SerializerBackend&, Axis<T>&);

  /** return the bin that a value will fall in
   *
   * @return the bin that the value will fall into
   * @param val the value that should be histogrammed
   */
  int bin(const precision_t &val)
  {
    return(static_cast<int>(nBins * (val - low) / (up-low)));
  }

  /** check if a bin is an underflow
   *
   * @return true if bin is underflow
   * @param bin the bin to check
   */
  bool isUndeflow(int bin)
  {
    return (bin < 0);
  }

  /** check if a bin is an overflow
   *
   * @return true if bin is overflow
   * @param bin the bin to check
   */
  bool isOverflow(int bin)
  {
    return (nBins < bin);
  }

  /** the number of bins in this axis */
  size_t nBins;

  /** lower end of the axis */
  precision_t low;

  /** upper end of the axis */
  precision_t up;

  /** the title of the axis */
  std::string title;
};

/** A result of a postprocessor
 *
 * The result can be a value or an 1d or 2d array. The 1d or 2d can be used as
 * a histogram.
 *
 * @todo add statistics for the histograming functionality
 *
 * @tparam T the type of data that the result is made of
 * @tparam precision how the precision for adding something to the array
 * @tparam world2hist function to convert world coordinates to histogram coordinates
 *
 * @author Lutz Foucar
 */
template <typename T>
class Result
{
public:
  /** the serialization version of this class */
  enum {serializationVersion=1};

  /** this classes type */
  typedef Result<T> self_type;

  /** a shared pointer of this class */
  typedef std::tr1::shared_ptr<self_type> shared_pointer;

  /** the values of this container */
  typedef T value_t;

  /** a coordinate of a 2d array */
  typedef std::pair<double, double> coordinate_t;

  /** the storage of this container */
  typedef std::vector<value_t> storage_t;

  /** a iterator on the storage */
  typedef typename storage_t::iterator iterator;

  /** a const iterator on the storage */
  typedef typename storage_t::const_iterator const_iterator;

  /** a reference to the storage */
  typedef typename storage_t::reference reference;

  /** a const reference to the storage */
  typedef typename storage_t::const_reference const_reference;

  /** the element accessor type of the storage */
  typedef typename storage_t::size_type size_type;

  /** the axis descriptions of this container */
  typedef Axis<double>  axe_t;

  /** the axis descriptions of this container */
  typedef std::vector<axe_t > axis_t;

  /** which axis one wants to have */
  enum axis_name {xAxis=0, yAxis};

  /** over/ underflow of 2d histogram */
  enum Quadrant{UpperLeft=0, UpperMiddle, UpperRight,
                Left,                     Right,
                LowerLeft  , LowerMiddle, LowerRight};

  /** the over/underflow bin of 1d histogram */
  enum OverUnderFlow{Overflow=0, Underflow};

  /** default constructor
   *
   * using this constructor will create a value container. Only one element is
   * allocated in the storage and no axis are defined.
   */
  Result()
    : _storage(1,0)
  {}

  /** 1d array container constructor
   *
   * use this constructor if you want to create a one dimensional array. In this
   * case the axis automatically set to go from 0 to size -1 having size bins.
   * The vector will be created with size elements
   *
   * @param xaxis The x-axis of the histogram
   */
  explicit Result(const size_type& size)
    : _axis(1),
      _storage(size,0)
  {
    _axis[xAxis] = axis_t(size,0,size-1,"x-Axis");
  }

  /** 1d histogram constructor
   *
   * use this constructor if you want to create a one dimensional histogram.
   * The axis container is resized to fit the xaxis. The storage is set up so
   * that it can hold all the bins of the 1D histogram.
   *
   * @param xaxis The x-axis of the histogram
   */
  explicit Result(const axis_t& xaxis)
    : _axis(1),
      _storage(xaxis.nBins,0)
  {
    _axis[xAxis] = xaxis;
  }

  /** 2d array conatiner constructor
   *
   * use this constructor if you want to create a two dimensional array. The
   * 2d array itself is represented in a linearized array.
   * The axis are created such that they will go from 0 .. rows or cols and have
   * rows or cols bins for the y- and x-axis respectively.
   * The axis container is allocated to fit both axis. The storage is set up so
   * that it can hold rows * cols elemnts in the linearized array.
   *
   * @param cols The x-axis of the histogram
   * @param rows The y-axis of the histogram
   */
  Result(const size_type& cols, const size_type& rows)
    : _axis(2),
      _storage(cols*rows,0)
  {
    _axis[xAxis] = axis_t(cols,0,cols-1,"x-Axis");
    _axis[yAxis] = axis_t(rows,0,rows-1,"y-Axis");
  }

  /** 2d histogram constructor
   *
   * use this constructor if you want to create a two dimensional histogram.
   * The axis container is allocated to fit both axis. The storage is set up so
   * that it can hold all the bins of the 2D histogram in a linearized array.
   *
   * @param xaxis The x-axis of the histogram
   * @param yaxis The y-axis of the histogram
   */
  Result(const axis_t& xaxis, const axis_t& yaxis)
    : _axis(2),
      _storage(xaxis.nBins * yaxis.nbins,0)
  {
    _axis[xAxis] = xaxis;
    _axis[yAxis] = yaxis;
  }

  /** get a specific axis
   *
   * only 1d and 2d histograms have axis. Therefore throws invalid_argument if
   * the requested axis does not exist
   *
   * @param axis the requested axis
   */
  const axis_t& axis(const axis_name& axis) const
  {
    if (_axis.size() <= axis)
      throw std::invalid_argument("Result::axis: the requested axis '" + toString(axis) +
                                  "' does not exist");
    return _axis[axis];
  }

  /** what is the dimension of the result
   *
   * depending on the size of the axis one can determine the dimension of the
   * result.
   *
   * @return dimension of the result
   */
  size_t dim() const
  {
    return _axis.size();
  }

  /** friend for serialization
   *
   * @note make one single instance (called "specialization" in generic terms)
   *       of the template a friend. Because the compiler knows from the
   *       parameter list that the template arguments one does not have to put
   *       those between <...>, so they can be left empty
   */
  friend SerializerBackend& operator<< <> (SerializerBackend&, const self_type&);

  /** friend for deserialization
   *
   * @note make one single instance (called "specialization" in generic terms)
   *       of the template a friend. Because the compiler knows from the
   *       parameter list that the template arguments one does not have to put
   *       those between <...>, so they can be left empty
   */
  friend SerializerBackend& operator>> <> (SerializerBackend&, self_type&);

  /** clear the contents of the result
   *
   * will use the lock to lock the data container before and then overwrite
   * everything with 0.
   */
  void clear()
  {
    QWriteLocker wlock(&lock);
    std::fill(_storage.begin(),_storage.end(),0.f);
  }

  /** assign the result container to a value
   *
   * this function should be used in case the result container is holding just
   * a value. In this case the storage should have only one element which is then
   * assigned to the passed value.
   *
   * @param value the value that is assigned to this container
   */
  self_type& operator=(const value_t& value)
  {
    assert(_axis.empty());
    _storage.front() = value;
    return *this;
  }

  /** evaluate whether value is zero
   *
   * asserts that the container is used as value by checking for 0 axis
   *
   * since the value will most likely be a floating point variable we check
   * whether the absolute value is smaller than the square root of epsilon of the
   * data type.
   *
   * @return true if the result value is zero, false otherwise
   */
  bool operator!() const
  {
    assert(_axis.empty());
    return (std::abs(_storage.front()) < std::sqrt(std::numeric_limits<value_t>::epsilon()));
  }

  /** return the value
   *
   * should only be used when container acts as a value
   *
   * @return the value of the Value like container
   */
  const_reference operator()() const
  {
    assert(_axis.empty());
    return _storage.front();
  }

  /** retrieve a iterator for read access to beginning */
  const_iterator begin()const {return _storage.begin();}

  /** retrieve iterator for write access to beginning */
  iterator begin() {return _storage.begin();}

  /** retrieve iterator to the end of storage */
  const_iterator end()const {return _storage.end();}

  /** calculate the correct bin index for a value
   *
   * @return correct bin index of 1d histogram or overflow / underflow
   * @param value The value whos corresponding bin should be found
   */
  size_t bin(const value_t &value) const
  {
    const int xBin(_axis[xAxis].bin(value));
    if (xBin == _axis[xAxis].isOverflow(xBin))
      return nxBins+Overflow;
    else if (xBin == _axis[xAxis].isUnderflow(xBin))
      return nxBins+Underflow;
    else
      return xBin;
  }

  /** calculate the correct bin index for a value
   *
   * @return correct bin index of 1d histogram or overflow / underflow
   * @param value The value whos corresponding bin should be found
   */
  size_t bin(const coordinate_t &coordinate) const
  {
    const int xBin(_axis[xAxis].bin(coordinate.first));
    const int yBin(_axis[yAxis].bin(coordinate.second));
    const long maxSize  = _axis[xAxis].nBins*_axis[yAxis].nBins;
    const bool xInRange(!(_axis[xAxis].isUnderflow(xBin) && _axis[xAxis].isOverflow(xBin)));
    const bool yInRange(!(_axis[yAxis].isUnderflow(yBin) && _axis[yAxis].isOverflow(yBin)));
    if (_axis[xAxis].isUnderflow(xBin) && _axis[yAxis].isUnderflow(yBin))
      return maxSize+LowerLeft;
    else if (_axis[xAxis].isOverflow(xBin)  && _axis[yAxis].isOverflow(yBin))
      return maxSize+UpperRight;
    else if (_axis[xAxis].isUnderflow(xBin) && _axis[yAxis].isOverflow(yBin))
      return maxSize+UpperLeft;
    else if (_axis[xAxis].isOverflow(xBin)  && _axis[yAxis].isUnderflow(yBin))
      return maxSize+LowerRight;
    else if (xInRange && _axis[yAxis].isUnderflow(yBin))
      return maxSize+LowerMiddle;
    else if (xInRange && _axis[yAxis].isOverflow(yBin))
      return maxSize+UpperMiddle;
    else if (_axis[xAxis].isUnderflow(xBin) && yInRange)
      return maxSize+Left;
    else if (_axis[xAxis].isOverflow(xBin) && yInRange)
      return maxSize+Right;
    else
      return yBin*_axis[xAxis].nBins + xBin;
  }

  /** insert a value at the right bin in the 1d array
   *
   * the position that is passed will be converted to the right bin number
   * using the world2hist function. The value in this bin is increased
   * by weight.
   *
   * @return iterator to the bin in the storage that was increased
   * @param pos the position that is converted to the bin in the histogram
   * @param weight the value that should be added to the value in the bin.
   *               Default ist 1.
   */
  iterator insert(const precision_t& pos, const value_t& weight=1)
  {
    _storage[bin(pos)] += weight;
  }

  /** insert a value at the right bin in the 2d array
   *
   * the position passed will be converted to the right bin number for each axis
   * using the utility world2hist function. The value in this bin is increased
   * by weight.
   *
   * @return iterator to the bin in the storage that was increased
   * @param pos the position that is converted to the bin in the histogram
   * @param weight the value that should be added to the value in the bin.
   *               Default ist 1.
   */
  iterator insert(const coordinate_t& pos, const value_t& weight=1)
  {
    _storage[bin(pos)] += weight;
  }

  /** enable accessing elements of the storage directly
   *
   * @return a reference to the element at position pos in the container.
   * @param pos Position of requested element in the container
   */
  reference operator[](size_type pos)
  {
    return _storage[pos];
  }

  /** enable accessing elements of the storage directly
   *
   * Returns a reference to the element at position pos in the container.
   *
   * @param pos Position of requested element in the container
   */
  const_reference operator[](size_type pos)const
  {
    return _storage[pos];
  }

  /** create a copy of the result
   *
   * @return shared pointer to this result
   */
  shared_pointer clone()const
  {
    shared_pointer sp(new self_type);
    sp->_axis = _axis;
    sp->_storage = _storage;
    return sp;
  }

protected:
  /** copy constructor
   *
   * @note We need to implement this ourselves, since it is not possitble to copy
   * construct the lock
   *
   * @param in the result to copy the values from
   */
  Result(const Result& in)
    : Serializable(in),
      _axis(in._axis),
      _storage(in._storage)
  {}

  /** prevent self assigment */
  self_type& operator=(const self_type&) {}

protected:
  /** lock for locking operations on the data of the container
   *
   * @note this needs to be mutable since we need to lock the data when
   *       serializing it and serializing is const by definition.
   */
  QReadWriteLock lock;

  /** the axis of the histogram */
  axis_t _axis;

  /** result storage */
  storage_t _storage;
};
}// end namespace cass
#endif
