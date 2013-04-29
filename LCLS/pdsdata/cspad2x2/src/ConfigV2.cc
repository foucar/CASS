#include "pdsdata/cspad2x2/ConfigV2.hh"

using namespace Pds::CsPad2x2;

ConfigV2::ConfigV2(
		   uint32_t inactiveRunMode,
		   uint32_t activeRunMode,
		   uint32_t runTriggerDelay,
		   uint32_t testDataIndex,
		   uint32_t payloadPerQuad,
		   uint32_t badAsicMask,
		   uint32_t AsicMask,
		   uint32_t roiMask) :
  _concentratorVersion(0),
  _protectionEnable(0),
  _inactiveRunMode(inactiveRunMode),
  _activeRunMode(activeRunMode),
  _runTriggerDelay(runTriggerDelay),
  _testDataIndex(testDataIndex),
  _payloadPerQuad(payloadPerQuad),
  _badAsicMask(badAsicMask),
  _AsicMask(AsicMask),
  _roiMask (roiMask)
{
  _protectionThreshold.adcThreshold=67;
  _protectionThreshold.pixelCountThreshold=1200;
}

ProtectionSystemThreshold* ConfigV2::protectionThreshold ()
{
  return &_protectionThreshold;
}

const ProtectionSystemThreshold* ConfigV2::protectionThreshold () const
{
  return &_protectionThreshold;
}

uint32_t* ConfigV2::concentratorVersionAddr()
{
  return &_concentratorVersion;
}

ConfigV2QuadReg* ConfigV2::quad()
{
  return &_quad;
}

const ConfigV2QuadReg* ConfigV2::quad() const
{
  return &_quad;
}

unsigned ConfigV2::roiMask      (int iq) const
{
  return _roiMask&0x3;
}

unsigned ConfigV2::roiMask      () const
{
  return _roiMask&0x3;
}

unsigned ConfigV2::numAsicsRead () const
{
  return  4;
}

unsigned ConfigV2::numAsicsStored(int iq) const
{
  unsigned m = roiMask(0);
  unsigned c;
  for(c=0; m; c++)
    m &= m-1;
  return c<<1;
}
unsigned ConfigV2::numAsicsStored() const
{
  return ConfigV2::numAsicsStored(0);
}
