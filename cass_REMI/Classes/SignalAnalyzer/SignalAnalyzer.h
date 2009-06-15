#ifndef __SignalAnalyzer_h__
#define __SignalAnalyzer_h__

#include "SoftTDC.h"

namespace cass
{
	namespace REMI
	{
		class SignalAnalyzer
		{
		public:
			SignalAnalyzer():fStdc(0),fMethod(-1)	{}
			~SignalAnalyzer();

		public:
			enum ESignalAnalyzeMethods{kCoM=0, kCfd, kDoNothing};
			void init(int analyzemethod);
			void FindPeaksIn(cass::REMI::RemiAnalysisEvent &e)const
			{
				fStdc->FindPeaksIn(e);
			}

		private:
			SoftTDC		*fStdc;
			int			 fMethod;
		};
	}//end namespace remi
}//end namespace cass
#endif
