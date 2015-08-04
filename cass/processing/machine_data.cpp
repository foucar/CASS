// Copyright (C) 2010 Lutz Foucar

/**
 * @file machine_data.cpp file contains definition of processors that
 *                        extract information from the beamline and epics data.
 *
 * @author Lutz Foucar
 */

#include <QtCore/QString>

#include "machine_data.h"
#include "histogram.h"
#include "machine_device.hpp"
#include "cass_settings.h"
#include "log.h"
#include "cass_exceptions.hpp"


using namespace cass;
using namespace MachineData;
using namespace std;
using namespace std::tr1;

// *** processors 120 retrives beamline data ***

pp120::pp120(const name_t &name)
  : Processor(name)
{
  loadSettings(0);
}

void pp120::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("Processor");
  s.beginGroup(name().c_str());
  _varname = s.value("VariableName","").toString().toStdString();
  setupGeneral();
  if (!setupCondition())
    return;

  createHistList(tr1::shared_ptr<Histogram0DFloat>(new Histogram0DFloat()));

  Log::add(Log::INFO,"Processor '" + name() + "': will retrieve datafield ' " +
           _varname + "' from beamline data. Condition is '" + _condition->name() +"'");
}

void pp120::process(const CASSEvent& evt, HistogramBackend &res)
{
  const Device &md
      (dynamic_cast<const Device&>
       (*(evt.devices().find(CASSEvent::MachineData)->second)));
  const Device::bldMap_t &bld(md.BeamlineData());
  Histogram0DFloat &result(dynamic_cast<Histogram0DFloat&>(res));
  result = bld.find(_varname) == bld.end() ? 0: bld.find(_varname)->second;
}








// *** processors 121 checks event code ***

pp121::pp121(const name_t &name)
  : Processor(name)
{
  loadSettings(0);
}

void pp121::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("Processor");
  s.beginGroup(name().c_str());
  _eventcode = s.value("EventCode",0).toUInt();
  setupGeneral();
  if (!setupCondition())
    return;
  createHistList(tr1::shared_ptr<Histogram0DFloat>(new Histogram0DFloat()));

  Log::add(Log::INFO,"Processor '"+ name() + "' will check whether event code '" +
           toString(_eventcode) + "' is present in the event. Condition is '" +
           _condition->name() +"'");
}

void pp121::process(const CASSEvent& evt, HistogramBackend &res)
{
  using namespace MachineData;
  const Device &md
      (dynamic_cast<const Device&>
       (*(evt.devices().find(CASSEvent::MachineData)->second)));
  const Device::evrStatus_t &evr(md.EvrData());
  Histogram0DFloat &result(dynamic_cast<Histogram0DFloat&>(res));
  result = ((_eventcode < evr.size()) ? evr[_eventcode] : 0);
}












// *** processors 122 retrieve eventID ***

pp122::pp122(const name_t &name)
  : Processor(name)
{
  loadSettings(0);
}

void pp122::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("Processor");
  s.beginGroup(name().c_str());

  setupGeneral();
  if (!setupCondition())
    return;
  _part = s.value("EventIDPart",0).toInt();

  createHistList(tr1::shared_ptr<Histogram0DFloat>(new Histogram0DFloat()));

  Log::add(Log::INFO,"Processor '" + name() + "' will retrieve the event ID from events. Condition is '" +
           _condition->name() + "'");
}

void pp122::process(const CASSEvent& evt, HistogramBackend &res)
{
  Histogram0DFloat &result(dynamic_cast<Histogram0DFloat&>(res));
  if (_part == lower)
    result = evt.id() & 0x00000000FFFFFFFF;
  if (_part == upper)
    result = (evt.id() & 0xFFFFFFFF00000000) >> 32;
  else
    result = evt.id();
}















// *** processors 123 retrieve beamline spectrometer data ***

pp123::pp123(const name_t &name)
  : Processor(name)
{
  loadSettings(0);
}

