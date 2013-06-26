#ifndef Pds_BldInfo_hh
#define Pds_BldInfo_hh

#include <stdint.h>
#include "pdsdata/xtc/Src.hh"

namespace Pds {

  class Node;

  class BldInfo : public Src {
  public:

    enum Type { EBeam            = 0,   // Global
                PhaseCavity      = 1,
                FEEGasDetEnergy  = 2,
                Nh2Sb1Ipm01      = 3,   // XPP + downstream
                HxxUm6Imb01      = 4,   // XRT
                HxxUm6Imb02      = 5,
                HfxDg2Imb01      = 6,
                HfxDg2Imb02      = 7,
                XcsDg3Imb03      = 8,
                XcsDg3Imb04      = 9,
                HfxDg3Imb01      = 10,
                HfxDg3Imb02      = 11,
                HxxDg1Cam        = 12,
                HfxDg2Cam        = 13,
                HfxDg3Cam        = 14,
                XcsDg3Cam        = 15,
                HfxMonCam        = 16,
                HfxMonImb01      = 17,
                HfxMonImb02      = 18,
                HfxMonImb03      = 19,
                MecLasEm01       = 20,  // MEC Local
                MecTctrPip01     = 21,
                MecTcTrDio01     = 22,
                MecXt2Ipm02      = 23,
                MecXt2Ipm03      = 24,
                MecHxmIpm01      = 25,
                GMD              = 26,  // SXR Local
                CxiDg1Imb01      = 27,  // CXI Local
                CxiDg2Imb01      = 28,
                CxiDg2Imb02      = 29,
                CxiDg4Imb01      = 30,
                CxiDg1Pim        = 31,
                CxiDg2Pim        = 32,
                CxiDg4Pim        = 33,
                XppMonPim0       = 34,
                XppMonPim1       = 35,
                XppSb2Ipm        = 36,
                XppSb3Ipm        = 37,
                XppSb3Pim        = 38,
                XppSb4Pim        = 39,
                XppEndstation0   = 40,
                XppEndstation1   = 41,
                MecXt2Pim02      = 42,
                MecXt2Pim03      = 43,
                CxiDg3Spec       = 44,
                NumberOf };

    BldInfo() {}
    BldInfo(uint32_t processId,
            Type     type);
    BldInfo(const char*);

    bool operator==(const BldInfo&) const;

    uint32_t processId() const;
    Type     type()  const;

    static const char* name(const BldInfo&);
  };

}
#endif
