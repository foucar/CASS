// Copyright (C) 2009 lmf

#ifndef CASS_DEVICEBACKEND_H
#define CASS_DEVICEBACKEND_H

#include "cass.h"

namespace cass
{
  class Serializer;

  class CASSSHARED_EXPORT DeviceBackend
  {
  public:
    virtual ~DeviceBackend() {}
    virtual void serialize(Serializer&)const=0;
    virtual void deserialize(Serializer&)=0;
  protected:
    uint16_t _version;
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
