#ifndef CASS_CCD_GLOBAL_H
#define CASS_CCD_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(CASS_CCD_LIBRARY)
#  define CASS_CCDSHARED_EXPORT Q_DECL_EXPORT
#else
#  define CASS_CCDSHARED_EXPORT Q_DECL_IMPORT
#endif

namespace cass
{
  namespace CCD
  {
    const size_t default_size(1000);
    const size_t default_size_sq(1000*1000);
  }
}


#endif // CASS_CCD_GLOBAL_H
