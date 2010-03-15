#include <stdio.h>
#include <memory.h>

#include "pdsdata/evr/DataV3.hh"

using namespace Pds;
using namespace EvrData;

DataV3::DataV3(uint32_t u32NumFifoEvents, const FIFOEvent* lFifoEvent) : _u32NumFifoEvents(u32NumFifoEvents)
{
  char *next = (char*) (this + 1);

  memcpy(next, lFifoEvent, _u32NumFifoEvents * sizeof(FIFOEvent));  
}

DataV3::DataV3(const DataV3& dataCopy)
{
  _u32NumFifoEvents = dataCopy.numFifoEvents();
  
  const char *src = (char*) (&dataCopy + 1);
  char *      dst = (char*) (this + 1);
  memcpy(dst, src, _u32NumFifoEvents * sizeof(FIFOEvent));  
}

const FIFOEvent&  DataV3::fifoEvent(unsigned int iEventIndex)  const
{
  const FIFOEvent *lFifoEvent = reinterpret_cast < const FIFOEvent * >(this + 1);
  return lFifoEvent[iEventIndex];
}

unsigned int DataV3::size() const
{
  return ( sizeof(*this) + _u32NumFifoEvents * sizeof(FIFOEvent) );
}

void DataV3::printFifoEvents() const
{
  printf( "# of Fifo Events: %u\n", _u32NumFifoEvents );
  
  for ( unsigned int iEventIndex=0; iEventIndex< _u32NumFifoEvents; iEventIndex++ )
  {
    const FIFOEvent& event = fifoEvent(iEventIndex);
    printf( "[%02u] Event Code %u  TimeStampHigh 0x%x  TimeStampLow 0x%x\n",
      iEventIndex, event.EventCode, event.TimestampHigh, event.TimestampLow );
  }
}

void DataV3::addFifoEvent( const FIFOEvent& fifoEvent )
{  
  FIFOEvent* pFifoEventNew = (FIFOEvent*) ((char*) this + size()); 
  *pFifoEventNew            = fifoEvent;  
  _u32NumFifoEvents++; 
}

void DataV3::clearFifoEvents()
{
  _u32NumFifoEvents = 0;
}

/*
 * static public function
 */
unsigned int DataV3::size(int iMaxNumFifoEvents)
{
  return ( sizeof(DataV3) + iMaxNumFifoEvents * sizeof(FIFOEvent) );
}
