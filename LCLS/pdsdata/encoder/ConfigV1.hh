#ifndef ENCODERCONFIGV1_HH
#define ENCODERCONFIGV1_HH

#include "pdsdata/xtc/TypeId.hh"
#include <stdint.h>

namespace Pds
{
   namespace Encoder
   {
      class ConfigV1;
   }
}

class Pds::Encoder::ConfigV1
{
 public:
   enum { Version = 1 };

   ConfigV1() {}
   ConfigV1( uint8_t chan_num,
             uint8_t count_mode,
             uint8_t quadrature_mode,
             uint8_t input_num,
             uint8_t input_rising );
   ~ConfigV1() {}

   static Pds::TypeId typeId()
      { return TypeId( TypeId::Id_EncoderConfig, Version ); }

   void dump() const;

   uint8_t _chan_num;
   uint8_t _count_mode;
   uint8_t _quadrature_mode;
   uint8_t _input_num;
   uint8_t _input_rising;  
};

#endif
