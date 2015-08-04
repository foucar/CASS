//Copyright (C) 2011, 2015 Lutz Foucar

/**
 * @file cass/serializable.hpp file contains base class all serializable classes
 *
 * @author Lutz Foucar
 */

#ifndef _SERIALIZABLE_HPP_
#define _SERIALIZABLE_HPP_

#include <stdint.h>
#include <sstream>

#include "serializer.hpp"

namespace cass
{
//forward declaration
class SerializerBackend;

/** Serializable.
 *
 * pure virtual class that all serializable classes should inherit from.
 * This makes sure that all classes that should be serializable
 *
 * @author Lutz Foucar
 */
class Serializable
{
public:
  /** constructor initializing the version*/
  explicit Serializable(uint16_t version)
    : _version(version)
  {}

  /** virtual destructor to avoid warning with gcc 4.1.2 */
  virtual ~Serializable(){}

  /** pure virtual function that needs to be defined by the derived class.
   *
   * will serialize an object to the Serializer class
   */
  virtual void serialize(SerializerBackend&)const=0;

  /** pure virtual function that needs to be defined by the derived class.
   *
   * will deserialize an object from the Serializer class
   */
  virtual bool deserialize(SerializerBackend&)=0;

  /** retrieve the version of the serializer */
  uint16_t ver()const {return _version;}

  /** write the version to the stream
   *
   * @param out the stream to write the version to
   */
  virtual void writeVersion(SerializerBackend &out)const
  {
    out.add(_version);
  }

  /** check the version
   *
   * asserts that the version of this class corresponsed to the one read
   * from the stream
   *
   * @param in the stream to read the version from
   */
  virtual void checkVersion(SerializerBackend& in)const
  {
    uint16_t version(in.retrieve<uint16_t>());
    if(version != _version)
    {
      std::stringstream ss;
      ss << "Version mismatch in '" << typeid(*this).name();
      ss << "': '" << version << "' != '" << _version << "'";
      throw std::runtime_error(ss.str());
    }
  }

protected:
  /** the version for de/serializing*/
  uint16_t _version;
};
}
#endif
