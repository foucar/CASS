// pnCCDEvent.h : Nils Kimmel 2009
// The pnCCD detector data that result from one machine event
// of e.g. a Free Electron Laser. This data collection is pipe-
// lined through the operator() of the class cass::pnCCD::Analysis
// and generates the basic analysis products for a user of raw
// pnCCD frames.

#ifndef PNCCD_EVENT_H
#define PNCCD_EVENT_H

#ifndef ROOT_Rtypes
#include "Rtypes.h"
#endif

#ifndef ROOT_TObject
#include "TObject.h"
#endif

// Include as defined in C99. This should thus hopefully work
// on all systems which support this standard:
#include <vector>
#include "pdsdata/pnCCD/fformat.h"
//#include <inttypes.h>
#include <string.h>
#include <stdint.h>

#include "pnccd_photon_hit.h"

//#include "cass_pnccd.h"
	
namespace cass
{
  namespace pnCCD
  {
  //class CASS_PNCCDSHARED_EXPORT pnCCDEvent
     class pnCCDEvent
     {
     public:
// Initialize the event data structure with the number of
// detectors (pixel arrays) , the size of the detectors ,
// the maximum of photon hits which should be stored in the
// event. This will allocate the necessary memory needed to store
// the data in the event, so be careful and keep in mind that
// memory allocation takes CPU time and space.
	pnCCDEvent(void);
	pnCCDEvent(uint16_t num_pixel_arrays,
                   std::vector<uint32_t> array_x_size,
                   std::vector<uint32_t> array_y_size,
                   std::vector<uint32_t> max_photons_per_event);
        ~pnCCDEvent();
// assignment operator that will only the relevant part of the event
        pnCCDEvent& operator=(const pnCCDEvent&);
// Initialize the event with the file header data from the
// xtc:
        bool init(fileHeaderType *pnccd_fhdr, uint32_t ccd_id);
// Initialize the event with frame data from the xtc:
        bool init(frameHeaderType *pnccd_frame, uint32_t ccd_id);
// Initialize the event storage with the given number of detectors
// and their array sizes:
        bool initEventStorage(void);
// Get access to some private members of this class:
        uint16_t              getNumPixArrays(void);
        std::vector<uint32_t> getArrXSize(void);
        std::vector<uint32_t> getArrYSize(void);
        std::vector<uint32_t> getMaxPhotPerEvt(void);
// Return the address of the raw signal array at the index
// index. Returns zero if no array with this index is allocated.
        uint16_t* rawSignalArrayAddr(uint16_t index);
        uint32_t  rawSignalArrayByteSize(uint16_t index);
// Return the address of the offset and, optionally common
// mode corrected signal value array:
        uint16_t* corrSignalArrayAddr(uint16_t index);
        uint32_t  corrSignalArrayByteSize(uint16_t index);
// Return the address of the unrecombined photon hit array:
        pnccd_photon_hit* unrecPhotonHitAddr(uint16_t index);
        uint32_t          numUnrecPhotonHits(uint16_t index);
// Return the address of the recombined photon hit array:
        pnccd_photon_hit* recomPhotonHitAddr(uint16_t index);
        uint32_t          numRecomPhotonHits(uint16_t index);
      private:
        uint16_t num_pixel_arrays_;
// These variables are set by the second constructor. The allocation
// of the local storage arrays is performed by the initEventStorage()
// member function.
        std::vector<uint32_t> array_x_size_;
	std::vector<uint32_t> array_y_size_;
	std::vector<uint32_t> max_photons_per_event_;
// The storage arrays for:
// -> detector array raw data
// -> offset and, if desired, common mode corrected raw data.
//    this is the most basic analysis product and corresponds
//    to the pixel signal map (without averaging) in Xonline.
// -> non-recombined X-ray photon hits which correspond to a
//    pixel signal which is above the signal threshold defined
//    in cass::pnCCD::Parameter.
// -> recombined X-ray photon hits which are created from the
//    non-recombined hits. Neighboring photon hits are grouped
//    into one hit which contains the sum of the recombined
//    pulse heights.
        std::vector<uint16_t*>          raw_signal_values_;
        std::vector<uint16_t*>          corr_signal_values_;
        std::vector<pnccd_photon_hit*>  unrec_photon_hits_;
        std::vector<uint32_t>           num_unrec_phits_;
        std::vector<pnccd_photon_hit*>  recom_photon_hits_;
        std::vector<uint32_t>           num_recom_phits_;
        ClassDefNV(pnCCDEvent,1)
      };
    } // end of scope of namespace pnCCD
} // end of scope of namespace cass


#endif // PNCCD_EVENT_H

// Local Variables:
// coding: utf-8
// mode: C++
// c-file-offsets: ((c . 0) (innamespace . 0))
// c-file-style: "Stroustrup"
// fill-column: 100
// End:

// end of file pnCCDEvent.h


