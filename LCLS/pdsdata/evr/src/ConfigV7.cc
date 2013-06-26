#include "pdsdata/evr/ConfigV7.hh"

#include <stdio.h>
#include <string.h>

using namespace Pds;
using namespace EvrData;

ConfigV7::ConfigV7(
   uint32_t neventcodes,  const EventCodeType*  eventcodes,
   uint32_t npulses,      const PulseType*      pulses,
   uint32_t noutputs,     const OutputMapType*  outputs,
   const SeqConfigType& seq_config ) :   
  _neventcodes(neventcodes),
  _npulses    (npulses), 
  _noutputs   (noutputs)
{
  char *next = (char*) (this + 1);

  memcpy(next, eventcodes, _neventcodes * sizeof(EventCodeType));
  next += _neventcodes * sizeof(EventCodeType);
  
  memcpy(next, pulses, _npulses * sizeof(PulseType));
  next += _npulses * sizeof(PulseType);
  
  memcpy(next, outputs, _noutputs * sizeof(OutputMapType));
  next += _noutputs * sizeof(OutputMapType);

  memcpy(next, &seq_config, seq_config.size());
}

uint32_t ConfigV7::neventcodes() const
{
  return _neventcodes;
}

const ConfigV7::EventCodeType& ConfigV7::eventcode(unsigned eventcodeIndex) const
{
  const EventCodeType *eventcodes = (const EventCodeType *) (this + 1);
  return eventcodes[eventcodeIndex];  
}


uint32_t ConfigV7::npulses() const
{
  return _npulses;
}
const ConfigV7::PulseType& ConfigV7::pulse(unsigned pulse) const
{
  const PulseType *pulses = (const PulseType *) (
    (char *) (this + 1) + _neventcodes * sizeof(EventCodeType) );
    
  return pulses[pulse];
}

uint32_t ConfigV7::noutputs() const
{
  return _noutputs;
}
const ConfigV7::OutputMapType & ConfigV7::output_map(unsigned output) const
{
  const OutputMapType *m = (const OutputMapType *) (
    (char *) (this + 1) + _neventcodes * sizeof(EventCodeType) +
    _npulses * sizeof(PulseType) );

  return m[output];
}

const SequencerConfigV1 & ConfigV7::seq_config() const
{
  return *reinterpret_cast<const SeqConfigType*>(&output_map(_noutputs));
}

unsigned ConfigV7::size() const
{
  return (sizeof(*this) + 
          _neventcodes * sizeof(EventCodeType) +
          _npulses     * sizeof(PulseType) + 
          _noutputs    * sizeof(OutputMapType) +
          seq_config().size());
}

void ConfigV7::print() const
{
  printf("Pds::EvrData::ConfigV7 event %d pulse %d output %d\n", _neventcodes, _npulses, _noutputs);
  
  for (unsigned int i=0; i<_neventcodes; ++i)
  {
    printf("  Event code [%d]\n", i);
    eventcode(i).print();
  }
  
  for (unsigned int i=0; i<_npulses; ++i)
  {
    printf("  Pulse [%d]\n", i);
    pulse(i).print();
  }

  for (unsigned int i=0; i<_noutputs; ++i)
  {
    printf("  Output [%d]\n", i);
    output_map(i).print();
  } 

}
