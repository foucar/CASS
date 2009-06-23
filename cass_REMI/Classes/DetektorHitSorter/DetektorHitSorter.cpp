#include <iostream>

#include "DetektorHitSorter.h"

#include "DetektorHitSorterSimple.h"

#include "REMIAnalysis.h"
#include "REMIEvent.h"
#include "Detector.h"
#include "Channel.h"


//______________________________________________the acutal worker_____________________________________________________________________________________________________________
cass::REMI::DetektorHitSorter::~DetektorHitSorter()
{
	//std::cout <<"delete dethitsorter"<<std::endl;
	for (size_t i=0;i<fDhs.size();++i)
		delete fDhs[i];
	fDhs.clear();
	//std::cout <<"done"<<std::endl;
}

//______________________________________________the sorting_____________________________________________________________________________________________________________
void cass::REMI::DetektorHitSorter::sort(cass::REMI::REMIEvent& e)
{
	for (size_t i=0; i<fDhs.size();++i)
        fDhs[i]->sort(e,e.detector(i));
}


//______________________________________________initialization_____________________________________________________________________________________________________________
void cass::REMI::DetektorHitSorter::init(const cass::REMI::Parameter& param)
{
	//clear the old sorters//
	for (size_t i=0;i<fDhs.size();++i)
		delete fDhs[i];
	fDhs.clear();

	//create a sorter for each Detektor
    for (size_t i=0;i<param.fDetectorParameters.size();++i)
	{
        switch (param.fDetectorParameters[i].fSortMethod)
		{
        case(kSimple):fDhs.push_back(new DetektorHitSorterSimple(param.fDetectorParameters[i])); break;
        //case(kDoNothing):fDhs.push_back(new DetektorHitSorterDoNothing(param.fDetPara[i]); break;
        default:std::cerr << "Sorter Method "<<param.fDetectorParameters[i].fSortMethod<<" not available"<<std::endl;break;
		}
	}
}
