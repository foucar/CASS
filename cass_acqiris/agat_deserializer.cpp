// Copyright (C) 2011 Lutz Foucar

/**
 * @file agat_deserializer.cpp contains functions to deserialize the data stream
 *                             sent by agat.
 *
 * @author Lutz Foucar
 */

#include <QtCore/QDataStream>

#include "agat_deserializer.h"

#include "cass_event.h"

using namespace cass;
using namespace ACQIRIS;
using namespace std;


bool deserializeNormalAgat::operator ()(QDataStream& stream, CASSEvent& evt)
{
  return true;
}

