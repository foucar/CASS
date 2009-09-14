#ifndef __DetektorHitSorterSimple_H_
#define __DetektorHitSorterSimple_H_

#include "DetektorHitSorterQuad.h"

namespace cass
{
    namespace REMI
    {
        //______________________MyDetektorHitSorter Simple Version______________________
        class DetektorHitSorterSimple : public DetektorHitSorterQuad
        {
        public:
            DetektorHitSorterSimple(const DetectorParameter&);

        public:
            void sort(REMIEvent&, Detector&);

        private:
            void sortForTimesum(Detector&);
        };

    }//end namespace remi
}//end namespace cass

#endif
