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
 float            f32ExposureTime, 
 float            f32CoolingTemp, 
 uint32_t         u32ReadoutSpeedIndex,
 uint16_t         u16ReadoutEventCode, 
 uint16_t         u16DelayMode) :
 _uWidth  (uWidth), 
 _uHeight (uHeight), 
 _uOrgX   (uOrgX), 
 _uOrgY   (uOrgY), 
 _uBinX   (uBinX), 
 _uBinY   (uBinY),
 _f32ExposureTime       (f32ExposureTime),
 _f32CoolingTemp        (f32CoolingTemp), 
 _u32ReadoutSpeedIndex  (u32ReadoutSpeedIndex), 
 _u16ReadoutEventCode   (u16ReadoutEventCode),
 _u16DelayMode          (u16DelayMode)
 {}

int ConfigV1::frameSize() const
{
  return sizeof(FrameV1) + _uWidth* _uHeight * 2; // 2 -> 16 bit color depth
  //return sizeof(FrameV1) + 4*1024*1024*2; // 2 -> 16 bit color depth // !! debug
}
