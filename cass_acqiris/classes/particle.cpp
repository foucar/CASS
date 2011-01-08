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
    /** base class of conditions for finding the right particle
     *
     * @author Lutz Foucar
     */
    class IsParticleHit
    {
    public:
      /** typedef defining the types of available conditions */
      enum ConditionType {tofcond, radcond, rectcond, tofradcond,tofrectcond};

      /** the comparison
       *
       * @return true when dethit fullfilles the condition
       * @param dethit the detector hit to check for the condition
       */
      virtual bool operator()(const DelaylineDetector::hit_t &dethit)const=0;

      /** read the parameters of the condition from the .ini file
       *
       * @param s the CASSSettings object to read the information from
       */
      virtual void loadSettings(CASSSettings &s)=0;

      /** create an instance of the chosen class
       *
       * @return pointer to instance of requested class
       * @param type the requested class type
       */
      static IsParticleHit* instance(const ConditionType& type);
    };

    /** a Time of Flight condition
     *
     * checks whether the detectorhit is in a predifined range in the time of
     * flight
     *
     * @author Lutz Foucar
     */
    class TofCond : public IsParticleHit
    {
    public:
      bool operator()(const DelaylineDetector::hit_t &dethit) const
      {
        return  (_tofcond.first < dethit["time"] && dethit["time"] < _tofcond.second);
      }

      void loadSettings(CASSSettings &s)
      {
        using namespace std;
        s.beginGroup("ToFCondition");
        _tofcond = make_pair(s.value("Low",0).toDouble(),
                             s.value("High",20000).toDouble());
        s.endGroup();
      }

    private:
      /** the tof range */
      std::pair<double,double> _tofcond;
    };

    /** a radius position condition
     *
     * checks whether the postion of the detectorhit on the detector is in a
     * given radius around a predefined center.
     *
     * @author Lutz Foucar
     */
    class RadCond : public IsParticleHit
    {
    public:
      bool operator()(const DelaylineDetector::hit_t &dethit) const
      {
        const double &x (dethit["x_mm"]);
        const double &y (dethit["y_mm"]);
        const double rad = sqrt(x*x + y*y);
        return (rad < _maxradius);
      }

      void loadSettings(CASSSettings &s)
      {
        using namespace std;
        s.beginGroup("RadiusCondition");
        _center = make_pair(s.value("CenterX",0).toDouble(),
                            s.value("CenterY",0).toDouble());
        _maxradius = s.value("MaximumRadius",100).toDouble();
        s.endGroup();
      }

    private:
      /** the center of the radius */
      std::pair<double,double> _center;

      /** the maximum radius of the condtion */
      double _maxradius;
    };

    /** a simple position condition
     *
     * checks whether the detector hit falls in a simple rectangular condition
     *
     * @author Lutz Foucar
     */
    class RectCond : public IsParticleHit
    {
    public:
      bool operator()(const DelaylineDetector::hit_t &dethit) const
      {
        const double &x (dethit["x_mm"]);
        const double &y (dethit["y_mm"]);
        const bool checkX(_xrange.first < x && x < _xrange.second);
        const bool checkY(_yrange.first < y && y < _yrange.second);
        return (checkX && checkY);
      }

      void loadSettings(CASSSettings &s)
      {
        using namespace std;
        s.beginGroup("SimplePositionCondition");
        _xrange = make_pair(s.value("XLow",-10).toDouble(),
                            s.value("XHigh",10).toDouble());
        _yrange = make_pair(s.value("YLow",-10).toDouble(),
                            s.value("YHigh",10).toDouble());
        s.endGroup();
      }

    private:
      /** the range in x */
      std::pair<double,double> _xrange;

      /** the range in y */
      std::pair<double,double> _yrange;
    };

    /** a combination of conditions
     *
     * this class combines two of the IsParticleHit conditions
     *
     * @tparam FistCondition class that defines the first condition
     * @tparam SecondCondition class that defines the second condition
     *
     * @author Lutz Foucar
     */
    template <class FirstCondition, class SecondCondition>
    class CombineConditions : public IsParticleHit
    {
    public:
      CombineConditions()
        :_conditions(std::make_pair(new FirstCondition, new SecondCondition))
      {}

      bool operator()(const DelaylineDetector::hit_t &dethit) const
      {
        IsParticleHit &firstCond (*_conditions.first);
        IsParticleHit &secondCond (*_conditions.second);
        return (firstCond(dethit) && secondCond(dethit));
      }

      void loadSettings(CASSSettings &s)
      {
        _conditions.first->loadSettings(s);
        _conditions.second->loadSettings(s);
      }

    private:
      std::pair<IsParticleHit*,IsParticleHit*> _conditions;
    };

    IsParticleHit* IsParticleHit::instance(const ConditionType &type)
    {
      IsParticleHit *cond(0);
      switch(type)
      {
      case tofcond:
        cond = new TofCond;
        break;
      case radcond:
        cond = new RadCond;
        break;
      case rectcond:
        cond = new RectCond;
        break;
      case tofrectcond:
        cond = new CombineConditions<TofCond,RectCond>();
        break;
      case tofradcond:
        cond = new CombineConditions<TofCond,RadCond>();
        break;
      default:
        throw std::invalid_argument("IsParticleHit::instance: No such condition type available");
      }
      return cond;
    }

    /** convert kartesian coordinates to polar coordinates
     *
     * will use the kartesian coordinates of the momentum vector of the particle
     * hit and add its polarcoordinates to the hit.
     *
     * @param hit the hit to make the transition with
     *
     * @author Lutz Foucar
     */
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
  delete _isParticleHit;
  IsParticleHit::ConditionType condtype
      (static_cast<IsParticleHit::ConditionType>(s.value("ConditionType",IsParticleHit::tofcond).toInt()));
  _isParticleHit = IsParticleHit::instance(condtype);
  _isParticleHit->loadSettings(s);

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
