// Copyright (C) 2009 Jochen Kuepper
// Copyright (C) 2009,2010 Lutz Foucar

/**
 * @file cass_machine.h global definition for machine data
 *
 * @author Lutz Foucar
 */

#ifndef CASS_MACHINE_GLOBAL_H
#define CASS_MACHINE_GLOBAL_H

#include <QtCore/qglobal.h>
#include <stdint.h>

#if defined(CASS_MACHINEDATA_LIBRARY)
#  define CASS_MACHINEDATASHARED_EXPORT Q_DECL_EXPORT
#else
#  define CASS_MACHINEDATASHARED_EXPORT Q_DECL_IMPORT
#endif

namespace cass
{
  namespace MachineData
  {
    /** we are assigned a given eventcode range which is 8 long
        this range does not start at 0 but at the offset which
        is defined here.*/
    const uint32_t EVREventCodeOffset=67;
  }
}
#endif // CASS_MACHINE_GLOBAL_H
