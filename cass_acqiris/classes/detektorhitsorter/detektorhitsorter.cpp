#include "detektorhitsorter.h"

#include "detector.h"
#include "channel.h"


//______________________________________________the sorting_____________________________________________________________________________________________________________
void cass::REMI::DetectorHitSorter::extractPeaksForSignal(std::vector<cass::REMI::Channel>& chans, cass::REMI::Signal &signal)
{
  //--fill the vector with peaks--//
  for (size_t i=0; i < chans[signal.chanNbr()].peaks().size();++i)
  {
    Peak &p = chans[signal.chanNbr()].peaks()[i];
    if (p.time()    >= signal.trLow())      //if time value is bigger than from
    if (p.time()    <  signal.trHigh())     //if time value is lower than to
    if (p.polarity()== signal.polarity())   //and peak has right polarity
    {
//      std::cout << "found peak that fits the needs "<<p.time()<<" "<<signal.trLow()<<" "<<signal.trHigh()<<" "<<p.polarity()<<" "<<signal.polarity()<<std::endl;
      signal.peaks().push_back(&p);
    }
  }
}

