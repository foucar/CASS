#include "peak.h"

//______________________________________________________________________________________________________________________
cass::REMI::Peak::Peak()
{
	Clear();
}

//______________________________________________________________________________________________________________________
void cass::REMI::Peak::Clear()
{
	fTime = 0;
	fCfd  = 0;
	fCom  = 0;

	fPolarity = 0;
	fSlope    = 0;

	fMaxpos         = 0;
	fMaximum        = 0;
	fHeight         = 0;
	fHeightAbziehen = 0;

	fFwhm         = 0;
	fWidth        = 0;
	fPosHalfLeft  = 0;
	fPosHalfRight = 0;

	fIntegral = 0;

	fStartpos = 0;
	fStoppos  = 0;

	fUsed = false;
}
