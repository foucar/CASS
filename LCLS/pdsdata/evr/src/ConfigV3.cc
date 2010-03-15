#include "pdsdata/evr/ConfigV3.hh"
#include "pdsdata/evr/EventCodeV3.hh"
#include "pdsdata/evr/PulseConfigV3.hh"
#include "pdsdata/evr/OutputMap.hh"

#include <string.h>

using namespace Pds;
using namespace EvrData;

ConfigV3::ConfigV3(
   uint32_t neventcodes,  const EventCodeV3*    eventcodes,
   uint32_t npulses,      const PulseConfigV3*  pulses,
   uint32_t noutputs,     const OutputMap*      outputs 
   ) :   
  _neventcodes(neventcodes),
  _npulses    (npulses), 
  _noutputs   (noutputs)
{
  char *next = (char*) (this + 1);

  memcpy(next, eventcodes, _neventcodes * sizeof(EventCodeV3));
  next += _neventcodes * sizeof(EventCodeV3);
  
  memcpy(next, pulses, _npulses * sizeof(PulseConfigV3));
  next += _npulses * sizeof(PulseConfigV3);
  
  memcpy(next, outputs, _noutputs * sizeof(OutputMap));
}

uint32_t ConfigV3::neventcodes() const
{
  return _neventcodes;
}

const EventCodeV3& ConfigV3::eventcode(unsigned eventcodeIndex) const
{
  const EventCodeV3 *eventcodes = (const EventCodeV3 *) (this + 1);
  return eventcodes[eventcodeIndex];  
}


uint32_t ConfigV3::npulses() const
{
  return _npulses;
}
const PulseConfigV3& ConfigV3::pulse(unsigned pulse) const
{
  const PulseConfigV3 *pulses = (const PulseConfigV3 *) (
    (char *) (this + 1) + _neventcodes * sizeof(EventCodeV3) );
    
  return pulses[pulse];
}

uint32_t ConfigV3::noutputs() const
{
  return _noutputs;
}
const OutputMap & ConfigV3::output_map(unsigned output) const
{
  const OutputMap *m = (const OutputMap *) (
    (char *) (this + 1) + _neventcodes * sizeof(EventCodeV3) +
    _npulses * sizeof(PulseConfigV3) );

  return m[output];
}


unsigned ConfigV3::size() const
{
  return (sizeof(*this) + 
   _neventcodes * sizeof(EventCodeV3) +
   _npulses     * sizeof(PulseConfigV3) + 
   _noutputs    * sizeof(OutputMap));
}

unsigned ConfigV3::size(unsigned maxNumEventCodes, unsigned maxNumPulses, unsigned maxNumOutputMaps)
{
  return (sizeof(ConfigV3) + 
    maxNumEventCodes * sizeof(EventCodeV3) +
    maxNumPulses     * sizeof(PulseConfigV3) + 
    maxNumOutputMaps    * sizeof(OutputMap));
}
  
uint8_t ConfigV3::opcodeFromBeamRate(BeamCode bc, RateCode rc) 
{
  static const unsigned beamOn     = 100;
  static const unsigned baseRate   = 40;
  static const unsigned singleShot = 150;
  
  unsigned v;
  if (rc==Single) {
    v = singleShot;
  }
  else {
    v = baseRate+unsigned(rc);
    if (bc==On) v += beamOn;
  }
  return v; 
}
