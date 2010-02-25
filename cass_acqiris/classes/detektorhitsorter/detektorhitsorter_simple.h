#ifndef __DetektorHitSorterSimple_H_
#define __DetektorHitSorterSimple_H_

#include "detektorhitsorter_quad.h"

namespace cass
{
  namespace REMI
  {
    //______________________MyDetektorHitSorter Simple Version______________________
    class DetectorHitSorterSimple : public DetectorHitSorterQuad
    {
    public:
      DetectorHitSorterSimple()    {std::cout << "adding simple detectorhitsorter"<<std::endl;}
      void sort(REMIEvent&, Detector&);
    public:
      enum LayersToUse {UV=0,UW,VW};

    private:
      void sortForTimesum(Detector&,AnodeLayer &x,AnodeLayer &y);
    };

  }//end namespace remi
}//end namespace cass

#endif
