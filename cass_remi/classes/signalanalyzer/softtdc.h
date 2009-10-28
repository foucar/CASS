#ifndef __SoftTDC_h__
#define __SoftTDC_h__

#include <vector>
#include <iostream>

namespace cass
{
    namespace REMI
    {
        class REMIEvent;
        //this class is placeholder for two other classes wich will be called
        //according to how many bits the instrument has
        class SoftTDC
        {
        public:
            virtual ~SoftTDC() {}
            virtual void FindPeaksIn(REMIEvent&)=0;
        };

        //this class does nothing
        class SoftTDCDoNothing : public SoftTDC
        {
        public:
            SoftTDCDoNothing(){std::cout << "using do nothing Softtdc"<<std::endl;}
            void FindPeaksIn(REMIEvent&)    {}
        };
    }//end namespace remi
}//end namespace cass
#endif
