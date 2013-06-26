#include "pdsdata/fccd/FccdConfigV2.hh"
#include "pdsdata/camera/FrameCoord.hh"

#include <string.h>

using namespace Pds;
using namespace FCCD;

FccdConfigV2::FccdConfigV2 (
  uint16_t    outputMode,
  bool        ccdEnable,
  bool        focusMode,
  uint32_t    exposureTime,
  float       dacVoltage1,
  float       dacVoltage2,
  float       dacVoltage3,
  float       dacVoltage4,
  float       dacVoltage5,
  float       dacVoltage6,
  float       dacVoltage7,
  float       dacVoltage8,
  float       dacVoltage9,
  float       dacVoltage10,
  float       dacVoltage11,
  float       dacVoltage12,
  float       dacVoltage13,
  float       dacVoltage14,
  float       dacVoltage15,
  float       dacVoltage16,
  float       dacVoltage17,
  uint16_t    waveform0,
  uint16_t    waveform1,
  uint16_t    waveform2,
  uint16_t    waveform3,
  uint16_t    waveform4,
  uint16_t    waveform5,
  uint16_t    waveform6,
  uint16_t    waveform7,
  uint16_t    waveform8,
  uint16_t    waveform9,
  uint16_t    waveform10,
  uint16_t    waveform11,
  uint16_t    waveform12,
  uint16_t    waveform13,
  uint16_t    waveform14
  ) :
  _outputMode             (outputMode),
  _ccdEnable              (ccdEnable),
  _focusMode              (focusMode),
  _exposureTime           (exposureTime),
  _dacVoltage1            (dacVoltage1),
  _dacVoltage2            (dacVoltage2),
  _dacVoltage3            (dacVoltage3),
  _dacVoltage4            (dacVoltage4),
  _dacVoltage5            (dacVoltage5),
  _dacVoltage6            (dacVoltage6),
  _dacVoltage7            (dacVoltage7),
  _dacVoltage8            (dacVoltage8),
  _dacVoltage9            (dacVoltage9),
  _dacVoltage10           (dacVoltage10),
  _dacVoltage11           (dacVoltage11),
  _dacVoltage12           (dacVoltage12),
  _dacVoltage13           (dacVoltage13),
  _dacVoltage14           (dacVoltage14),
  _dacVoltage15           (dacVoltage15),
  _dacVoltage16           (dacVoltage16),
  _dacVoltage17           (dacVoltage17),
  _waveform0              (waveform0),
  _waveform1              (waveform1),
  _waveform2              (waveform2),
  _waveform3              (waveform3),
  _waveform4              (waveform4),
  _waveform5              (waveform5),
  _waveform6              (waveform6),
  _waveform7              (waveform7),
  _waveform8              (waveform8),
  _waveform9              (waveform9),
  _waveform10             (waveform10),
  _waveform11             (waveform11),
  _waveform12             (waveform12),
  _waveform13             (waveform13),
  _waveform14             (waveform14)
{}
