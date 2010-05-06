// Copyright (C) 2009, 2010 Lutz Foucar

#ifndef PNCCDCONVERTER_H
#define PNCCDCONVERTER_H

#include "cass_pnccd.h"
#include "conversion_backend.h"
#include <vector>

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
//      Converter() {}
      /** operator to convert the LCLS Data to CASSEvent*/
      void operator()(const Pds::Xtc*, cass::CASSEvent*);
    private:
      /** store for the pnccd configuration. Will store the version and the
       * configuration itself in a pair
       */
      std::vector<std::pair<uint32_t, Pds::PNCCD::ConfigV2*> > _pnccdConfig;
    };
  }//end namespace vmi
}//end namespace cass

#endif
