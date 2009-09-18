// pnccd_photon_hit.h
// Declaration of a structure which describes a photon hit on
// a pnCCD. For use in the in the pnCCD library for cass.

#ifndef PNCCD_PHOTON_HIT_H
#define PNCCD_PHOTON_HIT_H

namespace cass
{
  namespace pnCCD
  {

    typedef struct raw_evt_t
    {
      uint16_t x;        // x coordinate
      uint16_t y;        // y coordinate
      uint16_t amp;      // signal amplitude in adu
      float    energy;   // energy in eV
      uint64_t index;    // running index of the event
      uint64_t frm_idx;  // index of the frame where the event
                         // was detected
    } pnccd_photon_hit;

  } // end of scope of namespace pnCCD
} // end of scope of namespace cass

#endif // PNCCD_PHOTON_HIT_H

// Local Variables:
// coding: utf-8
// mode: C++
// c-file-offsets: ((c . 0) (innamespace . 0))
// c-file-style: "Stroustrup"
// fill-column: 100
// End:

// end of file pnCCDEvent.h

