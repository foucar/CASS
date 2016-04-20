// Copyright (C) 2009 - 2014 Lutz Foucar

/**
 * @file machine_converter.cpp contains xtc converter for machine data
 *
 * @author Lutz Foucar
 */


#include <sstream>
#include <iostream>

#include "machine_converter.h"

#include "pdsdata/xtc/Xtc.hh"
#include "pdsdata/xtc/BldInfo.hh"
#include "pdsdata/bld/bldData.hh"
#include "pdsdata/epics/EpicsPvData.hh"
#include "pdsdata/evr/DataV3.hh"
#include "pdsdata/ipimb/DataV2.hh"
#include "pdsdata/xtc/DetInfo.hh"
#include "pdsdata/lusi/IpmFexV1.hh"
#include "pdsdata/lusi/IpmFexConfigV2.hh"
#include "pdsdata/control/ConfigV1.hh"
#include "pdsdata/control/PVControl.hh"
#include "pdsdata/psddl/usdusb.ddl.h"

#include "cass_event.h"
#include "log.h"
#include "machine_device.hpp"

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
                          Device::epicsDataMap_t::iterator first)
{
  const Pds::EpicsPvTime<valtype> &p
      (reinterpret_cast<const Pds::EpicsPvTime<valtype>&>(epicsData));
  const typename Pds::EpicsDbrTools::DbrTypeFromInt<valtype>::TDbr* value(&p.value);
  for(int i=0; i<epicsData.iNumElements; ++i)
    first++->second = *value++;
}

/** convert epics variable to double and fill store and cassevent
 *
 * convert the value contained in the Epics variable to a double and fill it
 * into the map. One has to define beforehand where to add it and pass the right
 * iterator to the function
 *
 * @tparam valtype the value type of the epics variable
 * @param epicsData the object that contains the epics data
 * @param storefirst iterator to the first entry in the epics part of store
 * @param cassfirst iterator to the first entry in the epics part of cassevent
 *
 * @author Lutz Foucar
 */
template <int valtype>
void epicsValToCassVal(const Pds::EpicsPvHeader& epicsData,
                       Device::epicsDataMap_t::iterator storefirst,
                       Device::epicsDataMap_t::iterator cassfirst)
{
  const Pds::EpicsPvTime<valtype> &p
      (reinterpret_cast<const Pds::EpicsPvTime<valtype>&>(epicsData));
  const typename Pds::EpicsDbrTools::DbrTypeFromInt<valtype>::TDbr* value(&p.value);
  for(int i=0; i<epicsData.iNumElements; ++i)
  {
    const double val(*value++);
    storefirst++->second = val;
    cassfirst++->second = val;
  }
}

/** convert epics variable to nothing
 *
 * don't do anything. This function is needed, because string types can't be
 * converted, therefore this is used as dummy for epics string types.
 *
 * @tparam valtype the value type of the epics variable
 * @param epicsData the object that contains the epics data
 * @param storefirst iterator to the first entry in the epics part of store
 * @param cassfirst iterator to the first entry in the epics part of cassevent
 *
 * @author Lutz Foucar
 */
void epicsValToNothing(const Pds::EpicsPvHeader& /*epicsData*/,
                       Device::epicsDataMap_t::iterator /*storefirst*/,
                       Device::epicsDataMap_t::iterator /*cassfirst*/)

{

}

/** Key for the xtc data lookup map
 *
 * Key for mapping Epics index and list to a specific name
 *
 * @author Lutz Foucar
 */
class XTCDataKey
{
public:
  /** constructor
   *
   * @param src the info about the source of the xtc data
   * @param index the index of the the xtc data in the list
   */
  XTCDataKey(const Pds::Src &src, uint32_t index)
    : _key(((static_cast<uint64_t>(src.phy())&0xffffffff)<<32) | (index&0xffffffff))
  {}

  /** check whether this is less than other
   *
   * @return true when this is smaller than other
   * @param other the other key that one compares this key to
   */
  bool operator <(const XTCDataKey& other) const
  {
    return _key < other._key;
  }

private:
  /** the combined key */
  uint64_t _key;

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
  _pdsTypeList.push_back(Pds::TypeId::Id_Spectrometer);
  _pdsTypeList.push_back(Pds::TypeId::Id_UsdUsbFexConfig);
  _pdsTypeList.push_back(Pds::TypeId::Id_UsdUsbFexData);

