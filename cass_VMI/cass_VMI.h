#ifndef CASS_VMI_GLOBAL_H
#define CASS_VMI_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(CASS_VMI_LIBRARY)
#  define CASS_VMISHARED_EXPORT Q_DECL_EXPORT
#else
#  define CASS_VMISHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // CASS_VMI_GLOBAL_H
