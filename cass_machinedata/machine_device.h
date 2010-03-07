//Copyright (C) 2010 lmf

#ifndef _MACHINEDATADEVICE_H_
#define _MACHINEDATADEVICE_H_

#include <map>
#include <string>

#include "device_backend.h"
#include "cass_machine.h"
#include "serializer.h"

namespace cass
{
  namespace MachineData
  {
    class CASS_MACHINEDATASHARED_EXPORT MachineDataDevice : public cass::DeviceBackend
    {
    public:
      MachineDataDevice()
        :_version(1),
        _energy(0),
        _wavelength(0)
      {
      }

      ~MachineDataDevice()  {}

    public:
      typedef std::map<std::string,double> epicsDataMap_t;
      typedef std::map<std::string,double> bldMap_t;

    public:
      void serialize(cass::Serializer&)const;
      void deserialize(cass::Serializer&);

    public:
      const epicsDataMap_t& EpicsData()const  {return _epicsdata;}
      epicsDataMap_t& EpicsData()             {return _epicsdata;}

      const bldMap_t& BeamlineData()const     {return _blddata;}
      bldMap_t&       BeamlineData()          {return _blddata;}

      double          energy()const           {return _energy;}
      double          energy()                {return _energy;}

      double          wavelength()const       {return _wavelength;}
      double          wavelength()            {return _wavelength;}

    private:
      //beamline data//
      bldMap_t       _blddata;  //map containing the beamlinedata
      epicsDataMap_t _epicsdata;//a map containing all epics data in the xtc stream

      //data that gets calculated in Analysis//
      double        _energy;    //the calculated puls energy
      double        _wavelength;//the corrosponding wavelength

    };
  }//end namespace machinedata
}//end namespace cass

inline void cass::MachineData::MachineDataDevice::serialize(cass::Serializer &out) const
{
  //the version//
  out.addUint16(_version);
  //the beamlinedata//
  out.addSizet(_blddata.size());
  //for each entry in the map copy the length of the name, the name, then the value//
  bldMap_t::const_iterator it;
  for (it = _blddata.begin (); it != _blddata.end (); ++it)
  {
      out.addString(it->first);
      out.addDouble(it->second);
  }
  //the epics data//
  out.addSizet(_epicsdata.size());
  //for each entry in the map copy the length of the name, the name, then the value//
  epicsDataMap_t::const_iterator it;
  for (it = _epicsdata.begin (); it != _epicsdata.end (); ++it)
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
