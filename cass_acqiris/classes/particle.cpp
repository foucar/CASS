#include "MyParticle.h"
#include "MyParticleInfo.h"

#include "../../lma2root/MyRootManager/MyHistos.h"
#include "../MyMomentaCalculator/MyMomentaCalculator.h"

//___________________________________________________________________________________________________________________________________________________________
void MyParticle::ReadFromInfo(const MyParticleInfo &pi)
{
	fCondTofFr	= pi.GetCondTofFr();
	fCondTofTo	= pi.GetCondTofTo();	
	fCondRad	= pi.GetPosFlag() ? TMath::Max(pi.GetCondWidthX(),pi.GetCondWidthY()) : pi.GetCondRad();
	fCondRadX	= pi.GetCondRadX();
	fCondRadY	= pi.GetCondRadY();
	fCondWidthX	= pi.GetCondWidthX();
	fCondWidthY	= pi.GetCondWidthY();
	fPosFlag	= pi.GetPosFlag();
	fAngle		= pi.GetAngle()*TMath::DegToRad();
	fXcor		= pi.GetXcor();
	fYcor		= pi.GetYcor();
	fSfx		= pi.GetSfx();
	fSfy		= pi.GetSfy();
	fT0			= pi.GetT0();
	fMass_au	= pi.GetMass_amu()*MyUnitsConv::amu2au();
	fCharge_au	= pi.GetCharge_au();
	fName		= pi.GetName();
	fSp			= pi.GetSpectrometer();
}

//___________________________________________________________________________________________________________________________________________________________
const MyParticleHit &MyParticle::AddHit(const MyDetektorHit &dh)
{
	//add a hit to this particle
	fPh.push_back(MyParticleHit(dh,*this));
	return fPh.back();
}
