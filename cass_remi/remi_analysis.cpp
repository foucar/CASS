#include "remi_analysis.h"
#include "remi_event.h"
#include "cassevent.h"

void cass::REMI::Analysis::init(const cass::ParameterBackend* param)
{
    //copy the parameters//
    fParam = *dynamic_cast<const cass::REMI::Parameter*>(param);
    //intitalize the signal analyzer//
    fSiganalyzer.init(fParam.fPeakfindingMethod);
    //initialize the Detectorhit sorter for each detector//
    fSorter.init(fParam);
}

void cass::REMI::Analysis::operator()(cass::CASSEvent* cassevent)
{
    //get the remievent from the cassevent//
    cass::REMI::REMIEvent& remievent = cassevent->REMIEvent();

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
