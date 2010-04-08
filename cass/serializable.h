//Copyright (C) 2010 lmf

#ifndef _SERIALIZABLE_H_
#define _SERIALIZABLE_H_

#include <stdint.h>

namespace cass
{
  //forward declaration
  class Serializer;

  /*! Serializable

  pure virtual class that all serializable classes should inherit from.

  @author lmf
  */
  class Serializable
  {
  public:
    /** constructor initializing the version*/
    Serializable(uint16_t version)
      :_version(version)
    {}
    /** virtual destructor to avoid warning with gcc  4.1.2  */
    virtual ~Serializable(){}
    /** pure virtual function that needs to be defined by the derived class
      will serialize an object to the Serializer class */
    virtual void serialize(cass::Serializer&)const=0;
    /** pure virtual function that needs to be defined by the derived class
      will deserialize an object from the Serializer class */
    virtual void deserialize(cass::Serializer&)=0;
  protected:
    /** the version for de/serializing*/
    uint16_t _version;
  };
}
#endif
