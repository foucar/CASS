//Copyright (C) 2010 - 2011 Lutz Foucar

/**
 * @file serializer.h file contains classes for serializing objects
 *
 * @author Lutz Foucar
 */

#ifndef _SERIALIZER_H_
#define _SERIALIZER_H_

#include <sstream>
#include <fstream>
#include <string>
#include <stdint.h>

#define SERIALIZER_INTERFACE_TEST // if this is set, SerializerBackend is made abstract (pure virtual member)
// to see if it is instantiated somewhere.
// switch it off in release mode for performance gain.

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
   */
  class  SerializerBackend
  {
  public:
    SerializerBackend() {
      _checkSumGroupStartedForRead = false;
      _checkSumGroupStartedForWrite = false;
    }
#ifdef SERIALIZER_INTERFACE_TEST
    virtual void abstractTest() = 0;
    virtual ~SerializerBackend(){}
#endif
    void flush() { _stream->flush(); }

    std::ostream& writeToStream( const char* data, std::streamsize n);
    std::istream& readFromStream( char* data, std::streamsize n);

    /** usage: add...; add...; startChecksumGroupforRead(); add...; add...; if (!endChecksumGroupForRead()) error(); **/
    void startChecksumGroupForRead();
    bool endChecksumGroupForRead();
    /** usage: retrieve...; retrieve...; startChecksumGroupforWrite(); retrieve...; retrieve...; endChecksumGroupForWrite(); **/
    void startChecksumGroupForWrite();
    void endChecksumGroupForWrite();

    void addString(const std::string&); //!< add string to serialized buffer
    void addUint16(const uint16_t);     //!< add uint16 to serialized buffer
    void addUint8(const uint8_t);       //!< add uint16 to serialized buffer
    void addInt16(const int16_t);       //!< add int16 to serialized buffer
    void addUint32(const uint32_t);     //!< add uint32 to serialized buffer
    void addInt32(const int32_t);       //!< add int32 to serialized buffer
    void addUint64(const uint64_t);     //!< add uint64 to serialized buffer
    void addInt64(const int64_t);       //!< add int64 to serialized buffer
    void addSizet(const size_t);        //!< add size_t to serialized buffer
    void addDouble(const double);       //!< add double to serialized buffer
    void addFloat(const float);         //!< add float to serialized buffer
    void addBool(const bool);           //!< add bool to serialized buffer

    std::string retrieveString();   //!< retrieve string from serialized buffer
    uint16_t    retrieveUint16();   //!< retrieve string from serialized buffer
    uint8_t     retrieveUint8();    //!< retrieve string from serialized buffer
    int16_t     retrieveInt16();    //!< retrieve string from serialized buffer
    uint32_t    retrieveUint32();   //!< retrieve string from serialized buffer
    int32_t     retrieveInt32();    //!< retrieve string from serialized buffer
    uint64_t    retrieveUint64();   //!< retrieve string from serialized buffer
    int64_t     retrieveInt64();    //!< retrieve string from serialized buffer
    size_t      retrieveSizet();    //!< retrieve string from serialized buffer
    double      retrieveDouble();   //!< retrieve string from serialized buffer
    float       retrieveFloat();    //!< retrieve string from serialized buffer
    bool        retrieveBool();     //!< retrieve string from serialized buffer

  protected:
    std::iostream* _stream;    //!< the string to serialize the objects to (buffer)

    uint16_t r_sum1, r_sum2, w_sum1, w_sum2;
    bool _checkSumGroupStartedForRead, _checkSumGroupStartedForWrite;
    void addToOutChecksum( const char* data, std::streamsize n);
    void addToInChecksum( const char* data, std::streamsize n);

  };

  /** A string serializer.
   *
   * class that will serialize / de serialize
   * Serializable classes to a stringstream
   *
   * @author Lutz Foucar
   * @author Stephan Kassemeyer
   */
  class  Serializer : public SerializerBackend
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
    virtual void abstractTest() {}
