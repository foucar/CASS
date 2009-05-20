#ifndef CASS_GLOBAL_H
#define CASS_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(CASS_LIBRARY)
#  define CASSSHARED_EXPORT Q_DECL_EXPORT
#else
#  define CASSSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // CASS_GLOBAL_H
