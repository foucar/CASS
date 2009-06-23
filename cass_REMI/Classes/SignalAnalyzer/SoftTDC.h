#ifndef __SoftTDC_h__
#define __SoftTDC_h__

#include <vector>
#include "REMIEvent.h"

namespace cass
{
	namespace REMI
	{
		//this class is placeholder for two other classes wich will be called 
		//according to how many bits the instrument has
		class SoftTDC
		{
		public:
            virtual void FindPeaksIn(REMIEvent&)=0;
		};

		//this class does nothing 
		class SoftTDCDoNothing : public SoftTDC
		{
		public:
            void FindPeaksIn(REMIEvent&){}
		};
	}//end namespace remi
}//end namespace cass
#endif
