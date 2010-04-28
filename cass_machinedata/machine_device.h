//Copyright (C) 2009, 2010 Lutz Foucar

#ifndef _MACHINEDATADEVICE_H_
#define _MACHINEDATADEVICE_H_

#include <map>
#include <vector>
#include <string>
#include <iostream>

#include "device_backend.h"
#include "cass_machine.h"
#include "serializer.h"

namespace cass
{
  namespace MachineData
  {
    /*! Container for all Machine related Data

      This device contains all data that is machine related
      <ul>
      <li>Beamline Data
      <li>Epics Data
      <li>Evr Data
      </ul>
      @author Lutz Foucar
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
      // the following was missing
      ~MachineDataDevice() {}

    public:
      /** typedef for more readable code
        @note instead of double as second one could make it a QVariant to be able to
              also store strings.
      */
      typedef std::map<std::string,double> epicsDataMap_t;
      /** typedef for more readable code*/
      typedef std::map<std::string,double> bldMap_t;
      /** typedef for more readable code*/
      typedef std::vector<bool> evrStatus_t;

    public:
      /** serialize the device to the serializer*/
      void serialize(cass::Serializer&);
      /** deserialize the device from the serializer*/
      void deserialize(cass::Serializer&);

    public:
      /** setters and getters*/

      const epicsDataMap_t &EpicsData()const  {return _epicsdata;}
      epicsDataMap_t &EpicsData()             {return _epicsdata;}

      const bldMap_t &BeamlineData()const     {return _blddata;}
      bldMap_t       &BeamlineData()          {return _blddata;}

      const evrStatus_t &EvrData()const       {return _evrdata;}
      evrStatus_t    &EvrData()               {return _evrdata;}

      double          energy()const           {return _energy;}
      double         &energy()                {return _energy;}

      double          wavelength()const       {return _wavelength;}
      double         &wavelength()            {return _wavelength;}

    private:
      //beamline data//
      bldMap_t       _blddata;  //!< map containing the beamlinedata
      epicsDataMap_t _epicsdata;//!< a map containing all epics data in the xtc stream
      evrStatus_t    _evrdata;  //!< a vector of bools describing the evr status

      //data that gets calculated in Analysis//
      double        _energy;    //!< the calculated puls energy
      double        _wavelength;//!< the corrosponding wavelength
    };
  }//end namespace machinedata
}//end namespace cass

inline void cass::MachineData::MachineDataDevice::serialize(cass::Serializer &out)
{
  //the version//
  out.addUint16(_version);
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

inline void cass::MachineData::MachineDataDevice::deserialize(cass::Serializer &in)
{
  //check whether the version fits//
  uint16_t ver = in.retrieveUint16();
  if(ver!=_version)
  {
    std::cerr<<"version conflict in MachineData: "<<ver<<" "<<_version<<std::endl;
    return;
  }

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
}

#endif
