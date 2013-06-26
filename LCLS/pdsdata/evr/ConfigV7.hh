//
//  Class for configuration of the Event Receiver
//
#ifndef Evr_ConfigV7_hh
#define Evr_ConfigV7_hh

#include <stdint.h>

#include "pdsdata/xtc/TypeId.hh"
#include "pdsdata/evr/EventCodeV6.hh"
#include "pdsdata/evr/PulseConfigV3.hh"
#include "pdsdata/evr/OutputMapV2.hh"
#include "pdsdata/evr/SequencerConfigV1.hh"

#pragma pack(4)

namespace Pds
{

namespace EvrData
{

class ConfigV7
{
  /*
   * Data layout:
   *
   * ---------------
   * Data members in this class
   * --------------- 
   * Event Code configurations  (array of class EventCodeType)
   * Pulse configurations       (array of class PulseType)
   * Output Map configurations  (array of class OutputMap)
   * Sequencer configuration    (class SeqConfigType)
   */
public:
  enum { Version = 7 };

  enum { MaxPulses  = 256 }; // Maximum pulses in the system
  enum { MaxOutputs = 256 }; // Maximum outputs in the system

  typedef EventCodeV6       EventCodeType;  
  typedef PulseConfigV3     PulseType;  
  typedef OutputMapV2       OutputMapType;  
  typedef SequencerConfigV1 SeqConfigType;

  ConfigV7(
    uint32_t neventcodes, const EventCodeType* eventcodes,
    uint32_t npulses,     const PulseType*     pulses,
    uint32_t noutputs,    const OutputMapType* outputs,
    const SeqConfigType& seq_config);    

  //  event codes appended to this structure   
  uint32_t                neventcodes ()          const;
  const  EventCodeType&   eventcode   (unsigned)  const;

  //  pulse configurations appended to this structure
  uint32_t          npulses     ()          const;
  const PulseType&  pulse       (unsigned)  const;

  //  logical output configurations appended to this structure
  uint32_t              noutputs    ()          const;
  const OutputMapType&  output_map  (unsigned)  const;
  
  const SeqConfigType&  seq_config() const;

  //  size including appended EventCode's, PulseType's and OutputMap's
  unsigned        size() const;  
  void            print() const; // print out values
  
private:
  uint32_t _neventcodes;
  uint32_t _npulses;
  uint32_t _noutputs;
};

} // namespace EvrData
} // namespace Pds

#pragma pack()

#endif
