#ifndef Pds_CompressedXtc_hh
#define Pds_CompressedXtc_hh

#include "pdsdata/compress/CompressedPayload.hh"
#include "pdsdata/xtc/Xtc.hh"

#include <list>

namespace boost { template<class T> class shared_ptr; };

namespace Pds {
  class CompressedXtc : public Xtc {
  public:
    static boost::shared_ptr<Xtc> uncompress(const Xtc&);
  public:
    CompressedXtc( Xtc&     xtc,
                   const std::list<unsigned>& headerOffsets,
                   unsigned headerSize,
                   unsigned depth,
                   CompressedPayload::Engine engine );
    CompressedXtc( Xtc&     xtc,
                   const std::list<unsigned>& headerOffsets,
                   const std::list<unsigned>& headerSizes,
                   unsigned depth,
                   CompressedPayload::Engine engine );
  public:
    void* operator new(size_t, void* p) { return p; }
  private:
    ~CompressedXtc() {}
  };
};

#endif
