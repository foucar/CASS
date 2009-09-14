#ifndef __DetektorHitSorter_H_
#define __DetektorHitSorter_H_

#include <vector>


namespace cass
{
    namespace REMI
    {
        class Parameter;
        class DetectorParameter;
        class Detector;
        class REMIEvent;
        //__________________________MyDetektorHitSorter_____________________________
        class DetektorHitSorterBase
        {
        public:
            DetektorHitSorterBase(const DetectorParameter&) {}
            virtual ~DetektorHitSorterBase()                {}

        public:
            virtual void sort(REMIEvent&, Detector&)=0;

        protected:
        };
        typedef std::vector<DetektorHitSorterBase*> dethitsorters_t;

        //the actual worker
        class DetektorHitSorter
        {
        public:
            DetektorHitSorter()     {}
            ~DetektorHitSorter();

        public:
            enum ESorterMethod {kSimple=0, kAchim, kDoNothing};
            void init(const Parameter&);
            void sort(REMIEvent&);

        private:
            dethitsorters_t     fDhs;	//vector containing pointers to the sorters for each detector
        };
    }//end namespace remi
}//end namespace cass
#endif
