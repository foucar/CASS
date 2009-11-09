// pnCCDEvent.h : Nils Kimmel 2009
// The pnCCD detector data that result from one machine event
// of e.g. a Free Electron Laser. This data collection is pipe-
// lined through the operator() of the class cass::pnCCD::Analysis
// and generates the basic analysis products for a user of raw
// pnCCD frames.


#include <vector>
#include <stdint.h>

#include "pnccd_detector.h"


#ifndef PNCCD_EVENT_H
#define PNCCD_EVENT_H

#ifndef ROOT_Rtypes
#include "Rtypes.h"
#endif

#ifndef ROOT_TObject
#include "TObject.h"
#endif

	
namespace cass
{
    namespace pnCCD
    {
        class pnCCDEvent
        {
        public:
            //some typedefs for more readable code
            typedef std::vector<pnCCDDetector> detectors_t;

        public:
            pnCCDEvent(void)    {}
            ~pnCCDEvent()       {}

        public:
            const detectors_t   &detectors()const   {return _detectors;}
            detectors_t         &detectors()        {return _detectors;}

        private:
            detectors_t          _detectors;     //a vector containing all detectors


            ClassDefNV(pnCCDEvent,1)            //the pnccd event
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


