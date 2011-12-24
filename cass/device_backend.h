// Copyright (C) 2009, 2010 Lutz Foucar

#ifndef CASS_DEVICEBACKEND_H
#define CASS_DEVICEBACKEND_H

#include <stdexcept>
#include <tr1/memory>

#include "cass.h"
#include "serializable.h"

namespace cass
{
/** A Baseclass for all Devices in the CASSEvent.
 *
 * All devices need to be serializable, therefore this class
 * inerhits from serializable.
 *
 * @todo make deserialization errors not propagate with bool but with exceptions
 * @author Lutz Foucar
 */
class CASSSHARED_EXPORT DeviceBackend : public cass::Serializable
{
public:
  /** a shared pointer of this type */
  std::tr1::shared_ptr<DeviceBackend> shared_pointer;

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

  /** detector getter.
   *
   * Must be implemented by commercial and pnccd devices
   */
  virtual const detectors_t *detectors()const
  {
    throw std::runtime_error("DeviceBackend::detectors(): device has no detectors");
    return 0;
  }

  /** detector getter.
   *
   * Must be implemented by commercial and pnccd devices
   */
  virtual detectors_t *detectors()
  {
    throw std::runtime_error("DeviceBackend::detectors(): device has no detectors");
    return 0;
  }
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
