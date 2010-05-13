#ifndef CASS_PNCCD_GLOBAL_H
#define CASS_PNCCD_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(CASS_PNCCD_LIBRARY)
#  define CASS_PNCCDSHARED_EXPORT Q_DECL_EXPORT
#else
#  define CASS_PNCCDSHARED_EXPORT Q_DECL_IMPORT
#endif

namespace cass
{
  namespace pnCCD
  {
    const size_t default_size(1024);
    const size_t default_size_sq(1024*1024);
  }
}
#endif // CASS_PNCCD_GLOBAL_H
