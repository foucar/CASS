#ifndef Cspad_ConfigV3QuadReg_hh
#define Cspad_ConfigV3QuadReg_hh

#include "Detector.hh"
#include "ConfigV1QuadReg.hh" // for the pots, gainmaps and readonly regs

#include <stdint.h>

#pragma pack(4)

namespace Pds {
  namespace CsPad {

    class ConfigV3QuadReg
    {

      public:
        ConfigV3QuadReg() {};
        ConfigV3QuadReg(
            uint32_t         shiftSelect[],
            uint32_t         edgeSelect[],
            uint32_t         readClkSet,
            uint32_t         readClkHold,
            uint32_t         dataMode,
            uint32_t         prstSel,
            uint32_t         acqDelay,
            uint32_t         intTime,
            uint32_t         digDelay,
            uint32_t         ampIdle,
            uint32_t         injTotal,
            uint32_t         rowColShiftPer,
            uint32_t         ampReset,
            uint32_t         digCount,
            uint32_t         digPeriod,
            uint32_t         biasTuning,
            uint32_t         pdpmndnmBalance)  :
            _readClkSet(readClkSet),
            _readClkHold(readClkHold),
            _dataMode(dataMode),
            _prstSel(prstSel),
            _acqDelay(acqDelay),
            _intTime(intTime),
            _digDelay(digDelay),
            _ampIdle(ampIdle),
            _injTotal(injTotal),
            _rowColShiftPer(rowColShiftPer),
            _ampReset(ampReset),
            _digCount(digCount),
            _digPeriod(digPeriod),
            _biasTuning(biasTuning),
            _pdpmndnmBalance(pdpmndnmBalance) {
          _shiftSelect[0] = shiftSelect[0];
          _shiftSelect[1] = shiftSelect[1];
          _shiftSelect[2] = shiftSelect[2];
          _shiftSelect[3] = shiftSelect[3];
          _edgeSelect [0] = edgeSelect [0];
          _edgeSelect [1] = edgeSelect [1];
          _edgeSelect [2] = edgeSelect [2];
          _edgeSelect [3] = edgeSelect [3];
        };

        const uint32_t*    shiftSelect()        const   { return _shiftSelect;    }
        const uint32_t*    edgeSelect()         const   { return _edgeSelect;     }
        uint32_t           readClkSet()         const   { return _readClkSet;     }
        uint32_t           readClkHold()        const   { return _readClkHold;    }
        uint32_t           dataMode()           const   { return _dataMode;       }
        uint32_t           prstSel()            const   { return _prstSel;        }
        uint32_t           acqDelay()           const   { return _acqDelay;       }
        uint32_t           intTime()            const   { return _intTime;        }
        uint32_t           digDelay()           const   { return _digDelay;       }
        uint32_t           ampIdle()            const   { return _ampIdle;        }
        uint32_t           injTotal()           const   { return _injTotal;       }
        uint32_t           rowColShiftPer()     const   { return _rowColShiftPer; }
        uint32_t           ampReset()           const   { return _ampReset;       }
        uint32_t           digCount()           const   { return _digCount;       }
        uint32_t           digPeriod()          const   { return _digPeriod;      }
        uint32_t           biasTuning()         const   { return _biasTuning;     }
        void               biasTuning(uint32_t b)       { _biasTuning = b;        }
        uint32_t           pdpmndnmBalance()    const   { return _pdpmndnmBalance; }
        Pds::CsPad::CsPadReadOnlyCfg&           ro      ()        { return _readOnly;       }
        const Pds::CsPad::CsPadReadOnlyCfg&     ro      ()  const { return _readOnly;       }
        Pds::CsPad::CsPadDigitalPotsCfg&        dp      ()        { return _digitalPots;    }
        const Pds::CsPad::CsPadDigitalPotsCfg&  dp      ()  const { return _digitalPots;    }
        Pds::CsPad::CsPadGainMapCfg*            gm      ()        { return &_gainMap;       }
        const Pds::CsPad::CsPadGainMapCfg*      gm      ()  const { return &_gainMap;       }
        Pds::CsPad::CsPadReadOnlyCfg*           readOnly()        { return &_readOnly;      }
        const Pds::CsPad::CsPadReadOnlyCfg*     readOnly()  const { return &_readOnly;      }

      private:
        uint32_t                         _shiftSelect[TwoByTwosPerQuad];
        uint32_t                         _edgeSelect[TwoByTwosPerQuad];
        uint32_t                         _readClkSet;
        uint32_t                         _readClkHold;
        uint32_t                         _dataMode;
        uint32_t                         _prstSel;
        uint32_t                         _acqDelay;
        uint32_t                         _intTime;
        uint32_t                         _digDelay;
        uint32_t                         _ampIdle;
        uint32_t                         _injTotal;
        uint32_t                         _rowColShiftPer;
        uint32_t                         _ampReset;
        uint32_t                         _digCount;
        uint32_t                         _digPeriod;
        // put unwritten registers after this
        uint32_t                         _biasTuning;  // bias tuning is used, but not written
                                                       // 2 bits per nibble, C2,C1,I5,I2
                                                       // bit order rc00rc00rc00rc
        uint32_t                         _pdpmndnmBalance;  // pMOS and nMOS Displacement and Main
                                                      // used but not written and not in GUI yet
                                                      // hard-wired to zero in GUI
                                                      // 2 bits per nibble, bit order pd00pm00nd00nm

        Pds::CsPad::CsPadReadOnlyCfg     _readOnly;
        Pds::CsPad::CsPadDigitalPotsCfg  _digitalPots;
        Pds::CsPad::CsPadGainMapCfg      _gainMap;
    };
  };
};
#pragma pack()

#endif // Cspad_ConfigV3QuadReg_hh
