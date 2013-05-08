// Copyright (C) 2011, 2013 Lutz Foucar

/**
 * @file mapcreator_base.cpp contains base class for all correction map creators.
 *
 * @author Lutz Foucar
 */

#include <sstream>
#include <stdexcept>

#include "mapcreator_base.h"

#include "mapcreators.h"
#include "mapcreators_online.h"
#include "gaincalibration.h"

using namespace cass;
using namespace pixeldetector;
using namespace std;
using namespace std::tr1;

MapCreatorBase::shared_pointer MapCreatorBase::instance(const string &type)
{
  shared_pointer ptr;
  if (type == "none")
    ptr = shared_pointer(new MapCreatorBase());
  else if (type == "fixed")
    ptr = shared_pointer(new FixedMaps());
  else if (type == "moving")
    ptr = shared_pointer(new MovingMaps());
  else if (type == "online")
    ptr = shared_pointer(new OnlineFixedCreator());
  else if (type == "onlinecommonmode")
    ptr = shared_pointer(new OnlineFixedCreatorCommonMode());
  else if (type == "GainFixedADURange")
    ptr = shared_pointer(new GainCalibration());
  else if (type == "hotpix")
    ptr = shared_pointer(new HotPixelsFinder());
  else
    throw invalid_argument("MapCreatorBase::instance: Map Creator type '" + type +
                           "' is unknown.");
  return ptr;
}

MapCreatorBase::~MapCreatorBase()
{

}

void MapCreatorBase::operator ()(const Frame&)
{

}

void MapCreatorBase::loadSettings(CASSSettings &)
{

}

void MapCreatorBase::controlCalibration(const std::string &)
{

}
