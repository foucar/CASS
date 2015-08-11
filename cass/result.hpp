// Copyright (C) 2011, 2012, 2015 Lutz Foucar

/**
 * @file result.hpp result classes
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

#include "serializable.hpp"

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
  serializer.add(static_cast<uint16_t>(Axis<T>::serializationVersion));
  serializer.add(axis.nBins);
  serializer.add(axis.low);
  serializer.add(axis.up);
  serializer.add(axis.title);
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
  uint16_t version(serializer.retrieve<uint16_t>());
  if(version != Axis<T>::serializationVersion)
    throw std::runtime_error("operator>>(serializer,Axis<T>): Version conflict");
  axis.nBins = serializer.retrieve<size_t>();
  axis.low = serializer.retrieve<T>();
  axis.up = serializer.retrieve<T>();
  axis.title = serializer.retrieve<std::string>();
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
  serializer.add(static_cast<uint16_t>(Result<T>::serializationVersion));
  serializer.add(result._id);
  serializer.add(result._name);
  serializer.add(static_cast<size_t>(result._axis.size()));
  for (typename Result<T>::axis_t::const_iterator it=result._axis.begin(), end = result._axis.end(); it != end; ++it)
    serializer << *it;
  serializer.add(static_cast<size_t>(result._storage.size()));
  for (typename Result<T>::const_iterator it=result.begin(),end=result.end(); it != end; ++it)
    serializer.add(*it);
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
  uint16_t version(serializer.retrieve<uint16_t>());
  if (version != Result<T>::serializationVersion)
    throw std::runtime_error("operator>>(serializer,Result<T>): Version conflict");
  result._id = serializer.retrieve<uint64_t>();
  result._name = serializer.retrieve<std::string>();
  result._axis.resize(serializer.retrieve<size_t>());
  for (typename Result<T>::axis_t::iterator it=result._axis.begin(), end = result._axis.end(); it != end; ++it)
    serializer >> (*it);
  const size_t size(serializer.retrieve<size_t>());
  result._storage.clear();
  for (size_t i(0); i < size; ++i)
    result._storage.push_back(serializer.retrieve<typename Result<T>::value_t>());
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
  enum {serializationVersion=10};

  /** the presision type of the axis boundaries */
  typedef T value_t;

  /** default Constructor */
  Axis()
    : nBins(0),
      low(0.),
      up(0.),
      title()
  {}

  /** Constructor
   *
   * will set the properties in the initializtion list. Will also set the version
   * for the de / serialization.
   *
   * @param nbrBins The Number of Bins the axis contains
   * @param lowerLimit The lower end of the axis
   * @param upperLimit The upper end of the axis
   */
  Axis(size_t nbrBins, value_t lowerLimit, value_t upperLimit, std::string title="Axis Title")
    : nBins(nbrBins),
      low(lowerLimit),
      up(upperLimit),
      title(title)
  {}

  /** calculate the position for a given bin
   *
   * @return the position of the bin
   * @param bin the bin to calculate the postion for
   */
  value_t pos(const int bin) const
  {
    return (low + (bin*(up - low)/nBins));
  }

  /** return the bin that a value will fall in
   *
   * in case the value is nan return a value that states and underflow
   *
   * @return the bin that the value will fall into or -1 in case value is nan
   * @param val the value that should be histogrammed
   */
  int bin(const value_t &val) const
  {
    if (!std::isfinite(val))
      return -1;
    return(static_cast<int>(nBins * (val - low) / (up-low)));
  }

  /** check if a bin is an underflow
   *
   * @return true if bin is underflow
   * @param bin the bin to check
   */
  bool isUnderflow(int bin) const
  {
    return (bin < 0);
  }

  /** check if a bin is an overflow
   *
   * @return true if bin is overflow
   * @param bin the bin to check
   */
  bool isOverflow(int bin) const
  {
    return (static_cast<int>(nBins) < bin);
  }

  /** the number of bins in this axis */
  size_t nBins;

  /** lower end of the axis */
  value_t low;

  /** upper end of the axis */
  value_t up;

  /** the title of the axis */
  std::string title;
};