#endif
  };

  /** A file output serializer.
   *
   * class that will serialize
   * Serializable classes to a stringstream
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
    virtual void abstractTest() {}
#endif

  protected:
    bool _opened;
  };

  /** A file input deserializer.
   *
   * class that will deserialize
   * Serializable classes from a stringstream
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
    virtual void abstractTest() {}
#endif

  protected:
    bool _opened;
  };


}//end namespace


// fletcher16 for 8bit input:
inline void cass::SerializerBackend::addToOutChecksum( const char* data, std::streamsize len)
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

// fletcher16 for 8bit input:
inline void cass::SerializerBackend::addToInChecksum( const char* data, std::streamsize len)
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

inline bool cass::SerializerBackend::endChecksumGroupForRead()
{
  _checkSumGroupStartedForRead = false;
  // finalize checksum:
  r_sum1 = (r_sum1 & 0xff) + (r_sum1 >> 8);
  r_sum2 = (r_sum2 & 0xff) + (r_sum2 >> 8);
  //retrieveUint8( reinterpret_cast<uint8_t>r_sum1 );
  if (retrieveUint8() != (uint8_t)r_sum1 ) return false;
  //retrieveUint8( reinterpret_cast<uint8_t>r_sum2 );
  if (retrieveUint8() != (uint8_t)r_sum2 ) return false;
  return true;
}

inline void cass::SerializerBackend::endChecksumGroupForWrite()
{
  _checkSumGroupStartedForWrite = false;
  // finalize checksum:
  w_sum1 = (w_sum1 & 0xff) + (w_sum1 >> 8);
  w_sum2 = (w_sum2 & 0xff) + (w_sum2 >> 8);
  //addUint8( reinterpret_cast<uint8_t>(w_sum1) );
  addUint8( (uint8_t)(w_sum1) );
  //addUint8( reinterpret_cast<uint8_t>(w_sum2) );
  addUint8( (uint8_t)(w_sum2) );
}

inline std::istream& cass::SerializerBackend::readFromStream( char* data, std::streamsize n)
{
  if (_checkSumGroupStartedForRead) {
    std::istream& stream = _stream->read(data, n); \
                           addToInChecksum(data, n);
    return(stream);
  }
  else return _stream->read(data, n);
}

inline std::ostream& cass::SerializerBackend::writeToStream( const char* data, std::streamsize n)
{
  if (_checkSumGroupStartedForWrite) addToOutChecksum(data, n);
  return _stream->write(data, n);
}

inline void cass::SerializerBackend::startChecksumGroupForRead()
{
  _checkSumGroupStartedForRead = true;
  r_sum1 = 0xff;
  r_sum2 = 0xff;
}

inline void cass::SerializerBackend::startChecksumGroupForWrite()
{
  _checkSumGroupStartedForWrite = true;
  w_sum1 = 0xff;
  w_sum2 = 0xff;
}


inline void cass::SerializerBackend::addString(const std::string& str)
{
  //first the length of the string, then the string itselve//
  size_t len = str.length ();
  addSizet(len);
  writeToStream (str.data(), len);
}
inline std::string cass::SerializerBackend::retrieveString()
{
  //first the length of the string, then the string itselve//
  size_t len = retrieveSizet();
  std::string str(len,' '); //create a temp string with right size
  readFromStream (&str[0], len);
  return str;
}

inline void cass::SerializerBackend::addUint8(const uint8_t ui)
{
  writeToStream (reinterpret_cast<const char *> (&ui), sizeof (uint8_t));
}
inline uint8_t cass::SerializerBackend::retrieveUint8()
{
  uint8_t ui;
  readFromStream (reinterpret_cast<char *> (&ui), sizeof (uint8_t));
  return ui;
}

inline void cass::SerializerBackend::addUint16(const uint16_t ui)
{
  writeToStream (reinterpret_cast<const char *> (&ui), sizeof (uint16_t));
}
inline uint16_t cass::SerializerBackend::retrieveUint16()
{
  uint16_t ui;
  readFromStream (reinterpret_cast<char *> (&ui), sizeof (uint16_t));
  return ui;
}

inline void cass::SerializerBackend::addInt16(const int16_t i)
{
  writeToStream (reinterpret_cast<const char *> (&i), sizeof (int16_t));
}
inline int16_t cass::SerializerBackend::retrieveInt16()
{
  int16_t i;
  readFromStream (reinterpret_cast<char *> (&i), sizeof (int16_t));
  return i;
}

inline void cass::SerializerBackend::addUint32(const uint32_t ui)
{
  writeToStream (reinterpret_cast<const char *> (&ui), sizeof (uint32_t));
}
inline uint32_t cass::SerializerBackend::retrieveUint32()
{
  uint32_t ui;
  readFromStream (reinterpret_cast<char *> (&ui), sizeof (uint32_t));
  return ui;
}

inline void cass::SerializerBackend::addInt32(const int32_t i)
{
  writeToStream (reinterpret_cast<const char *> (i), sizeof (int32_t));
}
inline int32_t cass::SerializerBackend::retrieveInt32()
{
  int32_t i;
  readFromStream (reinterpret_cast<char *> (&i), sizeof (int32_t));
  return i;
}

inline void cass::SerializerBackend::addUint64(const uint64_t ui)
{
  writeToStream (reinterpret_cast<const char *> (&ui), sizeof (uint64_t));
}
inline uint64_t cass::SerializerBackend::retrieveUint64()
{
  uint64_t ui;
  readFromStream (reinterpret_cast<char *> (&ui), sizeof (uint64_t));
  return ui;
}

inline void cass::SerializerBackend::addInt64(const int64_t i)
{
  writeToStream (reinterpret_cast<const char *> (&i), sizeof (int64_t));
}
inline int64_t cass::SerializerBackend::retrieveInt64()
{
  int64_t i;
  readFromStream (reinterpret_cast<char *> (&i), sizeof (int64_t));
  return i;
}

inline void cass::SerializerBackend::addSizet(const size_t s)
{
  writeToStream (reinterpret_cast<const char *> (&s), sizeof (size_t));
}
inline size_t cass::SerializerBackend::retrieveSizet()
{
  size_t s;
  readFromStream (reinterpret_cast<char *> (&s), sizeof (size_t));
  return s;
}

inline void cass::SerializerBackend::addFloat(const float f)
{
  writeToStream (reinterpret_cast<const char *> (&f), sizeof (float));
}
inline float cass::SerializerBackend::retrieveFloat()
{
  float f;
  readFromStream (reinterpret_cast<char *> (&f), sizeof (float));
  return f;
}

inline void cass::SerializerBackend::addDouble(const double d)
{
  writeToStream (reinterpret_cast<const char *> (&d), sizeof (double));
}
inline double cass::SerializerBackend::retrieveDouble()
{
  double d;
  readFromStream (reinterpret_cast<char *> (&d), sizeof (double));
  return d;
}

inline void cass::SerializerBackend::addBool(const bool b)
{
  writeToStream (reinterpret_cast<const char *> (&b), sizeof (bool));
}
inline bool cass::SerializerBackend::retrieveBool()
{
  bool b;
  readFromStream (reinterpret_cast<char *> (&b), sizeof (bool));
  return b;
}

#endif
