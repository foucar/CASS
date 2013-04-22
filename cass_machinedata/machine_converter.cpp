#include <sstream>
#include <iostream>

#include "machine_converter.h"

#include "pdsdata/xtc/Xtc.hh"
#include "pdsdata/bld/bldData.hh"
#include "pdsdata/epics/EpicsPvData.hh"
#include "pdsdata/evr/DataV3.hh"
#include "pdsdata/ipimb/DataV2.hh"
#include "pdsdata/xtc/DetInfo.hh"
#include "pdsdata/lusi/IpmFexV1.hh"
#include "pdsdata/lusi/IpmFexConfigV2.hh"
#include "pdsdata/control/ConfigV1.hh"
#include "pdsdata/control/PVControl.hh"

#include "cass_event.h"
#include "log.h"
#include "machine_device.h"

using namespace cass::MachineData;
using namespace std;

// =================define static members =================
cass::ConversionBackend::shared_pointer Converter::_instance;
QMutex Converter::_mutex;

cass::ConversionBackend::shared_pointer Converter::instance()
{
  QMutexLocker locker(&_mutex);
  if(!_instance)
  {
    _instance = ConversionBackend::shared_pointer(new Converter());
  }
  return _instance;
}
// ========================================================


namespace cass
{
namespace MachineData
{
/** convert epics variable to double
 *
 * convert the value contained in the Epics variable to a double and fill it
 * into the map. One has to define beforehand where to add it and pass the right
 * iterator to the function
 *
 * @tparam valtype the value type of the epics variable
 * @param epicsData the object that contains the epics data
 * @param first iterator to the first entry in the map that needs to be filled
 *
 * @author Lutz Foucar
 */
template <int valtype>
void convertEpicsToDouble(const Pds::EpicsPvHeader& epicsData,
                          MachineDataDevice::epicsDataMap_t::iterator first)
{
  const Pds::EpicsPvTime<valtype> &p
      (reinterpret_cast<const Pds::EpicsPvTime<valtype>&>(epicsData));
  const typename Pds::EpicsDbrTools::DbrTypeFromInt<valtype>::TDbr* value(&p.value);
  for(int i=0; i<epicsData.iNumElements; ++i)
    first++->second = *value++;
}


/** Key for the epics lookup map
 *
 * Key for mapping Epics index and list to a specific name
 *
 * @author Lutz Foucar
 */
class EpicsKey
{
public:
  /** constructor
   *
   * @param src the info about the source of the epics list
   * @param epicsIdx the index of the epics variable in the list
   */
  EpicsKey(const Pds::Src &src, int epicsIdx)
    : _phy(src.phy()),
      _idx(epicsIdx)
  {}

  /** check whether this is less than other
   *
   * will compare for less first _log. If this is the same it will
   * compare for less the _phys value and last the _index value. This makes sure
   * that the Id is unique.
   *
   * @return true when this is smaller than other
   * @param other the other key that one compares this key to
   */
  bool operator <(const EpicsKey& other) const
  {
    if (_phy != other._phy)
      return _phy < other._phy;
    return _idx < other._idx;
  }

private:
  /** the phy of the src */
  uint32_t _phy;

  /** the index of the epics variable */
  int16_t _idx;

};
}//end namespace MachineData
}//end namespace cass

Converter::Converter()
{
  _pdsTypeList.push_back(Pds::TypeId::Id_Epics);
  _pdsTypeList.push_back(Pds::TypeId::Id_FEEGasDetEnergy);
  _pdsTypeList.push_back(Pds::TypeId::Id_EBeam);
  _pdsTypeList.push_back(Pds::TypeId::Id_PhaseCavity);
  _pdsTypeList.push_back(Pds::TypeId::Id_EvrData);
  _pdsTypeList.push_back(Pds::TypeId::Id_IpimbData);
  _pdsTypeList.push_back(Pds::TypeId::Id_IpmFex);
  _pdsTypeList.push_back(Pds::TypeId::Id_ControlConfig);
}

