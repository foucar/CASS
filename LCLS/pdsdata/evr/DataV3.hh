//
//  Class for configuration of the Event Receiver
//
#ifndef Evr_DataV3_hh
#define Evr_DataV3_hh

#include "pdsdata/xtc/TypeId.hh"
#include <stdint.h>

#pragma pack(4)

namespace Pds
{
namespace EvrData
{

class FIFOEvent;

class DataV3
{
public:
  enum { Version = 3 };
  
  DataV3(uint32_t u32NumFifoEvents, const FIFOEvent* lFifoEvent);
  DataV3(const DataV3& dataCopy);
  
    
  uint32_t          numFifoEvents() const  { return _u32NumFifoEvents; }      
  const FIFOEvent&  fifoEvent(unsigned int iEventIndex)  const;

  unsigned int      size() const;

  void              printFifoEvents () const;
  void              addFifoEvent    ( const FIFOEvent& fifoEvent ); // return the number of total fifo events, including the new one
  void              clearFifoEvents ();
  
  /*
   * public static function
   */  
  static unsigned int size(int iMaxNumFifoEvents);
  
private:
  uint32_t _u32NumFifoEvents;
};

/*
 * Copied from /reg/g/pcds/package/external/evgr_V00-00-02/evgr/evr/evr.hh
 */
class FIFOEvent 
{
public:
  uint32_t TimestampHigh;
  uint32_t TimestampLow;
  uint32_t EventCode;
};

} // namespace EvrData
} // namespace Pds

#pragma pack()

#endif
