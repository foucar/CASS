#include "pdsdata/princeton/ConfigV1.hh"
#include "pdsdata/princeton/FrameV1.hh"

#include <string.h>

using namespace Pds;
using namespace Princeton;

ConfigV1::ConfigV1(
 uint32_t         uWidth, 
 uint32_t         uHeight, 
 uint32_t         uOrgX, 
 uint32_t         uOrgY, 
 uint32_t         uBinX, 
 uint32_t         uBinY,
 int16_t          i16CoolingTemp, 
 EnumExposureMode enumExposureMode, 
 float            f32ExposureTime, 
 int16_t          i16ReadoutSpeedIndex,
 uint8_t          u8DelayMode) :
 _uWidth  (uWidth), 
 _uHeight (uHeight), 
 _uOrgX   (uOrgX), 
 _uOrgY   (uOrgY), 
 _uBinX   (uBinX), 
 _uBinY   (uBinY),
 _i16CoolingTemp        (i16CoolingTemp), 
 _i16ExposureMode       ((int16_t)enumExposureMode), 
 _f32ExposureTime       (f32ExposureTime),
 _i16ReadoutSpeedIndex  (i16ReadoutSpeedIndex), 
 _u8DelayMode           (u8DelayMode)
 {}

int ConfigV1::frameSize() const
{
  //return sizeof(FrameV1) + _uWidth* _uHeight * 2; // 2 -> 16 bit color depth
  return sizeof(FrameV1) + 4*1024*1024*2; // 2 -> 16 bit color depth // !! debug
  //return sizeof(FrameV1) + 0; // 2 -> 16 bit color depth // !! debug
}
