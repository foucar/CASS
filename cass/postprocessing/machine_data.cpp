// Copyright (C) 2010 Lutz Foucar

#include <QtCore/QSettings>
#include <QtCore/QString>

#include "machine_data.h"
#include "histogram.h"
#include "machine_device.h"


// *** postprocessors 120 retrives beamline data ***

cass::pp120::pp120(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key)
{
//  loadSettings(0);
}

void cass::pp120::loadSettings(size_t)
{
  QSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  _varname = settings.value("VariableName","").toString().toStdString();
  if (!setupCondition())
    return;
  _result = new Histogram0DFloat();
  createHistList(2*cass::NbrOfWorkers);
  std::cout << "PostProcessor "<<_key
      <<": will retrieve datafield \""<<_varname
      <<"\" from beamline data"
      <<std::endl;
}

void cass::pp120::process(const CASSEvent& evt)
{
  using namespace cass::MachineData;
  if ((*_condition)(evt).isTrue())
  {
    const MachineDataDevice *mdev
        (dynamic_cast<const MachineDataDevice *>
         (evt.devices().find(CASSEvent::MachineData)->second));
    const MachineDataDevice::bldMap_t bld(mdev->BeamlineData());
    _result->lock.lockForWrite();
    *dynamic_cast<Histogram0DFloat*>(_result) = bld.find(_varname) == bld.end() ? 0: bld.find(_varname)->second;
    _result->lock.unlock();
  }
}













// *** postprocessors 130 retrives epics data ***

cass::pp130::pp130(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key)
{
//  loadSettings(0);
}

void cass::pp130::loadSettings(size_t)
{
  QSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  _varname = settings.value("VariableName","").toString().toStdString();
  if (!setupCondition())
    return;
  _result = new Histogram0DFloat();
  createHistList(2*cass::NbrOfWorkers);
  std::cout << "PostProcessor "<<_key
      <<": will retrieve datafield \""<<_varname
      <<"\" from epics data"
      <<std::endl;
}

void cass::pp130::process(const CASSEvent& evt)
{
  using namespace cass::MachineData;
  if ((*_condition)(evt).isTrue())
  {
    const MachineDataDevice *mdev
        (dynamic_cast<const MachineDataDevice *>
         (evt.devices().find(CASSEvent::MachineData)->second));
    const MachineDataDevice::epicsDataMap_t epics(mdev->EpicsData());
    _result->lock.lockForWrite();
    *dynamic_cast<Histogram0DFloat*>(_result) = epics.find(_varname) == epics.end() ? 0 : epics.find(_varname)->second;
    _result->lock.unlock();
  }
}













// *** postprocessors 230 calcs photonenergy from bld ***

cass::pp230::pp230(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key)
{
  //loadSettings(0);
}

void cass::pp230::loadSettings(size_t)
{
  if (!setupCondition())
    return;
  _result = new Histogram0DFloat();
  createHistList(2*cass::NbrOfWorkers);
  std::cout << "PostProcessor: "<<_key
      <<" calc photonenergy from beamline data"
      <<std::endl;
}

void cass::pp230::process(const CASSEvent& evt)
{
  using namespace cass::MachineData;
  if ((*_condition)(evt).isTrue())
  {
    const MachineDataDevice *mdev
        (dynamic_cast<const MachineDataDevice *>
         (evt.devices().find(CASSEvent::MachineData)->second));
    const MachineDataDevice::bldMap_t bld(mdev->BeamlineData());

    const double ebEnergy
        (bld.find("EbeamL3Energy") == bld.end() ? 0 : bld.find("EbeamL3Energy")->second);
    const double peakCurrent
        (bld.find("EbeamPkCurrBC2") == bld.end() ? 0 : bld.find("EbeamPkCurrBC2")->second);

    //  const double K (3.5);         // K of the undulator (provided by Marc Messerschmidt)
    //  const double lambda (3.0e7);  // LCLS undulator period in nm
    //  const double hc (1239.84172); // in eV*nm
    //  // electron energy in rest mass units (E/mc^2)
    //  double gamma (ebEnergy/(0.510998903));
    //  // resonant photon wavelength in same units as undulator period)
    //  double photonenergy (hc*2*gamma*gamma/(lambda*(1+K*K/2)));

    //=======================================================================================

    // Get present beam energy [GeV]
    const double DL2energyGeV (0.001*ebEnergy);
    // wakeloss prior to undulators
    const double LTUwakeLoss (0.0016293*peakCurrent);
    // Spontaneous radiation loss per segment
    const double SRlossPerSegment (0.63*DL2energyGeV);
    // wakeloss in an undulator segment
    const double wakeLossPerSegment (0.0003*peakCurrent);
    // energy loss per segment
    const double energyLossPerSegment (SRlossPerSegment + wakeLossPerSegment);
    // energy in first active undulator segment [GeV]
    const double energyProfile (DL2energyGeV - 0.001*LTUwakeLoss - 0.0005*energyLossPerSegment);
    // Calculate the resonant photon energy of the first active segment
    const double photonenergy (44.42*energyProfile*energyProfile);

    _result->lock.lockForWrite();
    *dynamic_cast<Histogram0DFloat*>(_result) = photonenergy;
    _result->lock.unlock();
  }
}
