/*
 *  lmf
 */
#include <iostream>

#include "machine_analysis.h"
#include "cass_event.h"
#include "machine_event.h"


void cass::MachineData::Parameter::load()
{
    //sync before loading//
    sync();

}

void cass::MachineData::Parameter::save()
{
}







void cass::MachineData::Analysis::operator()(cass::CASSEvent *cassevent)
{
//    cass::MachineData::MachineDataEvent& machinedataevent = cassevent->MachineDataEvent();
}
