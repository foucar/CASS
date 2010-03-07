//Copyright (C) 2010 lmf

#ifndef _SERIALIZER_H_
#define _SERIALIZER_H_

#include <sstream>
#include <string>
#include "cass.h"

namespace cass
{
  //class that will serialize / de serialize things to a stringstream
  class CASSSHARED_EXPORT Serializer
  {
  public:
    Serializer(){}
//    Serializer(std::stringstream &inputstream)
//      :_stream(inputstream)
//    {
//    }
//    const std::string &buffer()const  {return  _stream.str();}
//    std::string       &buffer()       {return  _stream.str();}

    void addString(const std::string&);
    void addUint16(const uint16_t);
    void addInt16(const int16_t);
    void addUint32(const uint32_t);
    void addInt32(const int32_t);
    void addUint64(const uint64_t);
    void addInt64(const int64_t);
    void addSizet(const size_t);
    void addDouble(const double);
    void addFloat(const float);
    void addBool(const bool);

    std::string retrieveString();
    uint16_t    retrieveUint16();
    int16_t     retrieveInt16();
    uint32_t    retrieveUint32();
    int32_t     retrieveInt32();
    uint64_t    retrieveUint64();
    int64_t     retrieveInt64();
    size_t      retrieveSizet();
    double      retrieveDouble();
    float       retrieveFloat();
    bool        retrieveBool();

  private:
    std::stringstream _stream;
  };
}


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
  std::string str(len,''); //create a temp string with right size
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
  _stream.write (reinterpret_castconst <char *> (&ui), sizeof (uint32_t));
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
