//Copyright (C) 2010 Lutz Foucar

#ifndef _SERIALIZER_H_
#define _SERIALIZER_H_

#include <sstream>
#include <string>
#include "cass.h"

namespace cass
{
  /** A serializer.
   * class that will serialize / de serialize
   * Serializable classes to a stringstream
   * @author Lutz Foucar
   */
  class CASSSHARED_EXPORT Serializer
  {
  public:
    /** constructor.
     * will open the stream in binary writing mode
     */
    Serializer()
      :_stream(std::ios_base::binary|std::ios_base::in)
    {}
    /** constructor.
     * will open the provided string for reading in binary mode
     * @param string the string that we want to read from
     */
    Serializer(const std::string &string)
      :_stream(string,std::ios_base::binary|std::ios_base::out)
    {}
    /** retrieve a const reference to the string*/
    const std::string buffer()const  {return _stream.str();}

    void addString(const std::string&); //!< add string to serialized buffer
    void addUint16(const uint16_t);     //!< add uint16 to serialized buffer
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
    std::stringstream _stream;    //!< the string to serialize the objects to (buffer)
  };
}//end namespace




inline void cass::Serializer::addString(const std::string& str)
{
  //first the length of the string, then the string itselve//
  size_t len = str.length ();
  addSizet(len);
  _stream.write (str.data(), len);
}
inline std::string cass::Serializer::retrieveString()
{
  //first the length of the string, then the string itselve//
  size_t len = retrieveSizet();
  std::string str(len,' '); //create a temp string with right size
  _stream.read (&str[0], len);
  return str;
}

inline void cass::Serializer::addUint16(const uint16_t ui)
{
  _stream.write (reinterpret_cast<const char *> (&ui), sizeof (uint16_t));
}
inline uint16_t cass::Serializer::retrieveUint16()
{
  uint16_t ui;
  _stream.read (reinterpret_cast<char *> (&ui), sizeof (uint16_t));
  return ui;
}

inline void cass::Serializer::addInt16(const int16_t i)
{
  _stream.write (reinterpret_cast<const char *> (&i), sizeof (int16_t));
}
inline int16_t cass::Serializer::retrieveInt16()
{
  int16_t i;
  _stream.read (reinterpret_cast<char *> (&i), sizeof (int16_t));
  return i;
}

inline void cass::Serializer::addUint32(const uint32_t ui)
{
  _stream.write (reinterpret_cast<const char *> (&ui), sizeof (uint32_t));
}
inline uint32_t cass::Serializer::retrieveUint32()
{
  uint32_t ui;
  _stream.read (reinterpret_cast<char *> (&ui), sizeof (uint32_t));
  return ui;
}

inline void cass::Serializer::addInt32(const int32_t i)
{
  _stream.write (reinterpret_cast<const char *> (i), sizeof (int32_t));
}
inline int32_t cass::Serializer::retrieveInt32()
{
  int32_t i;
  _stream.read (reinterpret_cast<char *> (&i), sizeof (int32_t));
  return i;
}

inline void cass::Serializer::addUint64(const uint64_t ui)
{
  _stream.write (reinterpret_cast<const char *> (&ui), sizeof (uint64_t));
}
inline uint64_t cass::Serializer::retrieveUint64()
{
  uint64_t ui;
  _stream.read (reinterpret_cast<char *> (&ui), sizeof (uint64_t));
  return ui;
}

inline void cass::Serializer::addInt64(const int64_t i)
{
  _stream.write (reinterpret_cast<const char *> (&i), sizeof (int64_t));
}
inline int64_t cass::Serializer::retrieveInt64()
{
  int64_t i;
  _stream.read (reinterpret_cast<char *> (&i), sizeof (int64_t));
  return i;
}

inline void cass::Serializer::addSizet(const size_t s)
{
  _stream.write (reinterpret_cast<const char *> (&s), sizeof (size_t));
}
inline size_t cass::Serializer::retrieveSizet()
{
  size_t s;
  _stream.read (reinterpret_cast<char *> (&s), sizeof (size_t));
  return s;
}

inline void cass::Serializer::addFloat(const float f)
{
  _stream.write (reinterpret_cast<const char *> (&f), sizeof (float));
}
inline float cass::Serializer::retrieveFloat()
{
  float f;
  _stream.read (reinterpret_cast<char *> (&f), sizeof (float));
  return f;
}

inline void cass::Serializer::addDouble(const double d)
{
  _stream.write (reinterpret_cast<const char *> (&d), sizeof (double));
}
inline double cass::Serializer::retrieveDouble()
{
  double d;
  _stream.read (reinterpret_cast<char *> (&d), sizeof (double));
  return d;
}

inline void cass::Serializer::addBool(const bool b)
{
  _stream.write (reinterpret_cast<const char *> (&b), sizeof (bool));
}
inline bool cass::Serializer::retrieveBool()
{
  bool b;
  _stream.read (reinterpret_cast<char *> (&b), sizeof (bool));
  return b;
}

#endif
