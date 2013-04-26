// Copyright (C) 2011 Lutz Foucar

/**
 * @file conversion_backend.cpp file contains base class for all format converters
 *
 * @author Lutz Foucar
 */

#include <stdexcept>

#include <QtCore/QMutex>

#include "conversion_backend.h"

#include "pdsdata/xtc/Xtc.hh"

#include "acqiris_converter.h"
#include "acqiristdc_converter.h"
#include "ccd_converter.h"
#include "machine_converter.h"
#include "pnccd_converter.h"
#include "lcls_converter.h"
#include "log.h"

using namespace std;
using namespace std::tr1;
using namespace cass;


ConversionBackend::shared_pointer ConversionBackend::instance(const string &type)
{
  shared_ptr<ConversionBackend> converter;
  if("Acqiris" == type)
    converter = ACQIRIS::Converter::instance();
  else if("AcqirisTDC" == type)
    converter = ACQIRISTDC::Converter::instance();
  else if("CCD" == type)
    converter = CCD::Converter::instance();
  else if("pnCCD" == type)
    converter = pnCCD::Converter::instance();
  else if("pixeldetector" == type)
    converter = pixeldetector::Converter::instance();
  else if("Machine" == type)
    converter = MachineData::Converter::instance();
  else if("Blank" == type)
    converter = shared_pointer(new ConversionBackend);
  else
    throw invalid_argument("ConversionBackend::instance(): Requested converter type '" +
                           type + "' is unkown");
  return converter;
}

void ConversionBackend::operator()(const Pds::Xtc *xtc, cass::CASSEvent*)
{
  Log::add(Log::DEBUG0,string("ConversionBackend::operator(): Converter for xtc type '") +
           Pds::TypeId::name(xtc->contains.id()) + "' has not been assigned or " +
           "implemented");
}
