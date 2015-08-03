// Copyright (C) 2009, 2010 Lutz Foucar

/**
 * @file device_backend.h contains base class for all devices that are part of
 *                        the cassevent.
 *
 * @author Lutz Foucar
 */

#ifndef CASS_DEVICEBACKEND_H
#define CASS_DEVICEBACKEND_H

#include <stdexcept>
#include <tr1/memory>

#include "cass.h"
#include "serializable.hpp"

namespace cass
{
/** A Baseclass for all Devices in the CASSEvent.
 *
 * @note All devices need to be serializable, therefore this class
 *       inerhits from serializable.
 *
 * @author Lutz Foucar
 */
class DeviceBackend : public Serializable
{
public:
  /** a shared pointer of this type */
  typedef std::tr1::shared_ptr<DeviceBackend> shared_pointer;

  /** constructor already initializing the serialization version */
  DeviceBackend(uint16_t version)
    :Serializable(version)
  {}

  /** serializer is still pure virtual */
  virtual void serialize(cass::SerializerBackend &)const=0;

  /** deserializer is still pure virtual */
  virtual bool deserialize(cass::SerializerBackend &)=0;

  /** virtual desctructor */
  virtual ~DeviceBackend() {}
};
}//end namespace cass

#endif
