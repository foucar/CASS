// Copyright (C) 2010 Lutz Foucar

#include <QtCore/QSettings>
#include <QtCore/QString>

#include "machine_data.h"
#include "histogram.h"
#include "machine_device.h"


// *** postprocessors 850 retrives beamline data ***

cass::pp850::pp850(PostProcessors& pp, cass::PostProcessors::id_t id)
  : PostprocessorBackend(pp, id), _data(0)
{
  loadSettings(0);
}

cass::pp850::~pp850()
{
  _pp.histograms_delete(_id);
  _data = 0;
}

void cass::pp850::loadSettings(size_t)
{
  QSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(QString("p") + QString::number(_id));

  _varname = settings.value("VarName","").toString().toStdString();

  //creat the resulting histogram from the first histogram
  _pp.histograms_delete(_id);
  _data = new Histogram0DFloat();
  _pp.histograms_replace(_id,_data);

  std::cout << "PostProcessor_"<<_id
      <<" will retrieve data \""<<_varname
      <<"\" from beamline data"
      <<std::endl;
}

void cass::pp850::operator()(const CASSEvent& evt)
{
  using namespace cass::MachineData;
  //retrieve beamline data from cassevent
  const MachineDataDevice *mdev
      (dynamic_cast<const MachineDataDevice *>
       (evt.devices().find(CASSEvent::MachineData)->second));
  const MachineDataDevice::bldMap_t bld(mdev->BeamlineData());

  _data->lock.lockForWrite();
  *_data = bld.find(_varname) == bld.end() ? 0: bld.find(_varname)->second;
  _data->lock.unlock();
}













// *** postprocessors 851 retrives beamline data ***

cass::pp851::pp851(PostProcessors& pp, cass::PostProcessors::id_t id)
  : PostprocessorBackend(pp, id), _data(0)
{
  loadSettings(0);
}

cass::pp851::~pp851()
{
  _pp.histograms_delete(_id);
  _data = 0;
}

void cass::pp851::loadSettings(size_t)
{
  QSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(QString("p") + QString::number(_id));

  _varname = settings.value("VarName","").toString().toStdString();

  //creat the resulting histogram from the first histogram
  _pp.histograms_delete(_id);
  _data = new Histogram0DFloat();
  _pp.histograms_replace(_id,_data);

  std::cout << "PostProcessor_"<<_id
      <<" will retrieve data \""<<_varname
      <<"\" from epics data"
      <<std::endl;
}

void cass::pp851::operator()(const CASSEvent& evt)
{
  using namespace cass::MachineData;
  //retrieve beamline data from cassevent
  const MachineDataDevice *mdev
      (dynamic_cast<const MachineDataDevice *>
       (evt.devices().find(CASSEvent::MachineData)->second));
  const MachineDataDevice::epicsDataMap_t epics(mdev->EpicsData());

  _data->lock.lockForWrite();
  *_data = epics.find(_varname) == epics.end() ? 0 : epics.find(_varname)->second;
  _data->lock.unlock();
}













// *** postprocessors 852 retrives beamline data ***

cass::pp852::pp852(PostProcessors& pp, cass::PostProcessors::id_t id)
  : PostprocessorBackend(pp, id), _data(0)
{
  _pp.histograms_delete(_id);
  _data = new Histogram0DFloat();
  _pp.histograms_replace(_id,_data);

  std::cout << "PostProcessor_"<<_id
      <<" calc photonenergy from beamline data"
      <<std::endl;
}

cass::pp852::~pp852()
{
  _pp.histograms_delete(_id);
  _data = 0;
}


void cass::pp852::operator()(const CASSEvent& evt)
{
  using namespace cass::MachineData;
  //retrieve beamline data from cassevent
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

  _data->lock.lockForWrite();
  *_data = photonenergy;
  _data->lock.unlock();
}
