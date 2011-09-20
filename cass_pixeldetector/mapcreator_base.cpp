// Copyright (C) 2011 Lutz Foucar

/**
 * @file mapcreator_base.cpp contains base class for all correction map creators.
 *
 * @author Lutz Foucar
 */

#include <sstream>
#include <stdexcept>

#include "mapcreator_base.h"

#include "mapcreators.h"

using namespace cass;
using namespace pixeldetector;
using namespace std;
using namespace std::tr1;

MapCreatorBase::shared_pointer MapCreatorBase::instance(const string &type)
{
  shared_pointer ptr;
  if (type == "none")
    ptr = shared_pointer(new NonAlteringMaps());
  else if (type == "fixed")
    ptr = shared_pointer(new FixedMaps());
  else if (type == "moving")
    ptr = shared_pointer(new MovingMaps());
  else
  {
    stringstream ss;
    ss << "MapCreatorBase::instance: Map Creator type '"<< type
        <<"' is unknown.";
    throw invalid_argument(ss.str());
  }
  return ptr;
}
