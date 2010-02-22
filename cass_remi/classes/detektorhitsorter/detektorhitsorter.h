#ifndef __DetektorHitSorter_H_
#define __DetektorHitSorter_H_

#include <vector>

namespace cass
{
  namespace REMI
  {
    class Detector;
    class AnodeLayer;
    class REMIEvent;
    class Channel;
    class Signal;

    //base class for detectorhit sorter
    class DetectorHitSorter
    {
    public:
      virtual ~DetectorHitSorter() {}
      virtual void sort(REMIEvent&, Detector&)=0;
    public:
      enum SorterTypes {kDoNothing=0, Simple, Achim};
    protected:
      void extractPeaksForSignal(std::vector<Channel>&, Signal&);
    };
  }//end namespace remi
}//end namespace cass
#endif
