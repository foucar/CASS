// Copyright (C) 2010 Lutz Foucar

#include <QtCore/QSettings>
#include <QtCore/QString>

#include "machine_data.h"
#include "histogram.h"
#include "machine_device.h"


// *** postprocessors 120 retrives beamline data ***

cass::pp120::pp120(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key), _value(0)
{
  loadSettings(0);
}

cass::pp120::~pp120()
{
  _pp.histograms_delete(_key);
  _value = 0;
}

void cass::pp120::loadSettings(size_t)
{
  QSettings settings;
  settings.beginGroup("PostProcessor/active");
  settings.beginGroup(_key.c_str());
  _varname = settings.value("VariableName","").toString().toStdString();

  //creat the resulting histogram from the first histogram
  _pp.histograms_delete(_key);
  _value = new Histogram0DFloat();
  _pp.histograms_replace(_key,_value);

  std::cout << "PostProcessor "<<_key
      <<": will retrieve datafield \""<<_varname
      <<"\" from beamline data"
      <<std::endl;
}

void cass::pp120::operator()(const CASSEvent& evt)
{
  using namespace cass::MachineData;
  //retrieve beamline data from cassevent
  const MachineDataDevice *mdev
      (dynamic_cast<const MachineDataDevice *>
       (evt.devices().find(CASSEvent::MachineData)->second));
  const MachineDataDevice::bldMap_t bld(mdev->BeamlineData());

  _value->lock.lockForWrite();
  *_value = bld.find(_varname) == bld.end() ? 0: bld.find(_varname)->second;
  _value->lock.unlock();
}













// *** postprocessors 130 retrives epics data ***

cass::pp130::pp130(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key), _value(0)
{
  loadSettings(0);
}

cass::pp130::~pp130()
{
  _pp.histograms_delete(_key);
  _value = 0;
}

void cass::pp130::loadSettings(size_t)
{
  QSettings settings;
  settings.beginGroup("PostProcessor/active");
  settings.beginGroup(_key.c_str());
  _varname = settings.value("VariableName","").toString().toStdString();
  //create the resulting histogram from the first histogram
  _pp.histograms_delete(_key);
  _value = new Histogram0DFloat();
  _pp.histograms_replace(_key,_value);

  std::cout << "PostProcessor "<<_key
      <<": will retrieve datafield \""<<_varname
      <<"\" from epics data"
      <<std::endl;
}

void cass::pp130::operator()(const CASSEvent& evt)
{
  using namespace cass::MachineData;
  //retrieve beamline data from cassevent
  const MachineDataDevice *mdev
      (dynamic_cast<const MachineDataDevice *>
       (evt.devices().find(CASSEvent::MachineData)->second));
  const MachineDataDevice::epicsDataMap_t epics(mdev->EpicsData());

  _value->lock.lockForWrite();
  *_value = epics.find(_varname) == epics.end() ? 0 : epics.find(_varname)->second;
  _value->lock.unlock();
}
