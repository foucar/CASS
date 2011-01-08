//Copyright (C) 2009-2010 Lutz Foucar

/**
 * @file spectrometer.cpp file contains the classes that describe a REMI type
 *                        spectrometer.
 *
 * @author Lutz Foucar
 */

#include <cmath>

#include "spectrometer.h"

#include "particle.h"
#include "cass_settings.h"

using namespace cass::ACQIRIS;

void Spectrometer::loadSettings(CASSSettings &s, const Particle& p)
{
  s.beginGroup("Spectrometer");
  int size = s.beginReadArray("Regions");
  for (int i = 0; i < size; ++i)
  {
    s.setArrayIndex(i);
    SpectrometerRegion region;
    region.loadSettings(s);
    _regions.push_back(region);
  }
  s.endArray();
  _BFieldIsOn = s.value("BFieldIsOn",false).toBool();
  _cyclotronPeriod = s.value("CyclotronPeriode",10).toDouble();
  _cyclotronPeriod *= (p.mass_au() / std::abs(p.charge_au()));
  _rotationClockwise = s.value("RotationClockwise",true).toBool();
  s.endGroup();
}

void SpectrometerRegion::loadSettings(CASSSettings &s)
{
  _length = s.value("Length",10).toDouble();
  _efield = s.value("EField",10).toDouble();
}
