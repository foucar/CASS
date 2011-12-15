// Copyright (C) 2011 Lutz Foucar

/**
 * @file shm_deserializer.cpp contains functors to deserialize the data stream
 *                            sent by shm2tcp.
 *
 * @author Lutz Foucar
 */

#include <QtCore/QDataStream>

#include "shm_deserializer.h"

#include "cass_event.h"

using namespace cass;
using namespace pnCCD;
using namespace std;

bool deserializeSHM::operator ()(QDataStream& stream, CASSEvent& evt)
{

}
