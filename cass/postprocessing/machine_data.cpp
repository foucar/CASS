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
      <<"\" from Beamlinedata"
      <<std::endl;
}

void cass::pp850::operator()(const CASSEvent& evt)
{
  using namespace cass::MachineData;
  //retrieve beamline data from cassevent
  MachineDataDevice *mdev
      (dynamic_cast<MachineDataDevice *>
      (evt.devices().find(CASSEvent::MachineData)->second));
  MachineDataDevice::bldMap_t bld(mdev->BeamlineData());

  _data->lock.lockForWrite();
  *_data = bld.find(_varname) == bld.end() ? 0: bld.find(_varname)->second;
  _data->lock.unlock();
}
