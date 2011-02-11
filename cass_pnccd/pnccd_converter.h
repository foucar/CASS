// Copyright (C) 2009, 2010, 2011 Lutz Foucar

#ifndef PNCCDCONVERTER_H
#define PNCCDCONVERTER_H

#include <vector>

#include <QtCore/QMutex>

#include "cass_pnccd.h"
#include "conversion_backend.h"

namespace Pds
{
  namespace PNCCD
  {
    class ConfigV1;
    class ConfigV2;
  }
}
namespace cass
{
  class CASSEvent;

  namespace pnCCD
  {
    /** Converter for pnCCD Data.
     *
     * @author Lutz Foucar
     */
    class CASS_PNCCDSHARED_EXPORT Converter : public cass::ConversionBackend
    {
    public:
      /** create singleton if doesnt exist already */
      static ConversionBackend::shared_pointer instance();

      /** operator to convert the LCLS Data to CASSEvent*/
      void operator()(const Pds::Xtc*, cass::CASSEvent*);

    private:
      /** constructor
       *
       * set up the pds type ids that it is responsible for
       */
      Converter();

      /** prevent copy construction */
      Converter(const Converter&);

      /** prevent assignment */
      Converter& operator=(const Converter&);

      /** the singleton container */
      static ConversionBackend::shared_pointer _instance;

      /** singleton locker for mutithreaded requests */
      static QMutex _mutex;

      /** store for the pnccd configuration.
       *
       * Will store the version and the configuration itself in a pair
       */
      std::vector<std::pair<uint32_t, Pds::PNCCD::ConfigV2*> > _pnccdConfig;
    };
  }//end namespace vmi
}//end namespace cass

#endif
