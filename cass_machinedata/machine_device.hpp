//Copyright (C) 2009, 2010, 2014 Lutz Foucar

/**
 * @file machine_device.hpp definitions of a machine device
 *
 * @author Lutz Foucar
 */

#ifndef _MACHINEDATADEVICE_H_
#define _MACHINEDATADEVICE_H_

#include <map>
#include <vector>
#include <string>
#include <iostream>

#include "device_backend.hpp"
#include "cass_machine.hpp"
#include "serializer.hpp"

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
class CASS_MACHINEDATASHARED_EXPORT MachineDataDevice
    : public cass::DeviceBackend
{
public:
  /** constructor initializing values to meaningful data*/
  MachineDataDevice()
    :DeviceBackend(1),
      _evrdata(8,false),
      _energy(0),
      _wavelength(0)
  {}

public:
  /** typedef for more readable code
   * @note instead of double as second one could make it a QVariant to be able to
   *       also store strings.
   */
  typedef std::map<std::string,double> epicsDataMap_t;

  /** typedef for more readable code*/
  typedef std::map<std::string,double> bldMap_t;

  /** typedef for more readable code*/
  typedef std::vector<bool> evrStatus_t;

  /** define the spectrometer data */
  typedef std::map<std::string,std::vector<uint32_t> > spectrometer_t;

public:
  /** serialize the device to the serializer*/
  void serialize(cass::SerializerBackend&)const;

  /** deserialize the device from the serializer*/
  bool deserialize(cass::SerializerBackend&);

public:
  //@{
  /** getter */
  const epicsDataMap_t  &EpicsData()const     {return _epicsdata;}
  const bldMap_t        &BeamlineData()const  {return _blddata;}
  const evrStatus_t     &EvrData()const       {return _evrdata;}
  double                 energy()const        {return _energy;}
  double                 wavelength()const    {return _wavelength;}
  const spectrometer_t  &spectrometers()const {return _spectrometers;}
  bool                   epicsFilled()const   {return _epicsFilled;}

  //@}
  //@{
  /** setter */
  bldMap_t       &BeamlineData()  {return _blddata;}
  epicsDataMap_t &EpicsData()     {return _epicsdata;}
  evrStatus_t    &EvrData()       {return _evrdata;}
  double         &energy()        {return _energy;}
  double         &wavelength()    {return _wavelength;}
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

  //data that gets calculated in Analysis//
  double        _energy;    //!< the calculated puls energy
  double        _wavelength;//!< the corrosponding wavelength

  /** status flag to tell whether the epics variables have been filled during conversion */
  bool _epicsFilled;
};
}//end namespace machinedata
}//end namespace cass

inline void cass::MachineData::MachineDataDevice::serialize(cass::SerializerBackend &out)const
{
  //the version//
  writeVersion(out);
  //the beamlinedata//
  out.addSizet(_blddata.size());
  //for each entry in the map copy the length of the name, the name, then the value//
  for (bldMap_t::const_iterator it = _blddata.begin (); it != _blddata.end (); ++it)
  {
    out.addString(it->first);
    out.addDouble(it->second);
  }
  //the epics data//
  out.addSizet(_epicsdata.size());
  //for each entry in the map copy the length of the name, the name, then the value//
  for (epicsDataMap_t::const_iterator it = _epicsdata.begin (); it != _epicsdata.end (); ++it)
  {
    out.addString(it->first);
    out.addDouble(it->second);
  }
}

inline bool cass::MachineData::MachineDataDevice::deserialize(cass::SerializerBackend &in)
{
  //check whether the version fits//
  checkVersion(in);

  //beamlinedata//
  //clear the map//
  _blddata.clear ();
  //read the size of the map//
  size_t len = in.retrieveSizet ();
  //retrieve every entry of the map
  for (size_t i=0; i<len; ++i)
  {
    std::string str = in.retrieveString();
    double val = in.retrieveDouble();
    _blddata[str] = val;
  }

  //epics data//
  //clear the map//
  _epicsdata.clear();
  //read the size of the map//
  len = in.retrieveSizet ();
  //retrieve every entry of the map
  for (size_t i=0; i<len; ++i)
  {
    std::string str = in.retrieveString();
    double val = in.retrieveDouble();
    _blddata[str] = val;
  }
  return true;
}

#endif
