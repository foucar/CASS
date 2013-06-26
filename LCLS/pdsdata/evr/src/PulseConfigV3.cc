#include "pdsdata/evr/PulseConfigV3.hh"

#include <stdio.h>
#include <string.h>

using namespace Pds;
using namespace EvrData;

PulseConfigV3::PulseConfigV3(
  uint16_t  u16PulseId,  
  uint16_t  u16Polarity,   // 0 -> positive polarity , 1 -> negative polarity
  uint32_t  u32Prescale,  // Clock divider
  uint32_t  u32Delay,     // Delay in 119MHz clks
  uint32_t  u32Width      // Width in 119MHz clks
  ) :
  _u16PulseId   (u16PulseId),
  _u16Polarity  (u16Polarity),
  _u32Prescale  (u32Prescale),
  _u32Delay     (u32Delay),
  _u32Width     (u32Width)
{  
}

void PulseConfigV3::print() const
{
  static const double EvrPeriod = 1./119e6;    
  printf("    Id %d  Polarity %c  Prescale %u Delay %u (%lg ns) Width %u (%lg ns)\n",
    _u16PulseId, (_u16Polarity == 0? '+' : '-'), _u32Prescale, 
    _u32Delay, _u32Delay*EvrPeriod*1e9, _u32Width, _u32Width*EvrPeriod*1e9);
}
