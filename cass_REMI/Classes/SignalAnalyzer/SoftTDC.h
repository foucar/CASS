#ifndef __SoftTDC_h__
#define __SoftTDC_h__

#include <vector>


namespace cass
{
	namespace REMI
	{
		class RemiAnalysisEvent;
		//this class is placeholder for two other classes wich will be called 
		//according to how many bits the instrument has
		class SoftTDC
		{
		public:
			virtual void FindPeaksIn(cass::REMI::RemiAnalysisEvent&)=0;
		};

		//this class does nothing 
		class SoftTDCDoNothing : public SoftTDC
		{
		public:
			void FindPeaksIn(cass::REMI::RemiAnalysisEvent&){}
		};
	}//end namespace remi
}//end namespace cass
#endif