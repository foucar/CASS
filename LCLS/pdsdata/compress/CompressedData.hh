#ifndef Pds_CompressedData_hh
#define Pds_CompressedData_hh

//
//  Each compressed data type will inherit first from this class and contain
//  a CompressedPayload object at its end.  In this manner, the decompression
//  can be handled without knowing the uncompressed data type.  This class
//  formalizes that layout.
//

#include <stdint.h>

namespace Pds {
  class CompressedPayload;
  class CompressedData {
  public:
    CompressedData() {}
    CompressedData(unsigned size);
    ~CompressedData() {}
  public:
    unsigned                 headerSize() const;
    const void*              header    () const;
    const CompressedPayload& pd        () const;
  private:
    uint32_t _headerSize;
  };
};

#endif
