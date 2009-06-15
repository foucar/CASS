#ifndef __SoftTDCCoM_h__
#define __SoftTDCCoM_h__

#include "SoftTDC.h"
namespace cass
{
namespace REMI
{

//this is called in case it is a 8 Bit Instrument
class SoftTDCCoM8Bit : public SoftTDC
{
public:
	void FindPeaksIn(cass::REMI::RemiAnalysisEvent&);
};

//this is called in case it is a 10 Bit Instrument
class SoftTDCCoM16Bit : public SoftTDC
{
public:
	void FindPeaksIn(cass::REMI::RemiAnalysisEvent&);
};
}//end namespace remi
}//end namespace cass

#endif
