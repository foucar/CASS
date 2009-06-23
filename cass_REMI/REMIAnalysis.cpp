#include "REMIAnalysis.h"
#include "REMIEvent.h"

void cass::REMI::Analysis::init(const cass::REMI::Parameter& param)
{
	//copy the parameters//
	fParam = param;
	//intitalize the signal analyzer//
    fSiganalyzer.init(param.fPeakfindingMethod);
	//initialize the Detectorhit sorter for each detector//
	fSorter.init(param);
}

void cass::REMI::Analysis::operator()(cass::REMI::REMIEvent& remievent)
{
	//find the peaks in the signals of all channels//
    fSiganalyzer.findPeaksIn(remievent);
	
	//extract the peaks for the layers//
	//and sort the peaks for detektor hits//
	//fill the results in the Cass Event//
	//this has to be done for each detektor individually//
    fSorter.sort(remievent);
}
