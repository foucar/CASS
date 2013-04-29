#ifndef Cspad2x2_ConfigV2_hh
#define Cspad2x2_ConfigV2_hh

//
//  CSPAD2x2 readout configuration and region of interest reduction
//

#include "pdsdata/cspad2x2/Detector.hh"
#include "pdsdata/cspad2x2/ConfigV2QuadReg.hh"

#include <stdint.h>
#include "pdsdata/cspad2x2/ProtectionSystem.hh"

#pragma pack(4)

namespace Pds
{
  namespace CsPad2x2
  {

    class ConfigV2
    {
      public:
        static const int Version               = 2;
        ConfigV2() {};
        ConfigV2( uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);

        ConfigV2QuadReg*                 quad();
        const ConfigV2QuadReg*           quad() const;
        uint32_t                         tdi() const { return _testDataIndex; }
        ProtectionSystemThreshold*       protectionThreshold ();
        const ProtectionSystemThreshold* protectionThreshold () const;
        const uint32_t                   protectionEnable () const { return _protectionEnable; }
        uint32_t                         protectionEnable ()       { return _protectionEnable; }
        void                             protectionEnable(uint32_t e) {_protectionEnable=e; }
        uint32_t                         inactiveRunMode()const{ return _inactiveRunMode; }
        uint32_t                         activeRunMode() const { return _activeRunMode; }
        uint32_t                         payloadSize  () const { return _payloadPerQuad; }
        uint32_t                         badAsicMask () const { return _badAsicMask; }
        //  "asicMask" is actually a 4-bit mask of 2x2 sectors applied to all quadrants
        uint32_t                         asicMask     () const { return _AsicMask; }
        //  "roiMask" is a mask of 2x1 slices
        unsigned                         roiMask      (int iq) const;
        unsigned                         roiMask      () const;
        uint32_t                         runTriggerDelay() {return _runTriggerDelay; }
        const uint32_t                   runTriggerDelay() const {return _runTriggerDelay; }
        void                             runTriggerDelay( uint32_t d ) { _runTriggerDelay = d; }
        unsigned                         numAsicsRead () const;
        unsigned                         numAsicsStored(int iq) const;
        unsigned                         numAsicsStored() const;
        uint32_t                         concentratorVersion() const {return _concentratorVersion; }
        uint32_t*                        concentratorVersionAddr();
        static const int                 version      ()       { return Version; }
      private:
        uint32_t                  _concentratorVersion;
        ProtectionSystemThreshold _protectionThreshold;
        uint32_t                  _protectionEnable;
        uint32_t                  _inactiveRunMode;
        uint32_t                  _activeRunMode;
        uint32_t                  _runTriggerDelay;
        uint32_t                  _testDataIndex;
        uint32_t                  _payloadPerQuad;
        uint32_t                  _badAsicMask;
        uint32_t                  _AsicMask;  // there are a maximum of 4 ASICs
        uint32_t                  _roiMask;
        ConfigV2QuadReg           _quad;
    };


  } // namespace CsPad2x2

} // namespace Pds 

#pragma pack()

#endif  // CSPAD_CONFIG_V2_HH
