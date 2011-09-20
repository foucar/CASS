// Copyright (C) 2011 Lutz Foucar

/**
 * @file mapcreators.h contains all correction map creators.
 *
 * @author Lutz Foucar
 */

#include "mapcreators.h"

#include "cass_settings.h"

using namespace cass;
using namespace pixeldetector;
using namespace std;

void NonAlteringMaps::operator ()(const Frame &frame)
{

}

void NonAlteringMaps::loadSettings(CASSSettings &s)
{

}

void FixedMaps::operator ()(const Frame &frame)
{

}

void FixedMaps::loadSettings(CASSSettings &s)
{

}

void MovingMaps::operator ()(const Frame &frame)
{

}

void MovingMaps::loadSettings(CASSSettings &s)
{

}
