#ifndef __DetektorHitSorterSimple_H_
#define __DetektorHitSorterSimple_H_

#include "DetektorHitSorterQuad.h"

namespace cass
{
namespace REMI
{

//______________________MyDetektorHitSorter Simple Version______________________
class DetektorHitSorterSimple : public DetektorHitSorterQuad
{
public:
	DetektorHitSorterSimple(const cass::REMI::DetectorParameter&);

public:
	void sort(cass::REMI::RemiAnalysisEvent&, cass::REMI::Detector&, cass::Event&);

private:
	void sortForTimesum(cass::REMI::Detector&);
};

}//end namespace remi
}//end namespace cass

#endif