#ifndef PRINCETON_CONFIG_V4_HH
#define PRINCETON_CONFIG_V4_HH

#include <stdint.h>
#include "ConfigV1.hh"
#include "ConfigV2.hh"
#include "ConfigV3.hh"


#pragma pack(4)

namespace Pds
{

namespace Princeton
{

class ConfigV4
{
public:
  enum { Version = 4 };

  ConfigV4()  {}
  ConfigV4(
   uint32_t         u32Width,
   uint32_t         u32Height,
   uint32_t         u32OrgX,
   uint32_t         u32OrgY,
   uint32_t         u32BinX,
   uint32_t         u32BinY,
   uint32_t         u32MaskedHeight,
   uint32_t         u32KineticHeight,
   float            f32VsSpeed,
   float            f32ExposureTime,
   float            f32CoolingTemp,
   uint8_t          u8GainIndex,
   uint8_t          u8ReadoutSpeedIndex,
   uint16_t         u16ExposureEventCode,
   uint32_t         u32NumDelayShots          = -1  // default value is used by the config program
   );

  uint32_t          width ()            const         { return _u32Width; }
  uint32_t          height()            const         { return _u32Height; }
  uint32_t          orgX  ()            const         { return _uOrgX; }
  uint32_t          orgY  ()            const         { return _uOrgY; }
  uint32_t          binX  ()            const         { return _uBinX; }
  uint32_t          binY  ()            const         { return _uBinY; }
  uint32_t          maskedHeight  ()    const         { return _u32MaskedHeight; }
  uint32_t          kineticHeight ()    const         { return _u32KineticHeight; }
  float             vsSpeed       ()    const         { return _f32VsSpeed; }
  float             exposureTime  ()    const         { return _f32ExposureTime; }
  float             coolingTemp   ()    const         { return _f32CoolingTemp; }
  uint8_t           gainIndex     ()    const         { return _u8GainIndex; }
  uint8_t           readoutSpeedIndex() const         { return _u8ReadoutSpeedIndex; }

  uint16_t          exposureEventCode() const         { return _u16ExposureEventCode; }
  uint32_t          numDelayShots()     const         { return _u32NumDelayShots; }

  uint32_t          setWidth    (uint32_t uWidth)     { return _u32Width = uWidth; }
  uint32_t          setHeight   (uint32_t uHeight)    { return _u32Height = uHeight; }
  uint16_t          setReadoutSpeedIndex
                            (uint16_t uSpeedIndex)    { return _u8ReadoutSpeedIndex = uSpeedIndex; }
  uint32_t          setNumDelayShots
                            (uint32_t uNumDelayShots) { return _u32NumDelayShots = uNumDelayShots;  }

  int               size      ()        const         { return sizeof(*this); }
  int               frameSize ()        const; // calculate the frame size based on the current ROI and binning settings

  ConfigV4(const ConfigV1& configV1);
  ConfigV4(const ConfigV2& configV2);
  ConfigV4(const ConfigV3& configV3);

private:
  uint32_t          _u32Width, _u32Height;
  uint32_t          _uOrgX,  _uOrgY;
  uint32_t          _uBinX,  _uBinY;
  uint32_t          _u32MaskedHeight;
  uint32_t          _u32KineticHeight;
  float             _f32VsSpeed;
  float             _f32ExposureTime;
  float             _f32CoolingTemp;
  uint8_t           _u8GainIndex;
  uint8_t           _u8ReadoutSpeedIndex;
  uint16_t          _u16ExposureEventCode;
  uint32_t          _u32NumDelayShots;

  friend class ConfigV5;
};

} // namespace Princeton

} // namespace Pds

#pragma pack()

#endif //#ifndef PRINCETON_CONFIG_V4_HH
