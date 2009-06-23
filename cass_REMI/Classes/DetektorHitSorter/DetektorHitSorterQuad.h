#ifndef __DetektorHitSorterQuad_H_
#define __DetektorHitSorterQuad_H_

#include "DetektorHitSorter.h"

namespace cass
{
    namespace REMI
    {

            //_______________________________________________________________________MyDetektorHitSorter________________________________________________________________________________________
            class DetektorHitSorterQuad : public DetektorHitSorterBase
            {
            public:
                DetektorHitSorterQuad(const cass::REMI::DetectorParameter&p):DetektorHitSorterBase(p)	{}
                virtual ~DetektorHitSorterQuad()							{}

            public:
                virtual void sort(REMIEvent&, Detector&)=0;

            protected:
                void FillHistosBeforeShift(const Detector&);
            };
    }//end namespace remi
}//end namespace cass
#endif
