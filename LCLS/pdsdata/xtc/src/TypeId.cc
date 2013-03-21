#include "pdsdata/xtc/TypeId.hh"

using namespace Pds;

TypeId::TypeId(Type type, uint32_t version, bool cmp) :
  _value((version<<16 )| type | (cmp ? 0x80000000:0)) {}

TypeId::TypeId(const TypeId& v) : _value(v._value) {}

uint32_t TypeId::value() const {return _value;}

uint32_t TypeId::version() const {return (_value&0xffff0000)>>16;}

TypeId::Type TypeId::id() const {return (TypeId::Type)(_value&0xffff);}

bool     TypeId::compressed() const { return _value&0x80000000; }

unsigned TypeId::compressed_version() const { return (_value&0x7fff0000)>>16; }

const char* TypeId::name(Type type)
{ 
   static const char* _names[NumberOf] = {
    "Any",                     // 0
    "Xtc",                     // 1
    "Frame",                   // 2
    "AcqWaveform",             // 3
    "AcqConfig",               // 4
    "TwoDGaussian",            // 5
    "Opal1kConfig",            // 6
    "FrameFexConfig",          // 7
    "EvrConfig",               // 8
    "TM6740Config",            // 9
    "RunControlConfig",        // 10
    "pnCCDframe",              // 11
    "pnCCDconfig",             // 12
    "Epics",                   // 13
    "FEEGasDetEnergy",         // 14
    "EBeamBld",                // 15
    "PhaseCavity",             // 16
    "PrincetonFrame",          // 17
    "PrincetonConfig",         // 18
    "EvrData",                 // 19
    "FrameFccdConfig",         // 20
    "FccdConfig",              // 21
    "IpimbData",               // 22
    "IpimbConfig",             // 23
    "EncoderData",             // 24
    "EncoderConfig",           // 25
    "EvrIOConfig",             // 26
    "PrincetonInfo",           // 27
    "CspadElement",            // 28
    "CspadConfig",             // 29
    "IpmFexConfig",            // 30
    "IpmFex",                  // 31
    "DiodeFexConfig",          // 32
    "DiodeFex",                // 33
    "PimImageConfig",          // 34
    "SharedIpimb",             // 35
    "AcqTDCConfig",            // 36
    "AcqTDCData",              // 37
    "Index",                   // 38
    "XampsConfig",             // 39
    "XampsElement",            // 40
    "Cspad2x2Element",         // 41
    "SharedPIM",               // 42
    "Cspad2x2Config",          // 43
    "FexampConfig",            // 44
    "FexampElement",           // 45
    "Gsc16aiConfig",           // 46
    "Gsc16aiData",             // 47
    "PhasicsConfig",           // 48
    "TimepixConfig",           // 49
    "TimepixData",             // 50
    "CspadCompressedElement",  // 51
    "OceanOpticsConfig",       // 52
    "OceanOpticsData",         // 53
    "EpicsConfig",             // 54
    "FliConfig",               // 55
    "FliFrame",                // 56
    "QuartzConfig",            // 57
    "Reserved1",               // 58
    "Reserved2",               // 59
    "AndorConfig",             // 60
    "AndorFrame",              // 61
    "UsdUsbData",              // 62
    "UsdUsbConfig",            // 63
    "GMD",                     // 64
    "SharedAcqADC",            // 65
    "OrcaConfig",              // 66
  };
  return (type < NumberOf ? _names[type] : "-Invalid-");
}
