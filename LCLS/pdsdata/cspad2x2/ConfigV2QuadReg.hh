#ifndef Cspad2x2_ConfigV2QuadReg_hh
#define Cspad2x2_ConfigV2QuadReg_hh

#include "pdsdata/cspad2x2/Detector.hh"
#include "pdsdata/cspad2x2/ConfigV1QuadReg.hh"  // for pots, readonlys and gainmap

#include <stdint.h>

#pragma pack(4)

namespace Pds {
  namespace CsPad2x2 {


    class ConfigV2QuadReg
    {

      public:
        ConfigV2QuadReg() {};
        ConfigV2QuadReg(
            uint32_t         shiftSelect,
            uint32_t         edgeSelect,
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
            uint32_t         PeltierEnable,
            uint32_t         kpConstant,
            uint32_t         kiConstant,
            uint32_t         kdConstant,
            uint32_t         humidThold,
            uint32_t         setPoint,
            uint32_t         biasTuning,
            uint32_t         pdpmndnmBalance) :
            _shiftSelect(shiftSelect),
            _edgeSelect(edgeSelect),
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
            _PeltierEnable(PeltierEnable),
            _kpConstant(kpConstant),
            _kiConstant(kiConstant),
            _kdConstant(kdConstant),
            _humidThold(humidThold),
            _setPoint(setPoint),
            _biasTuning(biasTuning),
            _pdpmndnmBalance(pdpmndnmBalance)
        {};

        uint32_t                                   shiftSelect()        const { return _shiftSelect;    }
        uint32_t                                   edgeSelect()         const { return _edgeSelect;     }
        uint32_t                                   readClkSet()         const { return _readClkSet;     }
        uint32_t                                   readClkHold()        const { return _readClkHold;    }
        uint32_t                                   dataMode()           const { return _dataMode;       }
        uint32_t                                   prstSel()            const { return _prstSel;        }
        uint32_t                                   acqDelay()           const { return _acqDelay;       }
        uint32_t                                   intTime()            const { return _intTime;        }
        uint32_t                                   digDelay()           const { return _digDelay;       }
        uint32_t                                   ampIdle()            const { return _ampIdle;        }
        uint32_t                                   injTotal()           const { return _injTotal;       }
        uint32_t                                   rowColShiftPer()     const { return _rowColShiftPer; }
        uint32_t                                   ampReset()           const { return _ampReset;       }
        uint32_t                                   digCount()           const { return _digCount;       }
        uint32_t                                   digPeriod()          const { return _digPeriod;      }
        uint32_t                                   biasTuning()         const { return _biasTuning;     }
        void                                       biasTuning(uint32_t b)     { _biasTuning = b;        }
        uint32_t                                   pdpmndnmBalance()    const { return _pdpmndnmBalance; }
        uint32_t                                   PeltierEnable()      const { return _PeltierEnable;  }
        uint32_t                                   kpConstant()         const { return _kpConstant;     }
        uint32_t                                   kiConstant()         const { return _kiConstant;     }
        uint32_t                                   kdConstant()         const { return _kdConstant;     }
        uint32_t                                   humidThold()         const { return _humidThold;     }
        uint32_t                                   setPoint()           const { return _setPoint;       }
        Pds::CsPad2x2::CsPad2x2ReadOnlyCfg&                 ro()              { return _readOnly;       }
        const Pds::CsPad2x2::CsPad2x2ReadOnlyCfg&           ro()        const { return _readOnly;       }
        Pds::CsPad2x2::CsPad2x2DigitalPotsCfg&              dp()              { return _digitalPots;    }
        const Pds::CsPad2x2::CsPad2x2DigitalPotsCfg&        dp()        const { return _digitalPots;    }
        Pds::CsPad2x2::CsPad2x2GainMapCfg*                  gm()              { return &_gainMap;       }
        const Pds::CsPad2x2::CsPad2x2GainMapCfg*            gm()        const { return &_gainMap;       }
        Pds::CsPad2x2::CsPad2x2ReadOnlyCfg*                 readOnly()        { return &_readOnly;      }
        const Pds::CsPad2x2::CsPad2x2ReadOnlyCfg*           readOnly()  const { return &_readOnly;      }

      private:
        uint32_t                            _shiftSelect;
        uint32_t                            _edgeSelect;
        uint32_t                            _readClkSet;
        uint32_t                            _readClkHold;
        uint32_t                            _dataMode;
        uint32_t                            _prstSel;
        uint32_t                            _acqDelay;
        uint32_t                            _intTime;
        uint32_t                            _digDelay;
        uint32_t                            _ampIdle;
        uint32_t                            _injTotal;
        uint32_t                            _rowColShiftPer;
        uint32_t                            _ampReset;
        uint32_t                            _digCount;
        uint32_t                            _digPeriod;
        uint32_t                            _PeltierEnable;
        uint32_t                            _kpConstant;
        uint32_t                            _kiConstant;
        uint32_t                            _kdConstant;
        uint32_t                            _humidThold;
        uint32_t                            _setPoint;
        // put unwritten registers after this
        uint32_t                            _biasTuning;  // bias tuning is used, but not written
                                                          // 2 bits per nibble, C2,C1,I5,I2
                                                          // bit order rc00rc00rc00rc
        uint32_t                            _pdpmndnmBalance;  // pMOS and nMOS Displacement and Main
                                                          // used but not written and not in GUI yet
                                                          // hard-wired to zero in GUI
                                                          // 2 bits per nibble, bit order pd00pm00nd00nm

        Pds::CsPad2x2::CsPad2x2ReadOnlyCfg    _readOnly;
        Pds::CsPad2x2::CsPad2x2DigitalPotsCfg _digitalPots;
        Pds::CsPad2x2::CsPad2x2GainMapCfg     _gainMap;
    };
  };
};

#pragma pack()

#endif
