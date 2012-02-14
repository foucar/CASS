//Copyright (C) 2010 Lutz Foucar

/**
 * @file serializable.h file contains base class all serializable classes
 *
 * @author Lutz Foucar
 */

#ifndef _SERIALIZABLE_H_
#define _SERIALIZABLE_H_

#include <stdint.h>
#include "serializer.h"

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
      :_version(version)
    {}

    /** virtual destructor to avoid warning with gcc 4.1.2 */
    virtual ~Serializable(){}

    /** pure virtual function that needs to be defined by the derived class.
     *
     * will serialize an object to the Serializer class
     */
    virtual void serialize(cass::SerializerBackend&)const=0;

    /** pure virtual function that needs to be defined by the derived class.
     *
     * will deserialize an object from the Serializer class
     */
    virtual bool deserialize(cass::SerializerBackend&)=0;

    /** retrieve the version of the serializer */
    uint16_t ver()const {return _version;}

    /** write the version to the stream
     *
     * @param out the stream to write the version to
     */
    virtual void writeVersion(SerializerBackend &out)const
    {
      out.addUint16(_version);
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
      uint16_t version;
      version = in.retrieveUint16();
      assert(version == _version);
    }

protected:
    /** the version for de/serializing*/
    uint16_t _version;
  };
}
#endif
