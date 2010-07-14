//Copyright (C) 2010 Lutz Foucar

/**
 * @file cass_event.h file contains declaration of the CASSEvent
 *
 * @author Lutz Foucar
 */

#ifndef CASSEVENT_H
#define CASSEVENT_H

#include <map>
#include <stdint.h>
#include "cass.h"
#include "serializable.h"


namespace cass
{
  class DeviceBackend;
  class SerializerBackend;

  /** Event to store all LCLS Data
   *
   * a cassevent that stores all information comming from
   * the machine, and also some calculated information
   *
   * @author Lutz Foucar
   */
  class CASSSHARED_EXPORT CASSEvent : public Serializable
  {
  public:
    /** constructor will create all devices*/
    CASSEvent();

    /** destroyes all devices */
    ~CASSEvent();

  public:
    /** known devices */
    enum Device{pnCCD, Acqiris, CCD, MachineData};

    /** mapping from device type to handler instance */
    typedef std::map<Device, DeviceBackend*> devices_t;

  public:
    /** serialize a event to the Serializer*/
    void serialize(SerializerBackend&)const;

    /** deserialize an event from the Serializer*/
    bool deserialize(SerializerBackend&);

  public:
    //@{
    /** setters */
    uint64_t        &id()             {return _id;}
    char            *datagrambuffer() {return _datagrambuffer;}
    devices_t       &devices()        {return _devices;}
    void             setFilename(const char * f) {_filename = f;}
    //@}
    //@{
    /** getters */
    uint64_t         id()const        {return _id;}
    const char      *datagrambuffer()const {return _datagrambuffer;}
    const devices_t &devices()const   {return _devices;}
    const char      *filename()const  {return _filename;}
    //@}

  private:
    /** id of the cassevent */
    uint64_t _id;

    /** list of devices for this event */
    devices_t _devices;

    /** buffer for the datagram that contains all LCLS information */
    char _datagrambuffer[cass::DatagramBufferSize];

    /** filename of XTC file which this event came from (if offline */
    const char * _filename;
  };
}//end namespace

#endif // CASSEVENT_H
