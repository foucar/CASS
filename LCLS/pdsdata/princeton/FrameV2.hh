#ifndef PRINCETON_FRAME_V2_HH
#define PRINCETON_FRAME_V2_HH

#include <stdio.h>
#include <stdint.h>
#include <stdexcept>

#pragma pack(4)

namespace Pds
{

namespace Princeton
{

class FrameV2
{
public:
  static const int    Version = 2;
  static const float  TemperatureNotDefined = -9999;

  FrameV2( uint32_t  iShotIdStart, float fReadoutTime );

  uint32_t          shotIdStart () const { return _iShotIdStart; }
  float             readoutTime () const { return _fReadoutTime; }
  float             temperature () const { return _fTemperature; }

  const uint16_t*   data        ()        const;

  void              setTemperature(float fTemperature) {_fTemperature = fTemperature;}

private:
  uint32_t  _iShotIdStart;
  float     _fReadoutTime;
  float     _fTemperature;
};


} // namespace Princeton

} // namespace Pds

#pragma pack()

#endif