/** A result of a processor
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
  enum {serializationVersion=10};

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

  /** define the size type of the storage */
  typedef typename storage_t::size_type size_type;

  /** define the shape of the result */
  typedef std::pair<size_type,size_type> shape_t;

  /** the axis descriptions of this container */
  typedef Axis<double>  axe_t;

  /** the axis descriptions of this container */
  typedef std::vector<axe_t > axis_t;

  /** which axis one wants to have */
  enum axis_name {xAxis=0, yAxis};

  /** over/ underflow of 2d histogram */
  enum Quadrant{UpperLeft=0, UpperMiddle, UpperRight,
                Left,                     Right,
                LowerLeft  , LowerMiddle, LowerRight, TwoDStatSize};

  /** the over/underflow bin of 1d histogram */
  enum OverUnderFlow{Overflow=0, Underflow, OneDStatSize};

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
    _axis[xAxis] = axe_t(size,0,size-1,"x-Axis");
  }

  /** 1d histogram constructor
   *
   * use this constructor if you want to create a one dimensional histogram.
   * The axis container is resized to fit the xaxis. The storage is set up so
   * that it can hold all the bins of the 1D histogram.
   *
   * @param xaxis The x-axis of the histogram
   */
  explicit Result(const axe_t& xaxis)
    : _axis(1),
      _storage(xaxis.nBins+OneDStatSize,0)
  {
    _axis[xAxis] = xaxis;
  }

  /** 2d array container or table constructor
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
    _axis[xAxis] = axe_t(cols,0,cols-1,"x-Axis");
    _axis[yAxis] = axe_t(rows,0,rows-1,"y-Axis");
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
  Result(const axe_t& xaxis, const axe_t& yaxis)
    : _axis(2),
      _storage(xaxis.nBins * yaxis.nBins + TwoDStatSize,0)
  {
    _axis[xAxis] = xaxis;
    _axis[yAxis] = yaxis;
  }

  /** read access to the axis
   *
   * @return const reference to the axis object
   */
  const axis_t& axis() const
  {
    return _axis;
  }

  /** write access to the axis
   *
   * @return reference to the axis object
   */
  axis_t& axis()
  {
    return _axis;
  }

  /** read access to a specific axis
   *
   * only 1d and 2d histograms have axis. Therefore throws invalid_argument if
   * the requested axis does not exist
   *
   * @param axis the requested axis
   */
  const axe_t& axis(const axis_name& axis) const
  {
    if (_axis.size() <= axis)
      throw std::invalid_argument("Result::axis: the requested axis does not exist");
    return _axis[axis];
  }

  /** write accesss to a specific axis
   *
   * only 1d and 2d histograms have axis. Therefore throws invalid_argument if
   * the requested axis does not exist
   *
   * @param axis the requested axis
   */
   axe_t& axis(const axis_name& axis)
  {
    if (_axis.size() <= axis)
      throw std::invalid_argument("Result::axis: the requested axis does not exist");
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
   * overwrite all values of the storage with 0.
   */
  void clear()
  {
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
  void setValue(const_reference value)
  {
    if (!_axis.empty())
      throw std::logic_error("Result::setValue: Try using the result as a value, but it has axis");
    _storage.front() = value;
  }

  /** return the value
   *
   * should only be used when container acts as a value
   *
   * @return the value of the Value like container
   */
  value_t getValue() const
  {
    if (!_axis.empty())
      throw std::logic_error("Result::getValue: Try using the result as a value, but it has axis");
    return _storage.front();
  }

  /** evaluate whether value is zero
   *
   * asserts that the container is used as value by checking for 0 axis
   *
   * since the value will most likely be a floating point variable we check
   * whether the absolute value is smaller than the square root of epsilon of the
   * data type.
   *
   * @return true if the result value is non-zero, false otherwise
   */
  bool isTrue() const
  {
    if (!_axis.empty())
      throw std::logic_error("Result::isTrue: Try using the result as a value, but it has axis");
    return !(std::abs(_storage.front()) < std::sqrt(std::numeric_limits<value_t>::epsilon()));
  }

  /** read access to the storage
   *
   * @return const reference to the storage
   */
  const storage_t& storage() const
  {
    return _storage;
  }

  /** write access to the storage
   *
   * @return  reference to the storage
   */
  storage_t& storage()
  {
    return _storage;
  }

  /** retrieve a iterator for read access to beginning
   *
   * @return const iterator to beginning
   */
  const_iterator begin() const {return _storage.begin();}

  /** retrieve iterator for write access to beginning
   *
   * @return  iterator to beginning
   */
  iterator begin() {return _storage.begin();}

  /** retrieve reference to the first element
   *
   * @return reference to the first element
   */
  reference front() {return _storage.front();}

  /** retrieve const reference to the first element
   *
   * @return const reference to the first element
   */
  const_reference front() const {return _storage.front();}

  /** retrieve iterator to the end of storage
   *
   * @return const iterator to end
   */
  const_iterator end() const {return _storage.end();}

  /** retrieve iterator to the end of storage
   *
   * @return iterator to end
   */
  iterator end() {return _storage.end();}

  /** calculate the correct bin index for a value
   *
   * @return correct bin index of 1d histogram or overflow / underflow
   * @param value The value whos corresponding bin should be found
   */
  size_t bin(const value_t &value) const
  {
    if (_axis.size() != 1)
      throw std::logic_error("Result::bin(): Result doesn't have dimension 1");
    const int xBin(_axis[xAxis].bin(value));
    const size_t nxBins(_axis[xAxis].nBins);
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
    if (_axis.size() != 2)
      throw std::logic_error("Result::bin(): Result doesn't have dimension 2");
    const int xBin(_axis[xAxis].bin(coordinate.first));
    const int yBin(_axis[yAxis].bin(coordinate.second));
    const long maxSize  = _axis[xAxis].nBins*_axis[yAxis].nBins;
    const bool xInRange(!_axis[xAxis].isUnderflow(xBin) && !_axis[xAxis].isOverflow(xBin));
    const bool yInRange(!_axis[yAxis].isUnderflow(yBin) && !_axis[yAxis].isOverflow(yBin));
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

  /** add the weight at the right bin for the value in the 1d array
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
  iterator histogram(const value_t& pos, const value_t& weight=1)
  {
    if (_axis.size() != 1)
      throw std::logic_error("Result::histogram(pos): Result doesn't have dimension 1");
    const size_t hist_bin(pos);
    if (hist_bin >= size())
      throw std::out_of_range("Result::histogram(pos): calculated bin isn't with the size of the storage");
    iterator it(begin() + hist_bin);
    *it += weight;
    return it;
  }

  /** add the weight at the right bin for the coordinate in the 2d array
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
  iterator histogram(const coordinate_t& pos, const value_t& weight=1)
  {
    if (_axis.size() != 2)
      throw std::logic_error("Result::histogram(coordinate): Result doesn't have dimension 2");
    if (bin(pos) >= size())
      throw std::out_of_range("Result::histogram(coordinate): calculated bin isn't with the size of the storage");
    iterator it(begin() + bin(pos));
    *it += weight;
    return it;
  }

  /** add row(s) to the result
   *
   * in case the result is used as a table this will add row(s) to the table
   *
   * @param rows the rows to be appended to the table like result
   */
  void appendRows(const storage_t &rows)
  {
    if (_axis.size() != 2)
      throw std::logic_error("Result::appendRows(): Result doesn't have dimension 2");
    if (rows.size() % _axis[xAxis].nBins)
      throw std::runtime_error("Result::appendRows: The rowsize is not a modulo of the rowsize of the table '");
    const int nRows(rows.size() / _axis[xAxis].nBins);
    _axis[yAxis].nBins += nRows;
    _axis[yAxis].up = axis(yAxis).nBins - 1;

    _storage.insert(_storage.end(),rows.begin(),rows.end());
  }

  /** reset the table like result */
  void resetTable()
  {
    if (_axis.size() != 2)
      throw std::logic_error("Result::resetTable(): Result doesn't have dimension 2");
    _axis[yAxis].nBins = 0;
    _axis[yAxis].up = -1;
  }

  /** append a value to the end of the result
   *
   * append the value and set the axis to reflect the new content
   * @param val the value to append
   */
  void append(const value_t &val)
  {
    if (_axis.size() != 1)
      throw std::logic_error("Result::push_back(): Result doesn't have dimension 1");
    _storage.push_back(val);
    _axis[xAxis].nBins = _storage.size();
    _axis[xAxis].up = _axis[xAxis].nBins - 1;
  }

  /** clear the appendable 1d like result
   *
   * clears the storage and reset xaxis to reflect the new content
   */
  void reset()
  {
    if (_axis.size() != 1)
      throw std::logic_error("Result::reset(): Result doesn't have dimension 1");
    _storage.clear();
    _axis[xAxis].nBins = _storage.size();
    _axis[xAxis].up = _axis[xAxis].nBins - 1;
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
  const_reference operator[](size_type pos) const
  {
    return _storage[pos];
  }

  /** return the shape of the result
   *
   * in case it is a value the constant 0,0 will be returned. In case it is a
   * 1d result the second parameter is 0, and the full shape in case it is a
   * 2d result will be returned.
   *
   * @return the shape of the result
   */
  shape_t shape() const
  {
    switch (_axis.size())
    {
    case 0:
      return std::make_pair(1,1);
      break;
    case 1:
      return std::make_pair(axis(xAxis).nBins,1);
      break;
    case 2:
      return std::make_pair(axis(xAxis).nBins,axis(yAxis).nBins);
      break;
    default:
      throw std::logic_error("Result::shape(): Result doesn't have dimension 2");
    }
  }

  /** return the size of the data as determined by the axis
   *
   * This size will be the size of the data without the space reserved for the
   * statistics
   *
   * @return the data size
   */
  size_type datasize() const
  {
    return (shape().first * shape().second);
  }

  /** return the raw size of the storage
   *
   * @return the size of the storage
   */
  size_type size() const
  {
    return _storage.size();
  }

  /** create a copy of the result
   *
   * @return shared pointer to this result
   */
  shared_pointer clone() const
  {
    shared_pointer sp(new self_type);
    sp->_axis = _axis;
    sp->_storage = _storage;
    sp->_name = _name;
    sp->_id = _id;
    return sp;
  }

  /** copy the contents of a different result to this result
   *
   * copy the axis and the storage only
   *
   * @param in input whos contents should be copied to here
   */
  void assign(const self_type& in)
  {
    _storage = in._storage;
    _axis = in._axis;
  }

  /** retrieve the name of the result
   *
   * @return the name of the result
   */
  std::string name() const
  {
    return _name;
  }

  /** set the name of the result
   *
   * @param name the name that the result should have
   */
  void name(const std::string & name)
  {
    _name = name;
  }

  /** retrieve the id of the result
   *
   * @return the id
   */
  uint64_t id()const
  {
    return _id;
  }

  /** set the id of the result
   *
   * @param id the id that the result should have
   */
  void id(uint64_t id)
  {
    _id = id;
  }

public:
  /** lock for locking operations on the data of the container
   *
   * @note this needs to be mutable since we need to lock the data when
   *       serializing it and serializing is const by definition.
   */
  mutable QReadWriteLock lock;

protected:
  /** copy constructor
   *
   * @note We need to implement this ourselves, since it is not possitble to copy
   *       construct the lock
   *
   * @param in the result to copy the values from
   */
  Result(const Result& in)
    : Serializable(in),
      _axis(in._axis),
      _storage(in._storage),
      _name(in._name),
      _id(in._id)
  {}

  /** prevent self assigment */
  self_type& operator=(const self_type&) {}

protected:
  /** the axis of the histogram */
  axis_t _axis;

  /** result storage */
  storage_t _storage;

  /** the name of this result */
  std::string _name;

  /** the id of the event that the contents reflect */
  uint64_t _id;
};
}// end namespace cass
#endif