  _epicsType2convFunc[DBR_TIME_SHORT] = &epicsValToCassVal<DBR_SHORT>;
  _epicsType2convFunc[DBR_TIME_FLOAT] = &epicsValToCassVal<DBR_FLOAT>;
  _epicsType2convFunc[DBR_TIME_ENUM] = &epicsValToCassVal<DBR_ENUM>;
  _epicsType2convFunc[DBR_TIME_LONG] = &epicsValToCassVal<DBR_LONG>;
  _epicsType2convFunc[DBR_TIME_DOUBLE] = &epicsValToCassVal<DBR_DOUBLE>;
  _epicsType2convFunc[DBR_TIME_STRING] = &epicsValToNothing;
}

void cass::MachineData::Converter::operator()(const Pds::Xtc* xtc, cass::CASSEvent* cassevent)
{

  switch (xtc->contains.id())
  {


  case(Pds::TypeId::Id_FEEGasDetEnergy):
  {
    Device &md(dynamic_cast<Device&>(*(cassevent->devices()[cass::CASSEvent::MachineData])));
    uint32_t version (xtc->contains.version());
    const Pds::BldDataFEEGasDetEnergy &gasdet =
        *reinterpret_cast<const Pds::BldDataFEEGasDetEnergy*>(xtc->payload());
    switch (version)
    {
    case(1):
      md.BeamlineData()["f_63_ENRC"] = gasdet.f_63_ENRC;
      md.BeamlineData()["f_64_ENRC"] = gasdet.f_64_ENRC;
    case(0):
      md.BeamlineData()["f_11_ENRC"] = gasdet.f_11_ENRC;
      md.BeamlineData()["f_12_ENRC"] = gasdet.f_12_ENRC;
      md.BeamlineData()["f_21_ENRC"] = gasdet.f_21_ENRC;
      md.BeamlineData()["f_22_ENRC"] = gasdet.f_22_ENRC;
      break;
    default:
      Log::add(Log::ERROR,"Unknown FEEGasDet version");
      break;
    }
  }


  case(Pds::TypeId::Id_EBeam):
  {
    Device &md(dynamic_cast<Device&>(*(cassevent->devices()[cass::CASSEvent::MachineData])));
    uint32_t version (xtc->contains.version());
    const Pds::BldDataEBeam &beam =
        *reinterpret_cast<const Pds::BldDataEBeam*>(xtc->payload());
    switch (version)
    {
    default:
      Log::add(Log::WARNING,"Unknown BLD version '" + toString(version) +
               "' using latest known version '7'");
    /** BldDataEBeamV7 is the same as BldDataEBeamV6.
     *  A sign-error error was discovered in the calculation of the photon
     *  energy that goes into the ebeam bld. This is fixed on the accelerator
     *  side, but we will increment the ebeam bld version number to V7 so the
     *  data is clearly marked as changed.
     */
    case (7):
    case (6):
    {
      if(!(Pds::BldDataEBeam::EbeamPhotonEnergyDamage & beam.uDamageMask))
        md.BeamlineData()["EbeamPhotonEnergy"]= beam.fEbeamPhotonEnergy;
      else
        Log::add(Log::VERBOSEINFO,"'EbeamPhotonEnergy' is damaged");

//      if(!(Pds::BldDataEBeam::EbeamXTCAVPhaseDamage & beam.uDamageMask))
        md.BeamlineData()["fEbeamLTU250"]= beam.fEbeamLTU250;
//      else
//        Log::add(Log::VERBOSEINFO,"'EbeamXTCAVPhase' is damaged");

//      if(!(Pds::BldDataEBeam::EbeamDumpChargeDamage & beam.uDamageMask))
        md.BeamlineData()["EbeamLTU450"]= beam.fEbeamLTU450;
//      else
//        Log::add(Log::VERBOSEINFO,"'EbeamDumpCharge' is damaged");

    }
    case (5):
    {
      if(!(Pds::BldDataEBeam::EbeamXTCAVAmplDamage & beam.uDamageMask))
        md.BeamlineData()["EbeamXTCAVAmpl"]= beam.fEbeamXTCAVAmpl;
      else
        Log::add(Log::VERBOSEINFO,"'EbeamXTCAVAmpl' is damaged");

      if(!(Pds::BldDataEBeam::EbeamXTCAVPhaseDamage & beam.uDamageMask))
        md.BeamlineData()["EbeamXTCAVPhase"]= beam.fEbeamXTCAVPhase;
      else
        Log::add(Log::VERBOSEINFO,"'EbeamXTCAVPhase' is damaged");

      if(!(Pds::BldDataEBeam::EbeamDumpChargeDamage & beam.uDamageMask))
        md.BeamlineData()["EbeamDumpCharge"]= beam.fEbeamDumpCharge;
      else
        Log::add(Log::VERBOSEINFO,"'EbeamDumpCharge' is damaged");
    }
    case (4):
    {
      if(!(Pds::BldDataEBeam::EbeamUndPosXDamage & beam.uDamageMask))
        md.BeamlineData()["EbeamUndPosX"]= beam.fEbeamUndPosX;
      else
        Log::add(Log::VERBOSEINFO,"'EbeamUndPosX' is damaged");

      if(!(Pds::BldDataEBeam::EbeamUndPosYDamage & beam.uDamageMask))
        md.BeamlineData()["EbeamUndPosY"]= beam.fEbeamUndPosY;
      else
        Log::add(Log::VERBOSEINFO,"'EbeamUndPosY' is damaged");

      if(!(Pds::BldDataEBeam::EbeamUndAngXDamage & beam.uDamageMask))
        md.BeamlineData()["EbeamUndAngX"]= beam.fEbeamUndAngX;
      else
        Log::add(Log::VERBOSEINFO,"'EbeamUndPosX' is damaged");

      if(!(Pds::BldDataEBeam::EbeamUndAngYDamage & beam.uDamageMask))
        md.BeamlineData()["EbeamUndAngY"]= beam.fEbeamUndAngY;
      else
        Log::add(Log::VERBOSEINFO,"'EbeamUndPosY' is damaged");
    }
    case (3):
    {
      if(!(Pds::BldDataEBeam::EbeamPkCurrBC1Damage & beam.uDamageMask))
        md.BeamlineData()["EbeamPkCurrBC1"]= beam.fEbeamPkCurrBC1;
      else
        Log::add(Log::VERBOSEINFO,"'EbeamPkCurrBC1' is damaged");

      if(!(Pds::BldDataEBeam::EbeamEnergyBC1Damage & beam.uDamageMask))
        md.BeamlineData()["fEbeamEnergyBC1"]= beam.fEbeamEnergyBC1;
      else
        Log::add(Log::VERBOSEINFO,"'fEbeamEnergyBC1' is damaged");
    }
    case (2):
    {
      if(!(Pds::BldDataEBeam::EbeamEnergyBC2Damage & beam.uDamageMask))
        md.BeamlineData()["EbeamEnergyBC2"]= beam.fEbeamEnergyBC2;
      else
        Log::add(Log::VERBOSEINFO,"'EbeamEnergyBC2' is damaged");
    }
    case (1):
    {
      if(!(Pds::BldDataEBeam::EbeamPkCurrBC2Damage & beam.uDamageMask))
        md.BeamlineData()["EbeamPkCurrBC2"]= beam.fEbeamPkCurrBC2;
      else
        Log::add(Log::VERBOSEINFO,"'EbeamPkCurrBC2' is damaged");
    }
    case (0):
    {
      if(!(Pds::BldDataEBeam::EbeamChargeDamage & beam.uDamageMask))
        md.BeamlineData()["EbeamCharge"]   = beam.fEbeamCharge;
      else
        Log::add(Log::VERBOSEINFO,"'EbeamCharge' is damaged");

      if(!(Pds::BldDataEBeam::EbeamL3EnergyDamage & beam.uDamageMask))
        md.BeamlineData()["EbeamL3Energy"] = beam.fEbeamL3Energy;
      else
        Log::add(Log::VERBOSEINFO,"'EbeamL3Energy' is damaged");

      if(!(Pds::BldDataEBeam::EbeamLTUAngXDamage & beam.uDamageMask))
        md.BeamlineData()["EbeamLTUAngX"]  = beam.fEbeamLTUAngX;
      else
        Log::add(Log::VERBOSEINFO,"'EbeamLTUAngX' is damaged");

      if(!(Pds::BldDataEBeam::EbeamLTUAngYDamage & beam.uDamageMask))
        md.BeamlineData()["EbeamLTUAngY"]  = beam.fEbeamLTUAngY;
      else
        Log::add(Log::VERBOSEINFO,"'EbeamLTUAngY' is damaged");

      if(!(Pds::BldDataEBeam::EbeamLTUPosXDamage & beam.uDamageMask))
        md.BeamlineData()["EbeamLTUPosX"]  = beam.fEbeamLTUPosX;
      else
        Log::add(Log::VERBOSEINFO,"'EbeamLTUPosX' is damaged");

      if(!(Pds::BldDataEBeam::EbeamLTUPosYDamage & beam.uDamageMask))
        md.BeamlineData()["EbeamLTUPosY"]  = beam.fEbeamLTUPosY;
      else
        Log::add(Log::VERBOSEINFO,"'EbeamLTUPosY' is damaged");
      break;
    }
    }
    break;
  }


  case(Pds::TypeId::Id_PhaseCavity):
  {
    Device &md(dynamic_cast<Device&>(*(cassevent->devices()[cass::CASSEvent::MachineData])));
    const Pds::BldDataPhaseCavity &cavity =
        *reinterpret_cast<const Pds::BldDataPhaseCavity*>(xtc->payload());
    md.BeamlineData()["Charge1"]  = cavity.fCharge1;
    md.BeamlineData()["Charge2"]  = cavity.fCharge2;
    md.BeamlineData()["FitTime1"] = cavity.fFitTime1;
    md.BeamlineData()["FitTime2"] = cavity.fFitTime2;
    break;
  }


  case(Pds::TypeId::Id_Epics):
  {
    /** need to lock this operation as it involves the store used by all */
    QMutexLocker lock(&_mutex);

    /** get the epics header and the epics id for this epics variable */
    const Pds::EpicsPvHeader& epicsData =
        *reinterpret_cast<const Pds::EpicsPvHeader*>(xtc->payload());
    XTCDataKey key(xtc->src,epicsData.iPvId);

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
          Log::add(Log::INFO,"MachineData::Converter: '" + entryname.str() +
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
      Device &md(dynamic_cast<Device&>(*(cassevent->devices()[cass::CASSEvent::MachineData])));

      /** now we need to find the variable name in the map, therefore we look up
       *  the name in the indexmap
       */
      KeyMap_t::const_iterator eIt(_index2name.find(key));
      if (eIt == _index2name.end())
        Log::add(Log::ERROR, "MachineData::Converter: Epics variable with id '" +
                 toString(epicsData.iPvId) + "' was not defined");
      else
      {
        string epicsVariableName(eIt->second);

        /** if it is an array go through all entries of the array create an entry
         *  in the map with the the index in brackets and initialize it with 0
         */
        if (epicsData.iNumElements > 1)
          epicsVariableName.append("[0]");

        /** try to find the the name in the map
         *  this returns an iterator to the first entry we found
         *  if it was an array we can then use the iterator to the next values
         */
        Device::epicsDataMap_t::iterator storeIt =
            _store.EpicsData().find(epicsVariableName);
        Device::epicsDataMap_t::iterator cassIt =
            md.EpicsData().find(epicsVariableName);
        /** if the name is not in the map, ouput error message */
        if (storeIt == _store.EpicsData().end() || cassIt == md.EpicsData().end())
          Log::add(Log::ERROR, "MachineData::Converter: Epics variable with id '" +
                   toString(epicsData.iPvId) + "' was not found in store or cassevent");
        /** otherwise extract the epicsData and write it into the map */
        else
        {
          _epicsType2convFunc[epicsData.iDbrType](epicsData,storeIt,cassIt);
        }
        /** set the variable that the epics store was filled */
        md.epicsFilled() = true;
      }
    }
    break;
  }


  case(Pds::TypeId::Id_EvrData):
  {
    Device &md(dynamic_cast<Device&>(*(cassevent->devices()[cass::CASSEvent::MachineData])));
    /** clear the status bytes of the event code */
    std::fill(md.EvrData().begin(),md.EvrData().end(),false);
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
      if (md.EvrData().size() < eventcode )
        md.EvrData().resize(eventcode+1,false);
      md.EvrData()[eventcode]=true;
    }
    break;
  }


  case(Pds::TypeId::Id_IpimbData):
  {
    Device &md(dynamic_cast<Device&>(*(cassevent->devices()[cass::CASSEvent::MachineData])));
    const Pds::DetInfo& info = *(Pds::DetInfo*)(&xtc->src);
    string detector(Pds::DetInfo::name(info.detector()));
    const Pds::Ipimb::DataV2& ipimbData =
        *reinterpret_cast<const Pds::Ipimb::DataV2*>(xtc->payload());
    md.BeamlineData()[detector + "_Channel0"] = ipimbData.channel0Volts();
    md.BeamlineData()[detector + "_Channel1"] = ipimbData.channel1Volts();
    md.BeamlineData()[detector + "_Channel2"] = ipimbData.channel2Volts();
    md.BeamlineData()[detector + "_Channel3"] = ipimbData.channel3Volts();
  }
    break;


  case(Pds::TypeId::Id_IpmFex):
  {
    Device &md(dynamic_cast<Device&>(*(cassevent->devices()[cass::CASSEvent::MachineData])));
    const Pds::DetInfo& info = *(Pds::DetInfo*)(&xtc->src);
    string detector(Pds::DetInfo::name(info.detector()));
    const Pds::Lusi::IpmFexV1& ipmfex =
        *reinterpret_cast<const Pds::Lusi::IpmFexV1*>(xtc->payload());
    for(size_t i=0; i<Pds::Lusi::IpmFexConfigV2::NCHANNELS; i++)
    {
      stringstream ss;
      ss << detector << "_CorrectChannel" << i;
      md.BeamlineData()[ss.str()] = ipmfex.channel[i];
    }
    md.BeamlineData()[detector + "_sum"]  = ipmfex.sum;
    md.BeamlineData()[detector + "_xPos"] = ipmfex.xpos;
    md.BeamlineData()[detector + "_yPos"] = ipmfex.ypos;
    break;
  }


  case(Pds::TypeId::Id_ControlConfig):
  {
    QMutexLocker lock(&_mutex);
    Device &md(dynamic_cast<Device&>(*(cassevent->devices()[cass::CASSEvent::MachineData])));
    /** add variables to store, cassevent and to log */
    const Pds::ControlData::ConfigV1& config = *reinterpret_cast<const Pds::ControlData::ConfigV1*>(xtc->payload());
    string log("MachineData::Converter: Calibcylce: [" +
               toString(config.npvControls())+ " values]: ");
    for (unsigned int i = 0; i < config.npvControls(); i++)
    {
      const Pds::ControlData::PVControl &pvControlCur = config.pvControl(i);
      _store.BeamlineData()[pvControlCur.name()] = pvControlCur.value();
      if(cassevent)
        md.BeamlineData()[pvControlCur.name()] = pvControlCur.value();
      log += string(pvControlCur.name()) + " = " + toString(pvControlCur.value()) + "; ";
    }
    Log::add(Log::INFO,log);
    break;
  }


  case(Pds::TypeId::Id_Spectrometer):
  {
    Device &md(dynamic_cast<Device&>(*(cassevent->devices()[cass::CASSEvent::MachineData])));
    uint32_t version(xtc->contains.version());
    string specname
        (Pds::BldInfo::name(reinterpret_cast<const Pds::BldInfo&>(xtc->src)));
    Device::spectrometer_t::mapped_type &horiz
        (md.spectrometers()[specname +"_horiz"]);
    Device::spectrometer_t::mapped_type &vert
        (md.spectrometers()[specname +"_vert"]);
    switch (version)
    {
    case (0):
    {
      const Pds::BldDataSpectrometerV0 &spec
          (*reinterpret_cast<const Pds::BldDataSpectrometerV0*>(xtc->payload()));
      horiz.assign(spec._hproj,spec._hproj+1024);
      vert.assign(spec._vproj,spec._vproj+256);
      break;
    }
    case (1):
    {
      const Pds::BldDataSpectrometerV1 *spec
          (reinterpret_cast<const Pds::BldDataSpectrometerV1*>(xtc->payload()));
      const uint32_t* first(reinterpret_cast<const uint32_t*>(reinterpret_cast<const char*>(spec)+48));
      const uint32_t* last(first + spec->_width);
      horiz.assign(first,last);
      break;
    }
    default:
      Log::add(Log::ERROR,"Unknown Spectrometer Version '" + toString(version) +
               "'. Skipping reading of Spectrometer data.");
      break;
    }
      break;
  }

  default: break;
  }//end switch
}

