#ifndef CASS_REMI_GLOBAL_H
#define CASS_REMI_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(CASS_REMI_LIBRARY)
#  define CASS_REMISHARED_EXPORT Q_DECL_EXPORT
#else
#  define CASS_REMISHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // CASS_REMI_GLOBAL_H
