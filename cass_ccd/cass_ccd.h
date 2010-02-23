#ifndef CASS_CCD_GLOBAL_H
#define CASS_CCD_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(CASS_CCD_LIBRARY)
#  define CASS_CCDSHARED_EXPORT Q_DECL_EXPORT
#else
#  define CASS_CCDSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // CASS_CCD_GLOBAL_H