void cass::MachineData::Converter::operator()(const Pds::Xtc* xtc, cass::CASSEvent* cassevent)
{
  /** during a configure transition we don't get a cassevent, so we should extract the machineevent
   *  only when cassevent is non zero
   */
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
    uint32_t version (xtc->contains.version());
    const Pds::BldDataEBeam &beam =
        *reinterpret_cast<const Pds::BldDataEBeam*>(xtc->payload());
    if (beam.uDamageMask)
    {
      for(size_t i(0); i<Pds::BldDataEBeam::nbrOf; ++i)
      {
        if ((0x1 << i) & beam.uDamageMask)
          Log::add(Log::VERBOSEINFO,"'" +
                   string(Pds::BldDataEBeam::name(static_cast<Pds::BldDataEBeam::varname>(i))) +
                   "' is damaged");
      }
    }
    switch (version)
    {
    case (3):
    {
      if(!((0x1 << Pds::BldDataEBeam::EbeamPkCurrBC1) & beam.uDamageMask))
        md->BeamlineData()["EbeamPkCurrBC1"]= beam.fEbeamPkCurrBC1;
      if(!((0x1 << Pds::BldDataEBeam::EbeamEnergyBC1) & beam.uDamageMask))
        md->BeamlineData()["fEbeamEnergyBC1"]= beam.fEbeamEnergyBC1;
    }
    case (2):
    {
      if(!((0x1 << Pds::BldDataEBeam::EbeamEnergyBC2) & beam.uDamageMask))
        md->BeamlineData()["fEbeamEnergyBC2"]= beam.fEbeamEnergyBC2;
    }
    case (1):
    {
      if(!((0x1 << Pds::BldDataEBeam::EbeamPkCurrBC2) & beam.uDamageMask))
        md->BeamlineData()["EbeamPkCurrBC2"]= beam.fEbeamPkCurrBC2;
    }
    case (0):
    {
      if(!((0x1 << Pds::BldDataEBeam::EbeamCharge) & beam.uDamageMask))
        md->BeamlineData()["EbeamCharge"]   = beam.fEbeamCharge;
      if(!((0x1 << Pds::BldDataEBeam::EbeamL3Energy) & beam.uDamageMask))
        md->BeamlineData()["EbeamL3Energy"] = beam.fEbeamL3Energy;
      if(!((0x1 << Pds::BldDataEBeam::EbeamLTUAngX) & beam.uDamageMask))
        md->BeamlineData()["EbeamLTUAngX"]  = beam.fEbeamLTUAngX;
      if(!((0x1 << Pds::BldDataEBeam::EbeamLTUAngY) & beam.uDamageMask))
        md->BeamlineData()["EbeamLTUAngY"]  = beam.fEbeamLTUAngY;
      if(!((0x1 << Pds::BldDataEBeam::EbeamLTUPosX) & beam.uDamageMask))
        md->BeamlineData()["EbeamLTUPosX"]  = beam.fEbeamLTUPosX;
      if(!((0x1 << Pds::BldDataEBeam::EbeamLTUPosY) & beam.uDamageMask))
        md->BeamlineData()["EbeamLTUPosY"]  = beam.fEbeamLTUPosY;
    }
    default:
      break;
    }
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
    /** get the epics header and the epics id for this epics variable */
    const Pds::EpicsPvHeader& epicsData =
        *reinterpret_cast<const Pds::EpicsPvHeader*>(xtc->payload());
    EpicsKey key(xtc->src,epicsData.iPvId);

    /** cntrl is a configuration type and will only be send with a configure
     *  transition
     */
    if ( dbr_type_is_CTRL(epicsData.iDbrType) )
    {
      const Pds::EpicsPvCtrlHeader& ctrl =
          static_cast<const Pds::EpicsPvCtrlHeader&>(epicsData);
      /** create a key for the epics list and index. If this is an additional
       *  list, prepend the detInfo to the epics variable name
       */
      string epicsVariableName(ctrl.sPvName);
      _index2name[key] = epicsVariableName;
      /** now we need to create the map which we will fill later with real values
       *  if this epics variable is an array we want an entry in the map for each entry in the array
       */
      if (ctrl.iNumElements > 1)
      {
        /** go through all entries of the array
         *  create an entry in the map with the the index in brackets
         *  and initialize it with 0
         */
        for (int i=0;i<ctrl.iNumElements;++i)
        {
          std::stringstream entryname;
          entryname << epicsVariableName << "[" << i << "]";
          _store.EpicsData()[entryname.str()] = 0.;
          Log::add(Log::INFO,"MachineData::Converter: '" + entryname.str() + \
                   "' is available in Epics Data");;
        }
      }
      /** otherwise we just add the name to the map and initialze it with 0 */
      else
      {
        _store.EpicsData()[epicsVariableName] = 0.;
        Log::add(Log::INFO,"MachineData::Converter: '" + epicsVariableName +
                 "' is available in Epics Data");
      }
    }
    /** time is the actual data, that will be send down the xtc with 1 Hz */
    else if(dbr_type_is_TIME(epicsData.iDbrType))
    {
      /** now we need to find the variable name in the map, therefore we look up
       *  the name in the indexmap
       */
      string epicsVariableName(_index2name[key]);
      /** if it is an array we added the braces with the array index before,
       *  so we need to add it also now before trying to find the name in the map
       */
      if (epicsData.iNumElements > 1)
        epicsVariableName.append("[0]");
      /** try to find the the name in the map
       *  this returns an iterator to the first entry we found
       *  if it was an array we can then use the iterator to the next values
       */
      MachineDataDevice::epicsDataMap_t::iterator it =
          _store.EpicsData().find(epicsVariableName);
      /** if the name is not in the map, ouput error message */
      if (it == _store.EpicsData().end())
        Log::add(Log::ERROR, "MachineData::Converter: Epics variable with id '" +
                 toString(epicsData.iPvId) + "' was not defined");
      /** otherwise extract the epicsData and write it into the map */
      else
      {
        switch(epicsData.iDbrType)
        {
        case DBR_TIME_SHORT:
          convertEpicsToDouble<DBR_SHORT>(epicsData,it);
          break;
        case DBR_TIME_FLOAT:
          convertEpicsToDouble<DBR_FLOAT>(epicsData,it);
          break;
        case DBR_TIME_ENUM:
          convertEpicsToDouble<DBR_ENUM>(epicsData,it);
          break;
        case DBR_TIME_LONG:
          convertEpicsToDouble<DBR_LONG>(epicsData,it);
          break;
        case DBR_TIME_DOUBLE:
          convertEpicsToDouble<DBR_DOUBLE>(epicsData,it);
          break;
        default:
          break;
        }
      }
    }
    break;
  }
  case(Pds::TypeId::Id_EvrData):
  {
    /** clear the status bytes of the event code */
    std::fill(md->EvrData().begin(),md->EvrData().end(),false);
    /** get the evr data */
    const Pds::EvrData::DataV3 &evrData =
        *reinterpret_cast<const Pds::EvrData::DataV3*>(xtc->payload());
    /** how many events have happened between the last event and now */
    const uint32_t nbrFifoEvents = evrData.numFifoEvents();
    /** go through all events and extract the eventcode from them */
    for (size_t i=0;i<nbrFifoEvents;++i)
    {
      const Pds::EvrData::DataV3::FIFOEvent& fifoEvent = evrData.fifoEvent(i);
      uint32_t eventcode = fifoEvent.EventCode;
      /** check if the array is big enough to hold the recorded eventcode */
      if (md->EvrData().size() < eventcode )
        md->EvrData().resize(eventcode+1,false);
      md->EvrData()[eventcode]=true;
    }
  }
    break;
  case(Pds::TypeId::Id_IpimbData):
  {
    const Pds::DetInfo& info = *(Pds::DetInfo*)(&xtc->src);
    string detector(Pds::DetInfo::name(info.detector()));
    const Pds::Ipimb::DataV2& ipimbData =
        *reinterpret_cast<const Pds::Ipimb::DataV2*>(xtc->payload());
    md->BeamlineData()[detector + "_Channel0"] = ipimbData.channel0Volts();
    md->BeamlineData()[detector + "_Channel1"] = ipimbData.channel1Volts();
    md->BeamlineData()[detector + "_Channel2"] = ipimbData.channel2Volts();
    md->BeamlineData()[detector + "_Channel3"] = ipimbData.channel3Volts();
  }
    break;


  case(Pds::TypeId::Id_IpmFex):
  {
    const Pds::DetInfo& info = *(Pds::DetInfo*)(&xtc->src);
    string detector(Pds::DetInfo::name(info.detector()));
    const Pds::Lusi::IpmFexV1& ipmfex =
        *reinterpret_cast<const Pds::Lusi::IpmFexV1*>(xtc->payload());
    for(size_t i=0; i<Pds::Lusi::IpmFexConfigV2::NCHANNELS; i++)
    {
      stringstream ss;
      ss << detector << "_CorrectChannel" << i;
      md->BeamlineData()[ss.str()] = ipmfex.channel[i];
    }
    md->BeamlineData()[detector + "_sum"]  = ipmfex.sum;
    md->BeamlineData()[detector + "_xPos"] = ipmfex.xpos;
    md->BeamlineData()[detector + "_yPos"] = ipmfex.ypos;
  }
    break;

  case(Pds::TypeId::Id_ControlConfig):
  {
    const Pds::ControlData::ConfigV1& config = *reinterpret_cast<const Pds::ControlData::ConfigV1*>(xtc->payload());
    for (unsigned int i = 0; i < config.npvControls(); i++)
    {
      const Pds::ControlData::PVControl &pvControlCur = config.pvControl(i);
      _pvStore[pvControlCur.name()] = pvControlCur.value();
    }
  }
    break;

  default: break;
  }
  /** copy the epics values in the storedevent to the machineevent */
  if (cassevent)
  {
    md->EpicsData() = _store.EpicsData();
    for(MachineDataDevice::bldMap_t::const_iterator it(_pvStore.begin()); it != _pvStore.end();++it)
    {
      md->BeamlineData()[it->first] = it->second;
    }
  }

}
