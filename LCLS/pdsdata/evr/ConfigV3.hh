//
//  Class for configuration of the Event Receiver
//
#ifndef Evr_ConfigV3_hh
#define Evr_ConfigV3_hh

#include <stdint.h>

#include "pdsdata/xtc/TypeId.hh"
#include "pdsdata/evr/EventCodeV3.hh"
#include "pdsdata/evr/PulseConfigV3.hh"
#include "pdsdata/evr/OutputMap.hh"

#pragma pack(4)

namespace Pds
{

namespace EvrData
{

class ConfigV3
{
  /*
   * Data layout:
   *
   * ---------------
   * Data members in this class
   * --------------- 
   * Event Code configurations  (array of class EventCodeV3)
   * Pulse configurations       (array of class PulseConfigV3)
   * Output Map configurations  (array of class OutputMap)
   */
public:
  enum { Version = 3 };
  
  typedef EventCodeV3   EventCodeType;  
  typedef PulseConfigV3 PulseType;  
  typedef OutputMap     OutputMapType;  

  ConfigV3(
    uint32_t neventcodes, const EventCodeV3*    eventcodes,
    uint32_t npulses,     const PulseConfigV3*  pulses,
    uint32_t noutputs,    const OutputMap*      outputs );    

  //  event codes appended to this structure   
  uint32_t              neventcodes ()          const;
  const  EventCodeV3&   eventcode   (unsigned)  const;

  //  pulse configurations appended to this structure
  uint32_t              npulses     ()          const;
  const PulseConfigV3&  pulse       (unsigned)  const;

  //  output configurations appended to this structure
  uint32_t              noutputs    ()          const;
  const OutputMap&      output_map  (unsigned)  const;

  //  size including appended EventCode's, PulseConfigV3's and OutputMap's
  unsigned        size() const;
  
private:
  uint32_t _neventcodes;
  uint32_t _npulses;
  uint32_t _noutputs;
};

} // namespace EvrData
} // namespace Pds

#pragma pack()

#endif
