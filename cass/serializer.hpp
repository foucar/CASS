//Copyright (C) 2010, 2015 Lutz Foucar

/**
 * @file cass/serializer.hpp file contains classes for serializing objects
 *
 * @author Lutz Foucar
 */

#ifndef _SERIALIZER_HPP_
#define _SERIALIZER_HPP_

#include <sstream>
#include <fstream>
#include <string>
#include <stdexcept>
#include <typeinfo>
#include <stdint.h>

/**
 * @note if this is set, SerializerBackend is made abstract (pure virtual member)
 *       to see if it is instantiated somewhere.
 *       switch it off in release mode for performance gain.
 * @author Stephan Kassemeyer
 */
//#define SERIALIZER_INTERFACE_TEST

namespace cass
{

/** A serializer.
 *
 * base class that will serialize / de serialize
 * Serializable classes to an iostream
 * This is an interface that should not be instantiated
 * (can be made pure virtual once virtual methods are introduced).
 *
 * @author Lutz Foucar
 * @author Stephan Kassemeyer
 */
class SerializerBackend
{
public:
  /** constructor */
  SerializerBackend()
    : _checkSumGroupStartedForRead(false),
      _checkSumGroupStartedForWrite(false)
  {}

#ifdef SERIALIZER_INTERFACE_TEST
  /** virtual member to test */
  virtual void abstractTest() = 0;

  /** virtual member to test */
  virtual ~SerializerBackend(){}
#endif

  /** flush the stream */
  void flush() { _stream->flush(); }

  /** write data to the stream
   *
   * @return reference to the out stream
   * @param data the data to write
   * @param n size of the data
   */
  std::ostream& writeToStream( const char* data, std::streamsize n)
  {
    if (_checkSumGroupStartedForWrite) addToOutChecksum(data, n);
    return _stream->write(data, n);
  }

  /** read data from the stream
   *
   * @return reference to the in stream
   * @param data the data to write
   * @param n size of the data
   */
  std::istream& readFromStream( char* data, std::streamsize n)
  {
    if (_checkSumGroupStartedForRead)
    {
      std::istream& stream = _stream->read(data, n);
      addToInChecksum(data, n);
      return(stream);
    }
    else
      return _stream->read(data, n);
  }

  //@{
  /** control checksum calculation for the next values added
   *
   * usage:
   * add...;
   * add...;
   * startChecksumGroupforRead();
   * add...;
   * add...;
   * if (!endChecksumGroupForRead()) error();
   */
  void startChecksumGroupForRead()
  {
    _checkSumGroupStartedForRead = true;
    r_sum1 = 0xff;
    r_sum2 = 0xff;
  }
  bool endChecksumGroupForRead()
  {
    _checkSumGroupStartedForRead = false;
    /** finalize checksum: */
    r_sum1 = (r_sum1 & 0xff) + (r_sum1 >> 8);
    r_sum2 = (r_sum2 & 0xff) + (r_sum2 >> 8);
    if (retrieve<uint8_t>() != (uint8_t)r_sum1 ) return false;
    if (retrieve<uint8_t>() != (uint8_t)r_sum2 ) return false;
    return true;
  }
  //@}

  //@{
  /** control checksum for writing to stream
   *
   * usage:
   * retrieve...;
   * retrieve...;
   * startChecksumGroupforWrite();
   * retrieve...;
   * retrieve...;
   * endChecksumGroupForWrite();
   */
  void startChecksumGroupForWrite()
  {
    _checkSumGroupStartedForWrite = true;
    w_sum1 = 0xff;
    w_sum2 = 0xff;
  }
  void endChecksumGroupForWrite()
  {
    _checkSumGroupStartedForWrite = false;
    /** finalize checksum: */
    w_sum1 = (w_sum1 & 0xff) + (w_sum1 >> 8);
    w_sum2 = (w_sum2 & 0xff) + (w_sum2 >> 8);
    add(static_cast<uint8_t>(w_sum1) );
    add(static_cast<uint8_t>(w_sum2) );
  }
  //@}

  /** add arbitrary value to the stream
   *
   * @tparam Type the type of the value
   * @param value the value to add
   */
  template <typename Type>
  void add(const Type& value)
  {
    writeToStream(reinterpret_cast<const char *> (&value), sizeof (Type));
  }

  /** read arbitrary value from stream
   *
   * @tparam Type the type of the value
   * @return the read value
   */
  template <typename Type>
  Type retrieve()
  {
    Type value;
    readFromStream (reinterpret_cast<char *> (&value), sizeof (Type));
    return value;
  }

protected:
  /** fletcher16 algorithm for 8 bit input
   *
   * @param data the data to add to the fletcher algorithm
   * @param len the size of the data to be added to the fletcher algorithm
   *
   * @author Stephan Kassemeyer
   */
  void addToOutChecksum( const char* data, std::streamsize len)
  {
    while (len) {
      size_t tlen = len > 21 ? 21 : len;
      len -= tlen;
      do {
        w_sum1 += *data++;
        w_sum2 += w_sum1;
      } while (--tlen);
      w_sum1 = (w_sum1 & 0xff) + (w_sum1 >> 8);
      w_sum2 = (w_sum2 & 0xff) + (w_sum2 >> 8);
    }
  }

