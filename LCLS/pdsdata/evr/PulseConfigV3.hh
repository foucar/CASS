#ifndef Evr_PulseConfigV3_hh
#define Evr_PulseConfigV3_hh

#include <stdint.h>

#pragma pack(4)

namespace Pds
{
namespace EvrData
{

class PulseConfigV3
{
public:
  PulseConfigV3(
    uint8_t   u8PulseId,  
    uint8_t   u8Polarity,       // 0 -> positive polarity , 1 -> negative polarity
    uint32_t  u32Prescale = 1,  // Clock divider
    uint32_t  u32Delay    = 0,  // Delay in 119MHz clks
    uint32_t  u32Width    = 0   // Width in 119MHz clks
    );
    
  PulseConfigV3() {} // For array initialization
    
  uint8_t   pulseId () const { return _u8PulseId; }
  uint8_t   polarity() const { return _u8Polarity; }
  uint32_t  prescale() const { return _u32Prescale; }
  uint32_t  delay   () const { return _u32Delay; }
  uint32_t  width   () const { return _u32Width; }
  
  void      setPulseId(uint8_t u8PulseId) { _u8PulseId = u8PulseId; }
  
private:
  uint8_t   _u8PulseId;
  uint8_t   _u8Polarity;
  uint32_t  _u32Prescale;
  uint32_t  _u32Delay;
  uint32_t  _u32Width;  
};

} // namespace EvrData
} // namespace Pds

#pragma pack()

#endif
