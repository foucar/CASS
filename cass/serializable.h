//Copyright (C) 2010 lmf

#ifndef _SERIALIZABLE_H_
#define _SERIALIZABLE_H_

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
    /** pure virtual function that needs to be defined by the derived class
      will serialize an object to the Serializer class */
    virtual void serialize(cass::Serializer&)const=0;
    /** pure virtual function that needs to be defined by the derived class
      will deserialize an object from the Serializer class */
    virtual void deserialize(cass::Serializer&)=0;
  };
}
#endif
