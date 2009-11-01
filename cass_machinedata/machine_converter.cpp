#include <sstream>
#include <iostream>

#include "machine_converter.h"

#include "pdsdata/xtc/Xtc.hh"
#include "pdsdata/bld/bldData.hh"
#include "pdsdata/epics/EpicsPvData.hh"
#include "cass_event.h"
#include "machine_event.h"



////little helperfunction to extract the values out of the epcis data thingi//
//template <int iDbrID>
//static void extractEpicsValue(cass::MachineData::MachineDataEvent::EpicsDataMap::iterator& it, const Pds::EpicsPvHeader &epicsHeader)
//{
//    const Pds::EpicsPvTime<iDbrID>& epicsTime = static_cast<const Pds::EpicsPvTime<iDbrID>&>(epicsHeader);
//    const Pds::EpicsDbrTools::DbrTypeFromInt<iDbrID>::TDbr* value = &epicsTime.value;
//    for (int i=0; i<epicsHeader.iNumElements;++i)
//        it->second = *value++;
//}

//the above doesn't work, need to find out why. For now we just use the code copied from matt weaver
#define CASETOVAL(timetype,valtype) case timetype: {			\
    const Pds::EpicsPvTime<valtype>& p = static_cast<const Pds::EpicsPvTime<valtype>&>(epicsData); \
    const Pds::EpicsDbrTools::DbrTypeFromInt<valtype>::TDbr* value = &p.value;	\
    for(int i=0; i<epicsData.iNumElements; i++)				\
        it->second = *value++; 					\
    break; }


cass::MachineData::Converter::Converter()
{
    _types.push_back(Pds::TypeId::Id_FEEGasDetEnergy);
    _types.push_back(Pds::TypeId::Id_EBeam);
    _types.push_back(Pds::TypeId::Id_PhaseCavity);
    _types.push_back(Pds::TypeId::Id_Epics);
}

void cass::MachineData::Converter::operator()(const Pds::Xtc* xtc, cass::CASSEvent* cassevent)
{
    MachineDataEvent &machinedataevent = cassevent->MachineDataEvent();
    machinedataevent.isFilled() = true;

    switch (xtc->contains.id())
    {
        case(Pds::TypeId::Id_FEEGasDetEnergy):
        {
            const Pds::BldDataFEEGasDetEnergy &gasdet = *reinterpret_cast<const Pds::BldDataFEEGasDetEnergy*>(xtc->payload());
            machinedataevent.f_11_ENRC() = gasdet.f_11_ENRC;
            machinedataevent.f_12_ENRC() = gasdet.f_12_ENRC;
            machinedataevent.f_21_ENRC() = gasdet.f_21_ENRC;
            machinedataevent.f_22_ENRC() = gasdet.f_22_ENRC;
            break;
        }
        case(Pds::TypeId::Id_EBeam):
        {
            const Pds::BldDataEBeam &beam = *reinterpret_cast<const Pds::BldDataEBeam*>(xtc->payload());
            machinedataevent.EbeamCharge()   = beam.fEbeamCharge;
            machinedataevent.EbeamL3Energy() = beam.fEbeamL3Energy;
            machinedataevent.EbeamLTUAngX()  = beam.fEbeamLTUAngX;
            machinedataevent.EbeamLTUAngY()  = beam.fEbeamLTUAngY;
            machinedataevent.EbeamLTUPosX()  = beam.fEbeamLTUPosX;
            machinedataevent.EbeamLTUPosY()  = beam.fEbeamLTUPosY;
            break;
        }
        case(Pds::TypeId::Id_PhaseCavity):
        {
            const Pds::BldDataPhaseCavity &cavity = *reinterpret_cast<const Pds::BldDataPhaseCavity*>(xtc->payload());
            machinedataevent.Charge1()  = cavity.fCharge1;
            machinedataevent.Charge2()  = cavity.fCharge2;
            machinedataevent.FitTime1() = cavity.fFitTime1;
            machinedataevent.FitTime2() = cavity.fFitTime2;
            break;
        }
        case(Pds::TypeId::Id_Epics):
        {
            const Pds::EpicsPvHeader& epicsData = *reinterpret_cast<const Pds::EpicsPvHeader*>(xtc->payload());
            //cntrl is a configuration type and will only be send with a configure transition//
            if ( dbr_type_is_CTRL(epicsData.iDbrType) )
            {
                const Pds::EpicsPvCtrlHeader& ctrl = static_cast<const Pds::EpicsPvCtrlHeader&>(epicsData);
                //record what name the pvId has, this help later to find the name, which is the index of map in machineevent//
                _index2name[ctrl.iPvId] = ctrl.sPvName;
                //now we need to create the map which we will fill later with real values//
                //if this epics variable is an array we want an entry in the map for each entry in the array//
                if (ctrl.iNumElements > 1)
                {
                    //go through all entries of the array//
                    //create an entry in the map with the the index in brackets//
                    //and initialize it with 0//
                    for (int i=0;i<ctrl.iNumElements;++i)
                    {
                        std::stringstream entryname;
                        entryname << ctrl.sPvName << "[" << i << "]";
                        machinedataevent.EpicsData()[entryname.str()] = 0.;
                    }
                }
                //otherwise we just add the name to the map and initialze it with 0//
                else
                {
                    machinedataevent.EpicsData()[ctrl.sPvName] = 0.;
                }

            }
            //time is the actual data, that will be send down the xtc with 1 Hz
            else if(dbr_type_is_TIME(epicsData.iDbrType))
            {
                //now we need to find the variable name in the map//
                //therefore we look up the name in the indexmap//
                std::string name = _index2name[epicsData.iPvId];
                //if it is an array we added the braces with the array index before,
                //so we need to add it also now before trying to find the name in the map//
                if (epicsData.iNumElements > 1)
                    name.append("[0]");
                //try to find the the name in the map//
                //this returns an iterator to the first entry we found//
                //if it was an array we can then use the iterator to the next values//
                MachineDataEvent::EpicsDataMap::iterator it = machinedataevent.EpicsData().find(name);
                //if the name is not in the map//
                //then output an erromessage//
                if (it == machinedataevent.EpicsData().end())
                    std::cout << "epics variable with id "<<epicsData.iPvId<<" was not defined"<<std::endl;
                //otherwise extract the epicsData and write it into the map
                else
                {
                    switch(epicsData.iDbrType)
                    {
//                        case(DBR_TIME_SHORT) : extractEpicsValue<DBR_SHORT> (it,epicsData); break;
//                        case(DBR_TIME_FLOAT) : extractEpicsValue<DBR_FLOAT> (it,epicsData); break;
//                        case(DBR_TIME_ENUM)  : extractEpicsValue<DBR_ENUM>  (it,epicsData); break;
//                        case(DBR_TIME_LONG)  : extractEpicsValue<DBR_LONG>  (it,epicsData); break;
//                        case(DBR_TIME_DOUBLE): extractEpicsValue<DBR_DOUBLE>(it,epicsData); break;
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

        default: break;
    }
}
