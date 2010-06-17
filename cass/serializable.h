//Copyright (C) 2010 Lutz Foucar

#ifndef _SERIALIZABLE_H_
#define _SERIALIZABLE_H_

#include <stdint.h>
#include "serializer.h"

namespace cass
{
  //forward declaration
  class SerializerBackend;

  /** Serializable
   * pure virtual class that all serializable classes should inherit from.
   * This makes sure that all classes that should be serializable
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
     * will serialize an object to the Serializer class
     */
    virtual void serialize(cass::SerializerBackend&)const=0;

    /** pure virtual function that needs to be defined by the derived class.
     * will deserialize an object from the Serializer class
     */
    virtual bool deserialize(cass::SerializerBackend&)=0;

    /** retrieve the version of the serializer */
    uint16_t ver()const {return _version;}

protected:
    /** the version for de/serializing*/
    uint16_t _version;
  };
}
#endif
