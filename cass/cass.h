#ifndef CASS_GLOBAL_H
#define CASS_GLOBAL_H

#include <QtCore/qglobal.h>
#include <vector>
#include <iterator>

#if defined(CASS_LIBRARY)
#  define CASSSHARED_EXPORT Q_DECL_EXPORT
#else
#  define CASSSHARED_EXPORT Q_DECL_IMPORT
#endif

namespace cass
{
  const size_t RingBufferSize=4;
  const size_t NbrOfWorkers=4;
  const size_t DatagramBufferSize=0x1000000;
  typedef float pixel_t;
}

#endif



// Local Variables:
// coding: utf-8
// mode: C++
// c-file-offsets: ((c . 0) (innamespace . 0))
// c-file-style: "Stroustrup"
// fill-column: 100
// End:
