#include <iostream>

#include "Detector.h"
#include "Channel.h"
#include "REMIAnalysis.h"



//______________________________________________________________________________________________________________________
void cass::REMI::Signal::extractFromChannels(const std::vector<cass::REMI::Channel>& chans)
{
	//--fill the vector with peaks--//
	for (size_t i=0; i < chans[fChNbr].nbrPeaks();++i)
	{
		const Peak &p = chans[fChNbr].peak(i);
		if (p.time()	>= fTrLow)				//if time value is bigger than from
		if (p.time()	<  fTrHigh)				//if time value is lower than to
		if (p.polarity()== fPolarity)			//and peak has right polarity
			fPeaks.push_back(p);
	}
}

//______________________________________________________________________________________________________________________
void cass::REMI::AnodeLayer::extractFromChannels(const std::vector<Channel>& chans)
{
	fOne.extractFromChannels(chans);
	fTwo.extractFromChannels(chans);
}

//______________________________________________________________________________________________________________________
cass::REMI::Detector::Detector(const DetectorParameter& p)
{
	fName			= p.fName;
	fHex			= p.fIsHex;
	fSortMethod		= p.fSortMethod;
	fDeadAnode		= p.fDeadAnode;
	fDeadMcp		= p.fDeadMcp;
	fMcpRadius		= p.fMcpRadius;
	fWLayerOffset	= p.fWLayerOffset;
	fRuntime		= p.fRuntime;
	fULayer			= p.fULayer;
	fVLayer			= p.fVLayer;
	fWLayer			= p.fWLayer;
	fMcp			= p.fMcp;
}

//________________________________________________________________________________________________________________________
cass::REMI::DetectorHit& cass::REMI::Detector::addHit(double x, double y, double t)
{
	//add a hit to this detector
	fHits.push_back(cass::REMI::DetectorHit(x,y,t));
	return fHits.back();
}


//______________________________________________________________________________________________________________________
void cass::REMI::Detector::extractFromChannels(const std::vector<Channel>& chans)
{
	fMcp.extractFromChannels(chans);
	fULayer.extractFromChannels(chans);
	fVLayer.extractFromChannels(chans);
	if (fHex)
		fWLayer.extractFromChannels(chans);
}