#include "machine_converter.h"

#include "pdsdata/xtc/Xtc.hh"
#include "pdsdata/bld/bldData.hh"
#include "cass_event.h"
#include "machine_event.h"


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
        default: break;
    }
}