  /** fletcher16 algorithm for 8 bit input
   *
   * @param data the data to add to the fletcher algorithm
   * @param len the size of the data to be added to the fletcher algorithm
   *
   * @author Stephan Kassemeyer
   */
  void addToInChecksum( const char* data, std::streamsize len)
  {
    while (len) {
      size_t tlen = len > 21 ? 21 : len;
      len -= tlen;
      do {
        r_sum1 += *data++;
        r_sum2 += r_sum1;
      } while (--tlen);
      r_sum1 = (r_sum1 & 0xff) + (r_sum1 >> 8);
      r_sum2 = (r_sum2 & 0xff) + (r_sum2 >> 8);
    }
  }

protected:
  /** the string to serialize the objects to (buffer) */
  std::iostream* _stream;

  /** sum 1 for reading */
  uint16_t r_sum1;

  /** sum 2 for reading */
  uint16_t r_sum2;

  /** sum 1 for writing */
  uint16_t w_sum1;

  /** sum 2 for writing */
  uint16_t w_sum2;

  /** flag to enable getting parts to a checksum */
  bool _checkSumGroupStartedForRead;

  /** flag to enable adding parts to a checksum */
  bool _checkSumGroupStartedForWrite;

};
/** specialization for strings
 *
 * write the length of the string, then the string itselve
 *
 * @param string to add to the stream
 */
template <>
inline
void SerializerBackend::add<std::string>(const std::string &str)
{
  const size_t len(str.length());
  add(len);
  writeToStream(str.data(), len);
}

/** specialization for string
 *
 * create a temp string with right size containing blanks and read data to it
 *
 * @return the retrieved string
 */
template <>
inline
std::string SerializerBackend::retrieve<std::string>()
{
  const size_t len(retrieve<size_t>());
  std::string str(len,' ');
  readFromStream (&str[0], len);
  return str;
}




/** A string serializer.
 *
 * class that will serialize / de serialize Serializable classes to
 * a stringstream
 *
 * @author Lutz Foucar
 * @author Stephan Kassemeyer
 */
class Serializer : public SerializerBackend
{
public:
  /** constructor.
   *
   * will open the stream in binary writing mode
   */
  Serializer()
  {
    _stream = new std::stringstream(std::ios_base::binary|std::ios_base::out);
  }

  /** constructor.
   *
   * will open the provided string for reading in binary mode
   *
   * @param string the string that we want to read from
   */
  Serializer(const std::string &string)
  {
    _stream = new std::stringstream(string,std::ios_base::binary|std::ios_base::in);
  }

  /** destructor.
   *
   * deletes stream object
   */
  ~Serializer()
  {
    delete _stream;
  }

  /** retrieve a const reference to the string.
   *
   * @return const string of our stringstream
   */
  const std::string buffer()const  {return dynamic_cast<std::stringstream*>(_stream)->str();}

#ifdef SERIALIZER_INTERFACE_TEST
  /** the abstract class test */
  virtual void abstractTest() {}
#endif
};





/** A file output serializer.
 *
 * class that will serialize Serializable classes to a file
 *
 * @author Stephan Kassemeyer
 */
class SerializerWriteFile : public SerializerBackend
{
public:
  /** constructor.
   *
   * will open the stream in binary reading/writing mode
   */
  SerializerWriteFile( const char* filename )
  {
    _stream = new std::fstream(filename, std::ios_base::binary|std::ios_base::out);
    _opened = true;
  }

  /** destructor.
   *
   * closes the file and deletes stream object.
   */
  ~SerializerWriteFile()
  {
    close();
    delete _stream;
  }

  /** close file */
  void close()  {if (_opened) dynamic_cast<std::fstream*>(_stream)->close();}

#ifdef SERIALIZER_INTERFACE_TEST
  /** abstract class test */
  virtual void abstractTest() {}
#endif

protected:
  /** flag to see if the file is opened */
  bool _opened;
};





/** A file input deserializer.
 *
 * class that will deserialize Serializable classes from a file
 *
 * @author Stephan Kassemeyer
 */
class SerializerReadFile : public SerializerBackend
{
public:
  /** constructor.
   *
   * will open the stream in binary reading/writing mode
   */
  SerializerReadFile( const char* filename )
  {
    _stream = new std::fstream(filename, std::ios_base::binary|std::ios_base::in);
    _opened = true;
  }

  /** destructor.
   *
   * closes the file and deletes stream object.
   */
  ~SerializerReadFile()
  {
    close();
    delete _stream;
  }

  /** close file */
  void close()  {if (_opened) dynamic_cast<std::fstream*>(_stream)->close();}

#ifdef SERIALIZER_INTERFACE_TEST
  /** abstract class test */
  virtual void abstractTest() {}
#endif

protected:
  /** flag to tell whether the file is opened */
  bool _opened;
};
}//end namespace cass
#endif