void Converter::prepare(cass::CASSEvent *evt)
{
  /** clear the beamline data by setting every value to 0
   *  and reset the filled flag
   *
   *  @note clearing is needed to be done at this point, because the map will
   *        be updated multiple times during the conversion process and
   *        therefore clearing it during the conversion will erase variables
   *        that have already been set.
   */
  if (evt)
  {
    QMutexLocker lock(&_mutex);
    Device &md(dynamic_cast<Device&>(*(evt->devices()[cass::CASSEvent::MachineData])));
    Device::bldMap_t::iterator bi (md.BeamlineData().begin());
    Device::bldMap_t::const_iterator bEnd (md.BeamlineData().end());
    for (; bi != bEnd ;++bi)
      bi->second = 0;
    md.epicsFilled() = false;
    /** copy values in the store to the event */
    md.EpicsData() = _store.EpicsData();
    /** @note we want to add the addional values that are in the store to
     *        the beamline data of the event therefore we should not use the
     *        assignment operator here
     */
    Device::bldMap_t::const_iterator it (_store.BeamlineData().begin());
    Device::bldMap_t::const_iterator End (_store.BeamlineData().end());
    for(; it != End; ++it)
      md.BeamlineData()[it->first] = it->second;
  }
}

void Converter::finalize(CASSEvent* /*evt*/)
{
}
