#include "pdsdata/encoder/ConfigV1.hh"
#include "pds/encoder/driver/pci3e-wrapper.hh"
#include <stdio.h>
#include <stdint.h>

using namespace Pds;
using namespace Encoder;

static const unsigned Version = 1;

void Pds::Encoder::ConfigV1::dump() const
{
  printf( "------Encoder Config-------------\n" );
  printf( "Channel #: %d\n", _chan_num );
  printf( "Encoder counting mode: %d (%s)",
          _count_mode,
          PCI3E::count_mode_to_name[_count_mode] );
  printf( "Encoder quadrature mode: %d (%s)",
          _quadrature_mode,
          PCI3E::quad_mode_to_name[_quadrature_mode] );
  printf( "External input for trigger: %d\n", _input_num );
  printf( "Trigger on edge: %d (%s)\n",
          _input_rising,
          _input_rising ? "Rising" : "Falling" );
}

Pds::Encoder::ConfigV1::ConfigV1( uint8_t chan_num,
                                  uint8_t count_mode,
                                  uint8_t quadrature_mode,
                                  uint8_t input_num,
                                  uint8_t input_rising ) 
   : _chan_num( chan_num ),
     _count_mode( count_mode ),
     _quadrature_mode( quadrature_mode ),
     _input_num( input_num ),
     _input_rising( input_rising )
{}
