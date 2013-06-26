#ifndef PRINCETON_CONFIG_V5_HH
#define PRINCETON_CONFIG_V5_HH

#include <stdint.h>
#include "ConfigV1.hh"
#include "ConfigV2.hh"
#include "ConfigV3.hh"
#include "ConfigV4.hh"

#pragma pack(4)

namespace Pds
{

namespace Princeton
{

class ConfigV5
{
public:
  enum { Version = 5 };

  ConfigV5()  {}
  ConfigV5(
   uint32_t         u32Width,
   uint32_t         u32Height,
   uint32_t         u32OrgX,
   uint32_t         u32OrgY,
   uint32_t         u32BinX,
   uint32_t         u32BinY,
   float            f32ExposureTime,
   float            f32CoolingTemp,
   uint16_t         u16GainIndex,
   uint16_t         u16ReadoutSpeedIndex,
   uint32_t         u32MaskedHeight,
   uint32_t         u32KineticHeight,
   float            f32VsSpeed,
   int16_t          i16InfoReportInterval,
   uint16_t         u16ExposureEventCode,
   uint32_t         u32NumDelayShots      // default value is used by the config program
   );

  uint32_t          width ()            const         { return _u32Width; }
  uint32_t          height()            const         { return _u32Height; }
  uint32_t          orgX  ()            const         { return _uOrgX; }
  uint32_t          orgY  ()            const         { return _uOrgY; }
  uint32_t          binX  ()            const         { return _uBinX; }
  uint32_t          binY  ()            const         { return _uBinY; }
  float             exposureTime  ()    const         { return _f32ExposureTime; }
  float             coolingTemp   ()    const         { return _f32CoolingTemp; }
  uint16_t          gainIndex     ()    const         { return _u16GainIndex; }
  uint16_t          readoutSpeedIndex() const         { return _u16ReadoutSpeedIndex; }
  uint32_t          maskedHeight  ()    const         { return _u32MaskedHeight; }
  uint32_t          kineticHeight ()    const         { return _u32KineticHeight; }
  float             vsSpeed       ()    const         { return _f32VsSpeed; }
  int16_t           infoReportInterval() const        { return _i16InfoReportInterval; }
  uint16_t          exposureEventCode() const         { return _u16ExposureEventCode; }
  uint32_t          numDelayShots()     const         { return _u32NumDelayShots; }

  uint32_t          setWidth    (uint32_t uWidth)     { return _u32Width = uWidth; }
  uint32_t          setHeight   (uint32_t uHeight)    { return _u32Height = uHeight; }
  uint16_t          setReadoutSpeedIndex
                            (uint16_t uSpeedIndex)    { return _u16ReadoutSpeedIndex = uSpeedIndex; }
  uint32_t          setNumDelayShots
                            (uint32_t uNumDelayShots) { return _u32NumDelayShots = uNumDelayShots;  }

  int               size      ()        const         { return sizeof(*this); }
  int               frameSize ()        const; // calculate the frame size based on the current ROI and binning settings

  ConfigV5(const ConfigV1& configV1);
  ConfigV5(const ConfigV2& configV2);
  ConfigV5(const ConfigV3& configV3);
  ConfigV5(const ConfigV4& configV4);

private:
  uint32_t          _u32Width, _u32Height;
  uint32_t          _uOrgX,  _uOrgY;
  uint32_t          _uBinX,  _uBinY;
  float             _f32ExposureTime;
  float             _f32CoolingTemp;
  uint16_t          _u16GainIndex;
  uint16_t          _u16ReadoutSpeedIndex;
  uint32_t          _u32MaskedHeight;
  uint32_t          _u32KineticHeight;
  float             _f32VsSpeed;
  int16_t           _i16InfoReportInterval;
  uint16_t          _u16ExposureEventCode;
  uint32_t          _u32NumDelayShots;
};

} // namespace Princeton

} // namespace Pds

#pragma pack()

#endif //#ifndef PRINCETON_CONFIG_V4_HH
