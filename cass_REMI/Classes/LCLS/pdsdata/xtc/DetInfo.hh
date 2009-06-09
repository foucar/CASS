#ifndef Pds_DetInfo_hh
#define Pds_DetInfo_hh

#include <stdint.h>
#include "pdsdata/xtc/Src.hh"

namespace Pds {

  class Node;

  class DetInfo : public Src {
  public:

    enum Detector {NoDetector,AmoIms,AmoPem,AmoETof,AmoITof,AmoMbs,AmoIis,
                   AmoXes,NumDetector};
    enum Device   {NoDevice,Evr,Acqiris,Opal1000,NumDevice};

    DetInfo(uint32_t processId,
            Detector det, uint32_t detId,
            Device dev,   uint32_t devId);

    uint32_t processId() const;
    Detector detector()  const;
    Device   device()    const;
    uint32_t detId()     const;
    uint32_t devId()     const;

    static const char* name(Detector);
    static const char* name(Device);
  };

}
#endif
