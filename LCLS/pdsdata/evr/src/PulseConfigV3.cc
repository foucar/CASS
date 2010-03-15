#include "pdsdata/evr/PulseConfigV3.hh"

#include <string.h>

using namespace Pds;
using namespace EvrData;

PulseConfigV3::PulseConfigV3(
  uint8_t   u8PulseId,  
  uint8_t   u8Polarity,   // 0 -> positive polarity , 1 -> negative polarity
  uint32_t  u32Prescale,  // Clock divider
  uint32_t  u32Delay,     // Delay in 119MHz clks
  uint32_t  u32Width      // Width in 119MHz clks
  ) :
  _u8PulseId  (u8PulseId),
  _u8Polarity (u8Polarity),
  _u32Prescale(u32Prescale),
  _u32Delay   (u32Delay),
  _u32Width   (u32Width)
{  
}
