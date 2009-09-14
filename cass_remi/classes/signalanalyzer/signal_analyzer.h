#ifndef __SignalAnalyzer_h__
#define __SignalAnalyzer_h__

#include "softtdc.h"
//#include "remi_event.h"

namespace cass
{
    namespace REMI
    {
        class REMIEvent;
        class SignalAnalyzer
        {
        public:
            SignalAnalyzer():fStdc(0),fMethod(-1)	{}
            ~SignalAnalyzer();

        public:
            enum ESignalAnalyzeMethods{kCoM=0, kCfd, kDoNothing};
            void init(int analyzemethod);
            void findPeaksIn(REMIEvent &remievent)const
            {
                fStdc->FindPeaksIn(remievent);
            }

        private:
            SoftTDC *fStdc;
            int      fMethod;
        };
    }//end namespace remi
}//end namespace cass
#endif
