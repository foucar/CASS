// Copyright (C) 2011 Lutz Foucar

/**
 * @file conversion_backend.cpp file contains base class for all format converters
 *
 * @author Lutz Foucar
 */

#include <sstream>
#include <stdexcept>

#include "conversion_backend.h"

#include "acqiris_converter.h"
#include "acqiristdc_converter.h"
#include "ccd_converter.h"
#include "machine_converter.h"
#include "pnccd_converter.h"

using namespace std;
using namespace std::tr1;
using namespace cass;

namespace cass
{
  /** Converter that does nothing
   *
   * This converter is a blank that does nothing. It will be used for converting
   * xtc id's that we either don't care about or the user has disabled in .ini
   *
   * @author Lutz Foucar
   */
  class CASSSHARED_EXPORT BlankConverter : public ConversionBackend
  {
  public:
    /** do nothing */
    void operator()(const Pds::Xtc*, cass::CASSEvent*) {}
  };

  namespace formatconverters
  {
    /** the names of converters */
    static const char* names[] =
    {
      "Machine",
      "Acqiris",
      "AcqirisTDC",
      "CCD",
      "pnCCD",
      "Blank"
    };
  }
}
list<string>  ConversionBackend::availableConverters(formatconverters::names,&formatconverters::names[4]);

ConversionBackend::converterPtr_t ConversionBackend::instance(const std::string &type)
{
  shared_ptr<ConversionBackend> converter;
  if("Acqiris" == type)
    converter = shared_ptr<ConversionBackend>(new ACQIRIS::Converter());
  else if("AcqirisTDC" == type)
    converter = shared_ptr<ConversionBackend>(new ACQIRISTDC::Converter());
  else if("CCD" == type)
    converter = shared_ptr<ConversionBackend>(new CCD::Converter());
  else if("pnCCD" == type)
    converter = shared_ptr<ConversionBackend>(new pnCCD::Converter());
  else if("Machine" == type)
    converter = shared_ptr<ConversionBackend>(new MachineData::Converter());
  else if("Blank" == type)
    converter = shared_ptr<ConversionBackend>(new BlankConverter());
  else
  {
    stringstream ss;
    ss<<"ConversionBackend::instance(): Requested converter type '"<<type
        <<"' is unkown";
    throw invalid_argument(ss.str());
  }
  return converter;
}
