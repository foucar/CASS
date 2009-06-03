#include "REMIAnalysis.h"

namespace cass
{
namespace REMI
{

void Analysis::init(const cass::REMI::Parameter& param)
{
	//copy the parameters//
	_param = param;
	//intitalize the signal analyzer//
	siganalyzer.init(param._anaMethod);
	//initialize the Detectorhit sorter//
	sorter.init(param._sortMethod);
}

void Analysis::operator()(const RawData& data, cass::Event &cassEvent)
{
	//create the datastructure from the RawData and the parameters//
	Remi::Event e(RawData.data(),RawData.config(),_param);

	//find the peaks in the signals of all channels//
	siganalyzer.FindPeaksIn(e);
	
	//extract the peaks for the layers//
	//and sort the peaks for detektor hits//
	//fill the results in the Cass Event//
	sorter.Sort(e,cassEvent);
}


}//end namespace Remi
}//end namespace cass

