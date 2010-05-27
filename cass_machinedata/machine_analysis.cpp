// Copyright (C) 2010 Lutz Foucar

#include "machine_analysis.h"
#include "cass_event.h"
#include "machine_device.h"


void cass::MachineData::Parameter::load()
{
//  //sync before loading//
//  sync();
//  _lambda = value("UndulatorPeriod_cm",3).toDouble();
//  _K = value("UndulatorK",3.5).toDouble();
}

void cass::MachineData::Parameter::save()
{
//  setValue("UndulatorPeriod_cm",_lambda);
//  setValue("UndulatorK",_K);
}







void cass::MachineData::Analysis::operator()(cass::CASSEvent *cassevent)
{

//  //retrive refernce to the machinedevice//
//  MachineDataDevice& machinedata =
//    *dynamic_cast<MachineDataDevice*>(cassevent->devices()[cass::CASSEvent::MachineData]);
//
//  //This uses Rick K. code at psexport:/reg/neh/home/rkirian/ana2 //
//
//  //Get electron beam parameters from beamline data/
//  //const double fEbeamCharge    = machinedata.EbeamCharge();    // in nC
//  const double fEbeamL3Energy  = machinedata.BeamlineData()["EbeamL3Energy"];  // in MeV
//  const double fEbeamPkCurrBC2 = machinedata.BeamlineData()["EbeamPkCurrBC2"]; // in Amps
//
//  //calculate the resonant photon energy//
//  // Get the present peak current in Amps
//  const double peakCurrent = fEbeamPkCurrBC2;
//  // Get present beam energy [GeV]
//  const double DL2energyGeV = 0.001*fEbeamL3Energy;
//  // wakeloss prior to undulators
//  const double LTUwakeLoss = 0.0016293*peakCurrent;
//  // Spontaneous radiation loss per segment
//  const double SRlossPerSegment = 0.63*DL2energyGeV;
//  // wakeloss in an undulator segment
//  const double wakeLossPerSegment = 0.0003*peakCurrent;
//  // energy loss per segment
//  const double energyLossPerSegment = SRlossPerSegment + wakeLossPerSegment;
//
//  // energy in first active undulator segment [GeV]
//  const double energyProfile = DL2energyGeV - 0.001*LTUwakeLoss - 0.0005*energyLossPerSegment;
//
//  // Calculate the resonant photon energy of the first active segment
//  // and the corrosponding wavelength
//  machinedata.energy() = 44.42*energyProfile*energyProfile;
//  machinedata.wavelength() =  1398.8/machinedata.energy();
}

