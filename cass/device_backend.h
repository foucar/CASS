// Copyright (C) 2009 lmf

#ifndef CASS_DEVICEBACKEND_H
#define CASS_DEVICEBACKEND_H

#include "cass.h"
#include "serializable.h"

namespace cass
{
  /*! A Baseclass for all Devices in the CASSEvent

  All devices need to be serializable, therefore this class
  inerhits from serializable
  @author lmf
  */
  class CASSSHARED_EXPORT DeviceBackend : public cass::Serializable
  {
  public:
    /** constructor already initializing the serialization version*/
    DeviceBackend(uint16_t version)
      :Serializable(version)
    {}
    /** serializer is still pure virtual*/
    virtual void serialize(cass::Serializer &)=0;
    /** deserializer is still pure virtual*/
    virtual void deserialize(cass::Serializer &)=0;
    /** virtual desctructor*/
    virtual ~DeviceBackend() {}
  };
}//end namespace cass

#endif



// Local Variables:
// coding: utf-8
// mode: C++
// c-file-offsets: ((c . 0) (innamespace . 0))
// c-file-style: "Stroustrup"
// fill-column: 100
// End:
