#include "REMIAnalysis.h"
#include "./Classes/Event/Event.h"
#include "Event.h"

void cass::REMI::Analysis::init(const cass::REMI::Parameter& param)
{
	//copy the parameters//
	fParam = param;
	//intitalize the signal analyzer//
	fSiganalyzer.init(param.fAnaMethod);
	//initialize the Detectorhit sorter for each detector//
	fSorter.init(param);
}

void cass::REMI::Analysis::operator()(const cass::REMI::RawData& data, cass::Event &cassEvent)
{
	//create the datastructure from the RawData and the parameters//
	cass::REMI::Event e(data.config,data.data,fParam);

	//find the peaks in the signals of all channels//
	fSiganalyzer.FindPeaksIn(e);
	
	//extract the peaks for the layers//
	//and sort the peaks for detektor hits//
	//fill the results in the Cass Event//
	//this has to be done for each detektor individually//
	fSorter.sort(e,cassEvent);
}
