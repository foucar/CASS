//Copyright (C) 2009-2010 Lutz Foucar

/**
 * @file particle.cpp file contains the classes that describe a particle that hit
 *                    a delayline detector.
 *
 * @author Lutz Foucar
 */

#include <cmath>

#include "particle.h"
#include "momenta_calculator.h"
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
      virtual bool operator()(const detectorHit_t &dethit)const=0;

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
     * @cassttng AcqirisDetectors/\%detectorname\%/\%particlename%/ToFCondition/{Low|High}\n
     *           The range to check whether the detectorhit is in.
     *           Default is 0|20000.
     *
     * @author Lutz Foucar
     */
    class TofCond : public IsParticleHit
    {
    public:
      bool operator()(const detectorHit_t &dethit) const
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
     * @cassttng AcqirisDetectors/\%detectorname\%/\%particlename%/RadiusCondition/{CenterX|CenterY}\n
     *           The position of the center of the radius to check for in mm
     *           Default is 0|0.
     * @cassttng AcqirisDetectors/\%detectorname\%/\%particlename%/RadiusCondition/{MaximumRadius}\n
     *           The maximum radius the position is checked for in mm. Default
     *           is 100.
     *
     * @author Lutz Foucar
     */
    class RadCond : public IsParticleHit
    {
    public:
      bool operator()(const detectorHit_t &dethit) const
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
     * @cassttng AcqirisDetectors/\%detectorname\%/\%particlename%/SimplePositionCondition/{XLow|XHigh}\n
     *           The range in the x-axis to check in mm. Default is -10|10.
     * @cassttng AcqirisDetectors/\%detectorname\%/\%particlename%/SimplePositionCondition/{YLow|YHigh}\n
     *           The range in the y-axis to check in mm. Default is -10|10.
     *
     * @author Lutz Foucar
     */
    class RectCond : public IsParticleHit
    {
    public:
      bool operator()(const detectorHit_t &dethit) const
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
     * @cassttng see TofCond, RadCond and RectCond for possible settings
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

      bool operator()(const detectorHit_t &dethit) const
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
    void kartesian2polar(particleHit_t& hit)
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

Particle::~Particle()
{
  delete _isParticleHit;
  delete _calc_detplane;
  delete _calc_tof;
}

void Particle::loadSettings(CASSSettings& s)
{
  _spectrometer.loadSettings(s);
  _copyandcorrect.loadSettings(s);
  _charge_au = s.value("Charge",1).toDouble();
  _mass_au = s.value("Mass",1).toDouble();
  if (!(_mass_au == 1 && _charge_au == -1))
    _mass_au *= 1836.15;
  delete _isParticleHit;
  IsParticleHit::ConditionType condtype
      (static_cast<IsParticleHit::ConditionType>(s.value("ConditionType",IsParticleHit::tofcond).toInt()));
  _isParticleHit = IsParticleHit::instance(condtype);
  _isParticleHit->loadSettings(s);
  delete _calc_detplane;
  if (_spectrometer.BFieldIsOn())
    _calc_detplane = MomentumCalculator::instance(MomentumCalculator::PxPyWBField);
  else
    _calc_detplane = MomentumCalculator::instance(MomentumCalculator::PxPyWOBField);
  delete _calc_tof;
  if (_spectrometer.regions().size() > 1)
    _calc_tof = MomentumCalculator::instance(MomentumCalculator::PzMultipleRegions);
  else
    _calc_tof = MomentumCalculator::instance(MomentumCalculator::PzOneRegion);
}

particleHits_t& Particle::hits()
{
  if (!_listIsCreated)
  {
    using namespace std;
    const IsParticleHit &isParticleHit (*_isParticleHit);
    const MomentumCalculator &calcpxpy (*_calc_detplane);
    const MomentumCalculator &calcpz (*_calc_tof);
    _listIsCreated = true;
    detectorHits_t::iterator dethit (_detectorhits->begin());
    for (; dethit != _detectorhits->end(); ++dethit)
    {
      if (isParticleHit(*dethit))
      {
        particleHit_t hit(_copyandcorrect(*dethit));
        calcpxpy(*this,hit);
        calcpz(*this,hit);
        kartesian2polar(hit);
        _particlehits.push_back(hit);
      }
    }
  }
  return _particlehits;
}

void Particle::associate(detectorHits_t *dethits)
{
  _listIsCreated = false;
  _particlehits.clear();
  _detectorhits = dethits;
}