void pp123::loadSettings(size_t)
{
  setupGeneral();
  if (!setupCondition())
    return;
  CASSSettings s;
  s.beginGroup("Processor");
  s.beginGroup(name().c_str());
  _specname = s.value("SpectrometerName","").toString().toStdString();
  int size(0);
  if (_specname.find("horiz") != string::npos)
    size = 1024;
  if (_specname.find("vert") != string::npos)
    size = 256;
  size = s.value("Size",size).toInt();
  createHistList(
        tr1::shared_ptr<Histogram1DFloat>
        (new Histogram1DFloat(size,0,size,"arb. units")));

  Log::add(Log::INFO,"Processor '" + name() + "' will retrieve the beamline spectrometer data '"+
           _specname +"'.Condition is '" + _condition->name() + "'");
}

void pp123::process(const CASSEvent& evt, HistogramBackend &res)
{
  using namespace MachineData;
  const Device &mdev
      (dynamic_cast<const Device&>
       (*(evt.devices().find(CASSEvent::MachineData)->second)));
  const Device::spectrometer_t &spectros(mdev.spectrometers());
  Device::spectrometer_t::const_iterator specIt(spectros.find(_specname));
  if (specIt == spectros.end())
    throw InvalidData("pp123::process (" + name() + "): Spectrometer with name '" +
                      _specname +"' is unknown.");
  const Device::spectrometer_t::mapped_type &spec(specIt->second);

  Histogram1DFloat &result(dynamic_cast<Histogram1DFloat&>(res));
  if (result.memory().size() < spec.size())
    throw logic_error("pp123:process (" + name() +
                      "): Result with size '" + toString(result.memory().size()) +
                      "'is not large enough to handle spectrometer data with size" +
                      toString(spec.size()) + "'");
  copy(spec.begin(),spec.end(),result.memory().begin());
  result.nbrOfFills() = 1;
}












// *** processors 130 retrives epics data ***

pp130::pp130(const name_t &name)
  : Processor(name)
{
  loadSettings(0);
}

void pp130::loadSettings(size_t)
{
  CASSSettings settings;
  settings.beginGroup("Processor");
  settings.beginGroup(name().c_str());
  _varname = settings.value("VariableName","").toString().toStdString();
  setupGeneral();
  if (!setupCondition())
    return;
  createHistList(tr1::shared_ptr<Histogram0DFloat>(new Histogram0DFloat()));
  Log::add(Log::INFO,"Processor '" + name() + "' will retrieve datafield' " +
           _varname +"' from epics data. Condition is" + _condition->name() + "'");
}

void pp130::process(const CASSEvent& evt, HistogramBackend &res)
{
  using namespace MachineData;
  const Device &md
      (dynamic_cast<const Device&>
       (*(evt.devices().find(CASSEvent::MachineData)->second)));
  const Device::epicsDataMap_t &epics(md.EpicsData());
  Histogram0DFloat &result(dynamic_cast<Histogram0DFloat&>(res));
  result = epics.find(_varname) == epics.end() ? 0 : epics.find(_varname)->second;
}













// *** processors 230 calcs photonenergy from bld ***

pp230::pp230(const name_t &name)
  : Processor(name)
{
  loadSettings(0);
}

void pp230::loadSettings(size_t)
{
  if (!setupCondition())
    return;
  setupGeneral();
  createHistList(tr1::shared_ptr<Histogram0DFloat>(new Histogram0DFloat()));
  Log::add(Log::INFO,"Processor '" + name() + "' calculates photonenergy from " +
           "'EbeamL3Energy' and 'EbeamPkCurrBC2'." + " Condition is '" +
           _condition->name() + "'");
}

void pp230::process(const CASSEvent& evt, HistogramBackend &res)
{
  using namespace MachineData;
  const Device &md
      (dynamic_cast<const Device&>
       (*(evt.devices().find(CASSEvent::MachineData)->second)));
  const Device::bldMap_t bld(md.BeamlineData());
  Histogram0DFloat &result(dynamic_cast<Histogram0DFloat&>(res));

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

  result = photonenergy;
}
