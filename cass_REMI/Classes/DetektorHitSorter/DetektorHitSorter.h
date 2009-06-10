#ifndef __DetektorHitSorter_H_
#define __DetektorHitSorter_H_

#include <vector>


namespace cass
{
	class Event;
	namespace REMI
	{
		class Parameter;
		class DetectorParameter;
		class Detector;
		class RemiAnalysisEvent;
//_______________________________________________________________________MyDetektorHitSorter________________________________________________________________________________________
class DetektorHitSorterBase
{
public:
	DetektorHitSorterBase(const DetectorParameter&)				{}
	virtual ~DetektorHitSorterBase()	{}

public:
	virtual void sort(RemiAnalysisEvent&, Detector&, cass::Event&)=0;

protected:
	enum EHistoIndex
	{
		kSumU=0,
		kSumV,
		kSumW,
		kSumVsURaw,
		kSumVsVRaw,
		kSumVsWRaw,
		kSumVsUShift,
		kSumVsVShift,
		kSumVsWShift,
		kSumVsUShiftCorr,
		kSumVsVShiftCorr,
		kSumVsWShiftCorr,
		kDetRaw_ns,
		kDetShi_ns,
		kDetUVRaw_ns,
		kDetUVShi_ns,
		kDetUWRaw_ns,
		kDetUWShi_ns,
		kDetVWRaw_ns,
		kDetVWShi_ns,
		kDetRaw_mm,
		kDetShi_mm,
		kDetUVRaw_mm,
		kDetUVShi_mm,
		kDetUWRaw_mm,
		kDetUWShi_mm,
		kDetVWRaw_mm,
		kDetVWShi_mm,
		kMcpDeadTime,
		kU1DeadTime,
		kU2DeadTime,
		kV1DeadTime,
		kV2DeadTime,
		kW1DeadTime,
		kW2DeadTime,
		kNbrRecHits,
		kNbrMCPHits,
		kNbrU1Hits,
		kNbrU2Hits,
		kNbrV1Hits,
		kNbrV2Hits,
		kNbrW1Hits,
		kNbrW2Hits,
		kURatio,
		kVRatio,
		kWRatio,
		kU1MCPRatio,
		kU2MCPRatio,
		kV1MCPRatio,
		kV2MCPRatio,
		kW1MCPRatio,
		kW2MCPRatio,
		kMCPRecRatio,
		kU1RecRatio,
		kU2RecRatio,
		kV1RecRatio,
		kV2RecRatio,
		kW1RecRatio,
		kW2RecRatio,
		kUsedMethod,
		kTime,
		kPosXVsTime,
		kPosYVsTime,
		kDetAll,
		kDetRisky,
		kDetNonRisky,
		kNonLinearityMap
	};
};

//the actual worker
typedef std::vector<DetektorHitSorterBase*> dethitsorters_t;
class DetektorHitSorter
{
public:
	DetektorHitSorter()		{}
	~DetektorHitSorter();

public:
	enum ESorterMethod {kSimple=0, kAchim, kDoNothing};
	void init(const Parameter&);
	void sort(RemiAnalysisEvent&, Event&);

private:
	dethitsorters_t		fDhs;	//vector containing pointers to the sorters for each detector
};


}//end namespace remi
}//end namespace cass
#endif