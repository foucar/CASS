//Copyright (C) 2009, 2010, 2014, 2015 Lutz Foucar

/**
 * @file machine_device.hpp definitions of a machine device
 *
 * @author Lutz Foucar
 */

#ifndef _MACHINEDATADEVICE_HPP_
#define _MACHINEDATADEVICE_HPP_

#include <map>
#include <vector>
#include <string>
#include <iostream>

#include "device_backend.hpp"
#include "serializable.hpp"

namespace cass
{
namespace MachineData
{
/** Container for all Machine related Data
 *
 * This device contains all data that is machine related
 * - Beamline Data
 * - Epics Data
 * - Evr Data
 *
 * @author Lutz Foucar
 */
class Device : public DeviceBackend
{
public:
  /** constructor initializing values to meaningful data*/
  Device()
    : DeviceBackend(1),
      _epicsFilled(false)
  {}

public:
  /** define the epics container
   * @note instead of double as second one could make it a QVariant to be able to
   *       also store strings.
   */
  typedef std::map<std::string,double> epicsDataMap_t;

  /** define the beamline data container */
  typedef std::map<std::string,double> bldMap_t;

  /** define the evr status container */
  typedef std::vector<bool> evrStatus_t;

  /** define the spectrometer data */
  typedef std::map<std::string,std::vector<uint32_t> > spectrometer_t;

public:
  /** serialize the device to the serializer
   *
   * @param out the stream to serialze this class to
   */
  void serialize(SerializerBackend &out)const
  {
    /** write the version */
    writeVersion(out);
    /** write the size of the beamlinedata */
    out.add(static_cast<size_t>(_blddata.size()));
    /** for each beamline entry in the map write the name and then the value */
    for (bldMap_t::const_iterator it = _blddata.begin (); it != _blddata.end (); ++it)
    {
      out.add(it->first);
      out.add(it->second);
    }
    /** write the epics data size */
    out.add(static_cast<size_t>(_epicsdata.size()));
    /** for each epics entry in the map write the name and then the value */
    for (epicsDataMap_t::const_iterator it = _epicsdata.begin (); it != _epicsdata.end (); ++it)
    {
      out.add(it->first);
      out.add(it->second);
    }
  }

  /** deserialize the device from the stream
   *
   * @return true when this class was deserialized from the stream sucessfully
   * @param in the stream to serialize this class from
   */
  bool deserialize(SerializerBackend &in)
  {
    /** check whether the version is correct */
    checkVersion(in);
    /** read the beamlinedata, first clear the existing map */
    _blddata.clear();
    /** then read the size of the map */
    size_t len(in.retrieve<size_t>());
    /** now retrieve every entry of the map and add it to the map */
    for (size_t i=0; i<len; ++i)
    {
      const bldMap_t::key_type key(in.retrieve<bldMap_t::key_type>());
      _blddata[key] = in.retrieve<bldMap_t::mapped_type>();
    }
    /** read the epics data, first clear the existing map */
    _epicsdata.clear();
    /** read the size of the map */
    len = in.retrieve<size_t>();
    /** now retrieve every entry of the map and add it to the map */
    for (size_t i=0; i<len; ++i)
    {
      const epicsDataMap_t::key_type key(in.retrieve<epicsDataMap_t::key_type>());
      _epicsdata[key] = in.retrieve<epicsDataMap_t::mapped_type>();
    }
    return true;
  }

public:
  //@{
  /** getter */
  const epicsDataMap_t  &EpicsData()const     {return _epicsdata;}
  const bldMap_t        &BeamlineData()const  {return _blddata;}
  const evrStatus_t     &EvrData()const       {return _evrdata;}
  const spectrometer_t  &spectrometers()const {return _spectrometers;}
  bool                   epicsFilled()const   {return _epicsFilled;}

  //@}
  //@{
  /** setter */
  bldMap_t       &BeamlineData()  {return _blddata;}
  epicsDataMap_t &EpicsData()     {return _epicsdata;}
  evrStatus_t    &EvrData()       {return _evrdata;}
  spectrometer_t &spectrometers() {return _spectrometers;}
  bool           &epicsFilled()   {return _epicsFilled;}
  //@}

private:
  //beamline data//
  bldMap_t       _blddata;  //!< map containing the beamlinedata
  epicsDataMap_t _epicsdata;//!< a map containing all epics data in the xtc stream
  evrStatus_t    _evrdata;  //!< a vector of bools describing the evr status

  /** container for beamline spectrometer data */
  spectrometer_t _spectrometers;

  /** status flag to tell whether the epics variables have been filled during conversion */
  bool _epicsFilled;
};

}//end namespace machinedata
}//end namespace cass
#endif
