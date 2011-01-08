//Copyright (C) 2009-2010 Lutz Foucar

/**
 * @file particle.cpp file contains the classes that describe a particle that hit
 *                    a delayline detector.
 *
 * @author Lutz Foucar
 */


#include "particle.h"
#include "cass_settings.h"

using namespace cass::ACQIRIS;

namespace cass
{
  namespace ACQIRIS
  {
    struct IsParticleHit
    {
      virtual bool operator()(const DelaylineDetector::hit_t &dethit){return false;}
    };
  }
}

void Particle::loadSettings(CASSSettings& s)
{
//	fCondTofFr	= pi.GetCondTofFr();
//	fCondTofTo	= pi.GetCondTofTo();
//	fCondRad	= pi.GetPosFlag() ? TMath::Max(pi.GetCondWidthX(),pi.GetCondWidthY()) : pi.GetCondRad();
//	fCondRadX	= pi.GetCondRadX();
//	fCondRadY	= pi.GetCondRadY();
//	fCondWidthX	= pi.GetCondWidthX();
//	fCondWidthY	= pi.GetCondWidthY();
//	fPosFlag	= pi.GetPosFlag();
//	fAngle		= pi.GetAngle()*TMath::DegToRad();
//	fXcor		= pi.GetXcor();
//	fYcor		= pi.GetYcor();
//	fSfx		= pi.GetSfx();
//	fSfy		= pi.GetSfy();
//	fT0			= pi.GetT0();
//	fMass_au	= pi.GetMass_amu()*MyUnitsConv::amu2au();
//	fCharge_au	= pi.GetCharge_au();
//	fName		= pi.GetName();
//	fSp			= pi.GetSpectrometer();
}

Particle::particleHits_t Particle::hits()
{
  using namespace std;
  if (!_listIsCreated)
  {
    _listIsCreated = true;
    DelaylineDetector::hits_t::iterator dethit (_detectorhits->begin());
    while(dethit != _detectorhits->end())
    {
      dethit = find_if(dethit,_detectorhits->end(),*_isParticleHit);
      if (dethit != _detectorhits->end())
      {
        particleHit_t hit;
//        hit["px"] = *_calc_px(*dethit);
//        hit["py"] = *_calc_py(*dethit);
//        hit["pz"] = *_calc_pz(*dethit);
        _particlehits.push_back(hit);
        ++dethit;
      }
    }
  }
  return _particlehits;
}

void Particle::associate(DelaylineDetector::hits_t *dethits)
{
  _listIsCreated = false;
  _particlehits.clear();
  _detectorhits = dethits;
}
