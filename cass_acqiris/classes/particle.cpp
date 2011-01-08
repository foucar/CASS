//Copyright (C) 2009-2010 Lutz Foucar

/**
 * @file particle.cpp file contains the classes that describe a particle that hit
 *                    a delayline detector.
 *
 * @author Lutz Foucar
 */

#include <cmath>

#include "particle.h"
#include "cass_settings.h"

using namespace cass::ACQIRIS;

namespace cass
{
  namespace ACQIRIS
  {
    class IsParticleHit;
    class IsParticleHit
    {
    public:
      virtual bool operator()(const DelaylineDetector::hit_t &dethit)const=0;
    };

    class TofCond : public IsParticleHit
    {
    public:
      bool operator()(const DelaylineDetector::hit_t &dethit) const
      {
        return  (_tofcond.first < dethit["time"] && _tofcond.second < dethit["time"]);
      }
    private:
      std::pair<double,double> _tofcond;
    };

//    class RadCond : public IsParticleHit
//    {
//    };
//
//    class QuadCond : public IsParticleHit
//    {
//    };
//
//    class TofRadCond : public IsParticleHit
//    {
//    };
//
//    class TofQuadCond : public IsParticleHit
//    {
//    };

    void kartesian2polar(Particle::particleHit_t& hit)
    {
      const double & x (hit["px"]);
      const double & y (hit["py"]);
      const double & z (hit["pz"]);
      double & rho (hit["roh"]);
      double & theta (hit["theta"]);
      double & phi (hit["phi"]);
      rho = sqrt(x*x + y*y + z*z);
      theta = atan2(y,x);
      phi = acos(z/rho);
    }
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

Particle::particleHits_t& Particle::hits()
{
  if (!_listIsCreated)
  {
    using namespace std;
    const IsParticleHit &isParticleHit (*_isParticleHit);
    const MomentumCalculator &calc_px (*_calc_px);
    const MomentumCalculator &calc_py (*_calc_py);
    const MomentumCalculator &calc_pz (*_calc_pz);
    _listIsCreated = true;
    DelaylineDetector::hits_t::iterator dethit (_detectorhits->begin());
    for (; dethit != _detectorhits->end(); ++dethit)
    {
      if (isParticleHit(*dethit))
      {
        particleHit_t hit;
//        hit["px"] = calc_px(*dethit);
//        hit["py"] = calc_py(*dethit);
//        hit["pz"] = calc_pz(*dethit);
        kartesian2polar(hit);
        _particlehits.push_back(hit);
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
