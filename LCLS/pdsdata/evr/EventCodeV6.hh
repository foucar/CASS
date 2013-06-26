#ifndef Evr_EventCodeV6_hh
#define Evr_EventCodeV6_hh

#include <stdint.h>

#pragma pack(4)

namespace Pds
{
namespace EvrData
{

class EventCodeV6
{
public:
  enum { DescSize = 16 };
  enum { MaxReadoutGroup = 7 };

  EventCodeV6() {}                         // For array initialization

  EventCodeV6(uint16_t    u16Code,         // Command
        const char* desc);

  EventCodeV6(uint16_t    u16Code,         // Readout code
        uint16_t    u16ReadGroup,
        const char* desc,
        uint32_t    u32MaskTrigger,
        uint32_t    u32MaskSet    , 
        uint32_t    u32MaskClear);
    
  EventCodeV6(uint16_t    u16Code,         // External control
        const char* desc,
        bool        bLatch,
        uint32_t    u32ReportDelay = 0,
        uint32_t    u32ReportWidth = 1);

  EventCodeV6(uint16_t    u16Code,         // Generic
        uint16_t    u16ReadGroup,
        const char* desc,
        bool        bReadout,
        bool        bCommand,    
        bool        bLatch,
        uint32_t    u32ReportDelay = 0,
        uint32_t    u32ReportWidth = 1,
        uint32_t    u32MaskTrigger = 0,
        uint32_t    u32MaskSet     = 0, 
        uint32_t    u32MaskClear   = 0
        );
    
  EventCodeV6(const EventCodeV6&);

  uint16_t    code      () const { return _u16Code; }
  const char* desc      () const { return _desc; }

  bool      isReadout   () const { return ( _u16MaskEventAttr & (1<<EventAttrBitReadout) )     != 0; }
  bool      isCommand   () const { return ( _u16MaskEventAttr & (1<<EventAttrBitCommand) )     != 0; }
  bool      isLatch     () const { return ( _u16MaskEventAttr & (1<<EventAttrBitLatch) )       != 0; }

  uint32_t  reportDelay () const { return _u32ReportDelay; }
  uint32_t  reportWidth () const { return _u32ReportWidth; }
  uint32_t  releaseCode () const { return _u32ReportWidth; }

  uint32_t  maskTrigger () const { return _u32MaskTrigger; }
  uint32_t  maskSet     () const { return _u32MaskSet; }
  uint32_t  maskClear   () const { return _u32MaskClear; }
  
  int16_t  readoutGroup () const { return _u16ReadGroup; }
  
  void     clearMask    (unsigned uTypeBit);
  void     setMaskBit   (unsigned uTypeBit, uint32_t u32MaskBit);  
  void     clearMaskBit (unsigned uTypeBit, uint32_t u32MaskBit);  
  
public:  
  void     print()        const;

private:
  enum EventAttrBitEnum { EventAttrBitReadout = 0, EventAttrBitCommand = 1, EventAttrBitLatch = 2 };
    
  uint16_t _u16Code;
  uint16_t _u16MaskEventAttr;
  uint32_t _u32ReportDelay;
  uint32_t _u32ReportWidth;
  uint32_t _u32MaskTrigger;
  uint32_t _u32MaskSet;
  uint32_t _u32MaskClear;
  char     _desc[DescSize];
  uint16_t _u16ReadGroup;  
};

} // namespace EvrData
} // namespace Pds

#pragma pack()

#endif
