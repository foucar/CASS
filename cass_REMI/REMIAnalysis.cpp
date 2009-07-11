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
    //copy the parameters to the event//
    remievent.CopyParameters(fParam);

	//find the peaks in the signals of all channels//
    fSiganalyzer.findPeaksIn(remievent);

	//extract the peaks for the layers//
	//and sort the peaks for detektor hits//
	//fill the results in the Cass Event//
	//this has to be done for each detektor individually//
    fSorter.sort(remievent);
}



// Local Variables:
// coding: utf-8
// mode: C++
// c-file-offsets: ((c . 0) (innamespace . 0))
// c-file-style: "Stroustrup"
// fill-column: 100
// End:
