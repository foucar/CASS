#ifndef __DetektorHitSorterSimple_H_
#define __DetektorHitSorterSimple_H_

#include "detektorhitsorter_quad.h"

namespace cass
{
    namespace REMI
    {
        //______________________MyDetektorHitSorter Simple Version______________________
        class DetectorHitSorterSimple : public DetectorHitSorterQuad
        {
        public:
            void sort(REMIEvent&, Detector&);

        private:
            void sortForTimesum(Detector&);
        };

    }//end namespace remi
}//end namespace cass

#endif
