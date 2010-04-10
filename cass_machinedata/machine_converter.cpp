#include <sstream>
#include <iostream>

#include "machine_converter.h"

#include "pdsdata/xtc/Xtc.hh"
#include "pdsdata/bld/bldData.hh"
#include "pdsdata/epics/EpicsPvData.hh"
#include "pdsdata/evr/DataV3.hh"

#include "cass_event.h"
#include "machine_device.h"



//Use the code copied from matt weaver to extract the value from the epicsheader
#define CASETOVAL(timetype,valtype) case timetype: {			\
  const Pds::EpicsPvTime<valtype>& p = static_cast<const Pds::EpicsPvTime<valtype>&>(epicsData); \
  const Pds::EpicsDbrTools::DbrTypeFromInt<valtype>::TDbr* value = &p.value;	\
  for(int i=0; i<epicsData.iNumElements; i++) \
  it++->second = *value++;\
  break; }


void cass::MachineData::Converter::operator()(const Pds::Xtc* xtc, cass::CASSEvent* cassevent)
{
  //during a configure transition we don't get a cassevent, so we should extract the machineevent//
  //only when cassevent is non zero//
  MachineDataDevice *md = 0;
  if (cassevent)
  {
    md = dynamic_cast<MachineDataDevice*>(cassevent->devices()[cass::CASSEvent::MachineData]);
    md->EpicsData() = _store.EpicsData();
  }

  switch (xtc->contains.id())
  {
  case(Pds::TypeId::Id_FEEGasDetEnergy):
    {
      const Pds::BldDataFEEGasDetEnergy &gasdet = 
        *reinterpret_cast<const Pds::BldDataFEEGasDetEnergy*>(xtc->payload());
      md->BeamlineData()["f_11_ENRC"] = gasdet.f_11_ENRC;
      md->BeamlineData()["f_12_ENRC"] = gasdet.f_12_ENRC;
      md->BeamlineData()["f_21_ENRC"] = gasdet.f_21_ENRC;
      md->BeamlineData()["f_22_ENRC"] = gasdet.f_22_ENRC;
      break;
    }
  case(Pds::TypeId::Id_EBeam):
    {
      const Pds::BldDataEBeam &beam = 
        *reinterpret_cast<const Pds::BldDataEBeam*>(xtc->payload());
      md->BeamlineData()["EbeamCharge"]   = beam.fEbeamCharge;
      md->BeamlineData()["EbeamL3Energy"] = beam.fEbeamL3Energy;
      md->BeamlineData()["EbeamLTUAngX"]  = beam.fEbeamLTUAngX;
      md->BeamlineData()["EbeamLTUAngY"]  = beam.fEbeamLTUAngY;
      md->BeamlineData()["EbeamLTUPosX"]  = beam.fEbeamLTUPosX;
      md->BeamlineData()["EbeamLTUPosY"]  = beam.fEbeamLTUPosY;
      md->BeamlineData()["EbeamPkCurrBC2"]= beam.fEbeamPkCurrBC2;
      break;
    }
  case(Pds::TypeId::Id_PhaseCavity):
    {
      const Pds::BldDataPhaseCavity &cavity = 
        *reinterpret_cast<const Pds::BldDataPhaseCavity*>(xtc->payload());
      md->BeamlineData()["Charge1"]  = cavity.fCharge1;
      md->BeamlineData()["Charge2"]  = cavity.fCharge2;
      md->BeamlineData()["FitTime1"] = cavity.fFitTime1;
      md->BeamlineData()["FitTime2"] = cavity.fFitTime2;
      break;
    }
  case(Pds::TypeId::Id_Epics):
    {
      //std::cout << "found epics typeid ";
      const Pds::EpicsPvHeader& epicsData = 
        *reinterpret_cast<const Pds::EpicsPvHeader*>(xtc->payload());
      //std::cout << epicsData.iDbrType<<std::endl;
      //cntrl is a configuration type and will only be send with a configure transition//
      if ( dbr_type_is_CTRL(epicsData.iDbrType) )
      {
        const Pds::EpicsPvCtrlHeader& ctrl = 
          static_cast<const Pds::EpicsPvCtrlHeader&>(epicsData);
        //std::cout << "epics control with id "<<ctrl.iPvId<<" and name "<< ctrl.sPvName<<" is added to index map"<<std::endl;
        //record what name the pvId has, this help later to find the name, which is the index of map in machineevent//
        _index2name[ctrl.iPvId] = ctrl.sPvName;
        //now we need to create the map which we will fill later with real values//
        //if this epics variable is an array we want an entry in the map for each entry in the array//
        if (ctrl.iNumElements > 1)
        {
          //std::cout << "ctrl is bigger than 1"<<std::endl;
          //go through all entries of the array//
          //create an entry in the map with the the index in brackets//
          //and initialize it with 0//
          for (int i=0;i<ctrl.iNumElements;++i)
          {
            std::stringstream entryname;
            entryname << ctrl.sPvName << "[" << i << "]";
            _store.EpicsData()[entryname.str()] = 0.;
            //std::cout << "add "<<entryname.str() << " to machinedatamap"<<std::endl;
          }
        }
        //otherwise we just add the name to the map and initialze it with 0//
        else
        {
          _store.EpicsData()[ctrl.sPvName] = 0.;
          //std::cout << "add "<<ctrl.sPvName << " to machinedatamap"<<std::endl;
        }
      }
      //time is the actual data, that will be send down the xtc with 1 Hz
      else if(dbr_type_is_TIME(epicsData.iDbrType))
      {
        //now we need to find the variable name in the map//
        //therefore we look up the name in the indexmap//
        std::string name = _index2name[epicsData.iPvId];
        //std::cout <<"found id "<<epicsData.iPvId<<" lookup in the indexmap revealed the name "<<name<<std::endl;
        //if it is an array we added the braces with the array index before,
        //so we need to add it also now before trying to find the name in the map//
        if (epicsData.iNumElements > 1)
          name.append("[0]");
        //std::cout << "now the name is "<<name<<std::endl;
        //try to find the the name in the map//
        //this returns an iterator to the first entry we found//
        //if it was an array we can then use the iterator to the next values//
        MachineDataDevice::epicsDataMap_t::iterator it = 
          _store.EpicsData().find(name);
        //if the name is not in the map//
        //then output an erromessage//
        if (it == _store.EpicsData().end())
          std::cerr << "epics variable with id "<<epicsData.iPvId<<" was not defined"<<std::endl;
        //otherwise extract the epicsData and write it into the map
        else
        {
          switch(epicsData.iDbrType)
          {
            CASETOVAL(DBR_TIME_SHORT ,DBR_SHORT)
              CASETOVAL(DBR_TIME_FLOAT ,DBR_FLOAT)
              CASETOVAL(DBR_TIME_ENUM  ,DBR_ENUM)
              CASETOVAL(DBR_TIME_LONG  ,DBR_LONG)
              CASETOVAL(DBR_TIME_DOUBLE,DBR_DOUBLE)
          default: break;
          }
        }
      }
      break;
    }
  case(Pds::TypeId::Id_EvrData):
    {
      //clear the status bytes of the event code//
      std::fill(md->EvrData().begin(),md->EvrData().end(),false);
      //get the evr data//
      const Pds::EvrData::DataV3 &evrData =
          *reinterpret_cast<const Pds::EvrData::DataV3*>(xtc->payload());
      //how many events have happened between the last event and now//
      const uint32_t nbrFifoEvents = evrData.numFifoEvents();
      //go through all events and extract the eventcode from them//
      for (size_t i=0;i<nbrFifoEvents;++i)
      {
        const Pds::EvrData::DataV3::FIFOEvent& fifoEvent = evrData.fifoEvent(i);
        uint32_t eventcode = fifoEvent.EventCode;
        //check wether the event code is of interest for us (is in our assigned range)//
        if (EVREventCodeOffset<= eventcode && eventcode < EVREventCodeOffset+8)
          //if so then set the flag that the code happend to true//
          md->EvrData()[eventcode-EVREventCodeOffset]=true;
      }
    }
  break;

  default: break;
  }
  //copy the epics values in the storedevent to the machineevent
  if (cassevent)
    md->EpicsData() = _store.EpicsData();

}
