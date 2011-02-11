// Copyright (C) 2011 Lutz Foucar

/**
 * @file conversion_backend.cpp file contains base class for all format converters
 *
 * @author Lutz Foucar
 */

#include <sstream>
#include <stdexcept>

#include <QtCore/QMutex>

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
  namespace Blank
  {
    /** Converter that does nothing
     *
     * This converter is a blank that does nothing. It will be used for converting
     * xtc id's that we either don't care about or the user has disabled in .ini
     *
     * @author Lutz Foucar
     */
    class CASSSHARED_EXPORT Converter : public ConversionBackend
    {
    public:
      /** create singleton if doesnt exist already */
      static ConversionBackend::shared_pointer instance();

      /** do nothing */
      void operator()(const Pds::Xtc*, cass::CASSEvent*) {}

    private:
      /** constructor
       *
       * sets up the pds type ids that it is responsible for
       */
      Converter() {}

      /** prevent copy construction */
      Converter(const Converter&);

      /** prevent assignment */
      Converter& operator=(const Converter&);

      /** the singleton container */
      static ConversionBackend::shared_pointer _instance;

      /** singleton locker for mutithreaded requests */
      static QMutex _mutex;
    };
  }
}

// =================define static members =================
ConversionBackend::shared_pointer Blank::Converter::_instance;
QMutex Blank::Converter::_mutex;

ConversionBackend::shared_pointer Blank::Converter::instance()
{
  QMutexLocker locker(&_mutex);
  if(!_instance)
  {
    _instance = ConversionBackend::shared_pointer(new Converter);
  }
  return _instance;
}
// ========================================================


ConversionBackend::shared_pointer ConversionBackend::instance(const std::string &type)
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
  else if("Machine" == type)
    converter = MachineData::Converter::instance();
  else if("Blank" == type)
    converter = Blank::Converter::instance();
  else
  {
    stringstream ss;
    ss<<"ConversionBackend::instance(): Requested converter type '"<<type
        <<"' is unkown";
    throw invalid_argument(ss.str());
  }
  return converter;
}
