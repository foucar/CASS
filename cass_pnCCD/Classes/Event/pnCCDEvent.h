// pnCCDEvent.h
// The pnCCD detector data that result from one machine event
// of e.g. a Free Electron Laser. This data collection is pipe-
// lined through the operator() of the class cass::pnCCD::Analysis
// and generates the basic analysis products for a user of raw
// pnCCD frames.

#ifndef PNCCD_EVENT_H
#define PNCCD_EVENT_H

namespace cass {
namespace pnCCD {

class pnCCDEvent {
public:
// Initialize the event data structure with the number of
// detectors (pixel arrays) , the size of the detectors ,
// the maximum of photon hits which should be stored in the
// event. This will allocate the necessary memory needed to store
// the data in the event, so be careful and keep in mind that
// memory allocation takes CPU time and space.
    pnCCDEvent(int num_pixel_arrays,
	       std::vector<int> array_x_size,
	       std::vector<int> array_y_size,
	       std::vector<int> max_photons_per_event);
    ~pnCCDEvent();
private:
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


