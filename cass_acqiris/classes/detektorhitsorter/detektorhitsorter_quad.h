#ifndef __DetektorHitSorterQuad_H_
#define __DetektorHitSorterQuad_H_

#include "detektorhitsorter.h"

namespace cass
{
    namespace REMI
    {
        class DetectorHitSorterQuad : public DetectorHitSorter
        {
        public:
            virtual ~DetectorHitSorterQuad()							{}

        public:
            virtual void sort(REMIEvent&, Detector&)=0;

        protected:
            void FillHistosBeforeShift(const Detector&);
        };
    }//end namespace remi
}//end namespace cass
#endif
