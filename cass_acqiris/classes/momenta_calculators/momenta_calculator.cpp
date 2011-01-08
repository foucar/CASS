//Copyright (C) 2001-2010 Lutz Foucar

/**
 * @file momenta_calculator.cpp file contains the classes that calculate the
 *                              momenta of particles from their detector hits.
 *
 * @author Lutz Foucar
 */

#include <cmath>
#include <iostream>
#include <stdexcept>

#include "momenta_calculator.h"
#include "spectrometer.h"
#include "particle.h"
#include "cass_settings.h"


//-----------------Constants--------------------------
//#define amu_au 1836.15											//Convert Atomic Mass Unit to a.u.
//#define Vcm_au	1.9446905050536652707924548855431e-10			//1 a.u. = 5.14220642e9 V/cm
//#define ns_au 4.1341373336561364143973749949088e+7				//1 a.u. = 2.418884326505e-8 ns
//#define mm_au 1.8897261249935897655251029944769e+7				//1 a.u. = 0.5291772108e-7 mm
//#define mmns_au 0,45710289051340228274244496244648				//convert mm/ns in a.u.
//#define mmnsns 0.17588201489603770767263287993687e-1				//convert V/cm * C[a.u.]/M[a.u] to mm/ns^2



namespace cass
{
  namespace ACQIRIS
  {
    namespace UnitConvertion
    {
      //@{
      /** Atomic Units -> SI Units */
      inline double au2m()      {return 5.29177210818E-11;}
      inline double au2mm()     {return au2m()*1E3;}
      inline double au2s()      {return 2.41888432650516E-17;}
      inline double au2ns()     {return au2s()*1E9;}
      inline double au2mPs()    {return 2.187691263373E6;}
      inline double au2mmPns()  {return au2mPs()*1E-6;}
      inline double au2kg()     {return 9.10938215E-31;}
      //@}
      //@{
      /** SI Units -> Atomic Units */
      inline double mm2au()     {return 1./au2mm();}
      inline double ns2au()     {return 1./au2ns();}
      inline double mmPns2au()  {return 1./au2mmPns();}
      inline double kg2au()     {return 1./au2kg();}
      //@}
      /** convert V/cm * C[a.u.]/M[a.u] to mm/ns^2 */
      inline double VPcm2mmPns()  {return 0.17588201489603770767263287993687e-1;}
      /** Atomic mass unit -> SI Unit */
      inline double amu2kg()    {return 1.66053878283E-27;}
      /** Atomic mass unit -> Atomic Units */
      inline double amu2au()    {return (amu2kg()*kg2au());}
      /** Atomic Units ->Atomic mass unit */
      inline double au2amu()    {return 1./amu2au();}
    }
    inline double Pi()   {return 3.1415;}

    /** calculate Momentum in Detektor Plane
     *
     * calculates the momentum of the particle in the detector plane. This is
     * for the case that a magnetic field is present in the spectrometer.
     * Usually this is there because one wants to collect all electrons in the
     * spectrometer.
     *
     * First calculate the total momentum in the detector plane. Then we have to
     * find out where the initial direction of emmision this means we need to
     * find the angle between the x-axis and the emission angle phi.The angle
     * depends whether the cyclotron motion is ccw or cw, meaning the B-Field is
     * pointing into the detektor-plane or out of the detector plane respectivly.
     *
     * @param[in] x_mm the x position of the hit in mm
     * @param[in] y_mm the y position of the hit in mm
     * @param[in] tof_ns the time of flight of the hit in ns
     * @param[in] particle the particle properties
     * @param[out] px_au the x momentum in atomic units
     * @param[out] py_au the y momentum in atomic units
     *
     * @author Lutz Foucar
     */
    void getDetPlaneMomenta(double x_mm, double y_mm, double tof_ns, const Particle& particle,
                            double &px_au, double &py_au)
    {
      const double tgyr_ns (particle.spectrometer().cyclotronPeriod_ns());
      const double mass_au (particle.mass_au());
      const bool rotationClockwise (particle.spectrometer().rotationClockWise());
      //--convert everything to a.u.--//
      const double tgyr_au (tgyr_ns * UnitConvertion::ns2au());
      const double x_au (x_mm * UnitConvertion::mm2au());
      const double y_au (y_mm * UnitConvertion::mm2au());
      //--calculate the total momentum in Detektor Plane--//
      const double wt_total ((2.*Pi()*tof_ns) / (tgyr_ns));
      //if we already have more than 1 turn, take wt modulo 2*Pi
      const double wt (fmod(wt_total,2.*Pi()));
      const double p_total (( sqrt(x_au*x_au + y_au*y_au) * mass_au * Pi() ) / ( sin(0.5*wt) * tgyr_au ));
      // theta is the angle between x-axis and position on the detektor. The
      // function atan2 will give values between [-PI..PI] but we need [0..2*PI]
      double theta = ( atan2(y_mm,x_mm)<0 ) ? atan2(y_mm,x_mm)+ 2.*Pi() : atan2(y_mm,x_mm);
      //--if the cyc motion is ccw phi is theta - wt/2 if it cw its theta + wt/2--//
      double phi   = (rotationClockwise)? theta + 0.5*wt : theta - 0.5*wt;
      //--calculate the x and y momenta knowing the initial direction of the total momentum--//
      px_au = p_total * cos(phi);
      py_au = p_total * sin(phi);
    }

    /** calculate the momentum in the detector plane
     *
     * calculate the momentum in the detector plane in case when there is no
     * magnetic field present. This is simply done by calulating the velocity
     * of the particle perpendicular to the time of flight axis when it hits
     * the detector. This is determined by the time if flew and the position it
     * hit the detector.
     *
     * @return the momentum of the particle along the axis
     * @param axis_mm the position of the hit on the axis in mm
     * @param tof_ns the time of flight of the hit in ns
     * @param mass_au the mass of the particle in atomic units
     *
     * @author Lutz Foucar
     */
    double getDetPlaneMomentum(double axis_mm, double tof_ns, double mass_au)
    {
      return ((axis_mm * mass_au) / tof_ns ) * UnitConvertion::mmPns2au();
    }

    /** Momentum along time of flight
     *
     * this will use a direct direct calculation since there is only one
     * spectrometer region.
     *
     * calculates the acceleration from the E-field in the region, charge and
     * mass of the particle. The result will be in \f$\frac{mm^2}{ns}\f$. With
     * this one can calculate the velocity of the particle after it has reached
     * the detector in \f$\frac{mm}{ns}\f$. After converting \f$\frac{mm}{ns}\f$
     * to atomic units one simply has to multiply the velocity with the mass in
     * atomic units to get momentum in atomic units.
     *
     * @return the momentum along the time of flight axis in atomic units
     * @param tof_ns the time of flight of the hit in ns
     * @param mass_au the mass of the particle in atomic units
     * @param charge_au the charge of the particle in atomic units
     * @param sr the spectrometer region through which the particle flys.
     *
     * @author Lutz Foucar
     */
    double getZMom(double tof_ns, double mass_au, double charge_au, const SpectrometerRegion &sr)
    {
      double a (sr.EField_Vpcm() * charge_au/mass_au * UnitConvertion::VPcm2mmPns());
      double v (sr.length_mm()/tof_ns - 0.5*a*tof_ns);
      double v_au (v * UnitConvertion::mmPns2au());
      double p_au (v_au * mass_au);
      return p_au;
    }

    /** helper function for endless SpectrometerRegions
     *
     * this helper will calculate the time of flight of a particle with a
     * given mass and charge in a spectrometer
     *
     * @return the time of flight in ns
     * @param v0 the initial velocity of the particle
     * @param mass_au the mass of the particle in atomic units
     * @param charge_au the charge of the particle in atomic units
     * @param spec the spectrometer through which the particle is flying
     *
     * @author Lutz Foucar
     */
    double evalFunc(double v0, double mass_au, double charge_au, const Spectrometer &spec)
    {
      double tges=0;
      double v = v0;
      const Spectrometer::regions_t &sr (spec.regions());
      for (size_t nReg=0; nReg<sr.size(); ++nReg)
      {
        //calculate the accereration from the E-field in the region, charge and mass of the particle//
        double a = sr[nReg].EField_Vpcm() * charge_au/mass_au * UnitConvertion::VPcm2mmPns();	//the acceleration in Region 1 in mm/ns^2
        double s = sr[nReg].length_mm();											//the length of this region in mm

        //calc how long one particle will fly with the given initial velocity//
        double tt=0;
        if (abs(a) > 1e-8)		//if there is an accerlartion
          tt = (-v + sqrt(v*v + 2.*a*s))/a;
        else					//if there is no accerlaration (eg drift)
          tt = s/v;

        //during its time in this Region the particle gained velocity if it wasn't a drift (a=0)
        //add this additional velocity to its initial velocity
        v += a*tt;

        //add the time that the particle stayed in this region to the total time
        tges += tt;
      }

      //return the total time
      return tges;
    }

    /** Momentum along time of flight
     *
     * find momentum iterativly. There is no analytical solution for when there
     * are more than two spectrometer regions through which the particle is
     * flying.
     *
     * We have measured the time of flight of the particle, what we need to find
     * out is to which initial momentum does this time of flight fit. Since we
     * have a function that will calculate the time of flight for a given intial
     * velocity, we can now use this function and vary the intial velocity so long
     * until the measured time of flight is the same as the calculated time of
     * flight for a chosen velocity.\n
     * Our initail guess for the velocity is the velocity the particle would
     * have, when there is only one spectrometer region. So first calculate the
     * velocity for when only the first region of the spectrometer would exist.
     * Then calculate the tof for this velocity and find the next step of the
     * iteration using Newtons Approximation. This means that we calculate the
     * slope of the function at the current position. The next iteration is where
     * the linear function with the slope crosses the right fx0. Do this until
     * the time of flight is close to the one we measured. Then calculate the
     * momentum of particle whos velocity gave the right time of flight.
     *
     * @return the momentum along the time of flight axis in atomic units only
     *         the tof_ns parameter is non negative. When it is negative return
     *         a high number (1e15)
     * @param tof_ns the time of flight of the hit in ns
     * @param mass_au the mass of the particle in atomic units
     * @param charge_au the charge of the particle in atomic units
     * @param spectrometer the spectrometer through which the particle is flying
     *
     * @author Lutz Foucar
     */
    double getZMomIter(double tof_ns, double mass_au, double charge_au, const Spectrometer &spectrometer)
    {
      //when a negative time was given return because there will be no good
      //solution
      if (tof_ns < 0)
        return 1e15;
      const double eField_Vpcm (spectrometer.regions()[0].EField_Vpcm());
      const double length_mm (spectrometer.regions()[0].length_mm());
      //calculate the acceleration from the E-field in the region, charge and
      //mass of the particle
      double a (eField_Vpcm * charge_au/mass_au * UnitConvertion::VPcm2mmPns());
      //begin with a v when there would be only one Region//
      double x0  (length_mm/tof_ns - 0.5*a*tof_ns);
      double fx0 (evalFunc(x0,mass_au,charge_au,spectrometer));
      //use Newtons Approximation//
      while(abs(fx0 - tof_ns) > 0.01)
      {
        //we need to find the slope of the function at point x0 therefore we need
        //a second point which is close to x0
        double x1 (1.1 * x0);
        double fx1 (evalFunc(x1,mass_au,charge_au,spectrometer));
        //calculate the slope//
        double m ((fx0-fx1)/(x0-x1));
        //the next starting point is the point where the slopeline crosses the
        //wanted fx0 value, put damping factor of 0.7 in addition
        x0  = x0 + 0.7*(tof_ns-fx0)/m;
        fx0 = evalFunc(x0,mass_au,charge_au,spectrometer);
      }
      double v_au (x0 * UnitConvertion::mmPns2au());
      double p_au (v_au * mass_au);
      return p_au;
    }




    ////###########################Momentum along Tof analytical version########
    ////________________________________________________________________________
    //double MyMomentaCalculator::getZMomentum(double tof_ns, double acc_mm, double drift_mm,
    //									       double charge_au, double mass_au,
    //										   double Efeld1_Vpcm, double Efeld2_Vpcm)
    //{
    //	/***********************************************************************
    //	  This function derives the momentum along the time of flight axis for
    //	  two regions. Those two regions can have 2 different accerlerations and
    //	  two differnt lengths. It does it analyticly using a function that
    //	  solves the qubic equation:
    //	  (-0.5*a2+0.5*a1)*t1^3 + (a2*t-0.5*a1*t)*t1^2 + (s1+s2-0.5*a2*t^2)*t1 + (-s1*t) = 0
    //	************************************************************************/
    //
    //	//--convert all values to a.u.--//
    //	double s1 = acc_mm * mm_au;
    //	double s2 = drift_mm * mm_au;
    //	double t = tof_ns * ns_au;
    //	double q = charge_au;
    //	double mass = mass_au;
    //	double Efeld1_au = Efeld1_Vpcm * Vcm_au;
    //	double Efeld2_au = Efeld2_Vpcm * Vcm_au;
    //
    //
    //	double a1 = Efeld1_au * q/mass;
    //	double a2 = Efeld2_au * q/mass;
    //
    //	double A = (a2*t - 0.5*a1*t) / (-0.5*a2 + 0.5*a1);
    //	double B = (s1 + s2 - 0.5*a2*t*t) / (-0.5*a2 + 0.5*a1);
    //	double C = (- s1*t) / (-0.5*a2 + 0.5*a1);
    //
    //
    //	double t1=0;										//the variable that we want to solve the function for
    //	double dummy1=0, dummy2=0;							//these are needed for the function to work
    //	int nbrL = solve_qubic(A,B,C,&t1,&dummy1,&dummy2);	//solve qubic equation
    //
    //	//--if something went wrong return 0--//
    //	if ((nbrL != 1) || dummy1 || dummy2 || (t1 < 0.))
    //		return 0.;
    //
    //	//--calculate the momentum from the found t1 value--//
    //	double v0 = (s1/t1) - 0.5*a1*t1;
    //	double mom = v0 * mass;
    //
    //
    //	return mom;
    //}
    //
    //#define SWAP(a,b) do { double tmp = b ; b = a ; a = tmp ; } while(0) //needed for solve_cubic()
    ////_________________________________________________________________________________
    //int MyMomentaCalculator::solve_qubic(double a, double b, double c, double * x0, double * x1, double * x2)
    //{
    //	/* Copyright (C) 1996, 1997, 1998, 1999, 2000 Brian Gough
    //	*
    //	* This program is free software; you can redistribute it and/or modify
    //	* it under the terms of the GNU General Public License as published by
    //	* the Free Software Foundation; either version 2 of the License, or (at
    //	* your option) any later version.
    //	*
    //	* This program is distributed in the hope that it will be useful, but
    //	* WITHOUT ANY WARRANTY; without even the implied warranty of
    //	* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    //	* General Public License for more details.
    //	*
    //	* You should have received a copy of the GNU General Public License
    //	* along with this program; if not, write to the Free Software
    //	* Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
    //	*/
    //
    //	/* solve_cubic.c - finds the real roots of x^3 + a x^2 + b x + c = 0 */
    //
    //
    //	double q = (a * a - 3 * b);
    //	double r = (2 * a * a * a - 9 * a * b + 27 * c);
    //
    //	double Q = q / 9;
    //	double R = r / 54;
    //
    //	double Q3 = Q * Q * Q;
    //	double R2 = R * R;
    //
    //	double CR2 = 729 * r * r;
    //	double CQ3 = 2916 * q * q * q;
    //
    //	if (R == 0 && Q == 0)
    //	{
    //		*x0 = - a / 3 ;
    //		*x1 = - a / 3 ;
    //		*x2 = - a / 3 ;
    //		return 3 ;
    //	}
    //	else if (CR2 == CQ3)
    //	{
    //		/* this test is actually R2 == Q3, written in a form suitable
    //			for exact computation with integers */
    //
    //		/* Due to finite precision some double roots may be missed, and
    //			considered to be a pair of complex roots z = x +/- epsilon i
    //			close to the real axis. */
    //
    //		double sqrtQ = sqrt (Q);
    //
    //		if (R > 0)
    //		{
    //			*x0 = -2 * sqrtQ  - a / 3;
    //			*x1 = sqrtQ - a / 3;
    //			*x2 = sqrtQ - a / 3;
    //		}
    //		else
    //		{
    //			*x0 = - sqrtQ  - a / 3;
    //			*x1 = - sqrtQ - a / 3;
    //			*x2 = 2 * sqrtQ - a / 3;
    //		}
    //		return 3 ;
    //	}
    //	else if (CR2 < CQ3) /* equivalent to R2 < Q3 */
    //	{
    //		double sqrtQ = sqrt (Q);
    //		double sqrtQ3 = sqrtQ * sqrtQ * sqrtQ;
    //		double theta = acos (R / sqrtQ3);
    //		double norm = -2 * sqrtQ;
    //		*x0 = norm * cos (theta / 3) - a / 3;
    //		//--orig but need changes due to the missing of M_PI--//
    ////      *x1 = norm * cos ((theta + 2.0 * M_PI) / 3) - a / 3;
    ////      *x2 = norm * cos ((theta - 2.0 * M_PI) / 3) - a / 3;
    //		*x1 = norm * cos ((theta + 2.0 * PI) / 3) - a / 3;
    //		*x2 = norm * cos ((theta - 2.0 * PI) / 3) - a / 3;
    //
    //		/* Sort *x0, *x1, *x2 into increasing order */
    //
    //		if (*x0 > *x1)
    //			SWAP(*x0, *x1) ;
    //
    //		if (*x1 > *x2)
    //		{
    //			SWAP(*x1, *x2) ;
    //
    //			if (*x0 > *x1)
    //				SWAP(*x0, *x1) ;
    //        }
    //
    //		return 3;
    //	}
    //	else
    //    {
    //		double sgnR = (R >= 0 ? 1 : -1);
    //		double A = -sgnR * pow (fabs (R) + sqrt (R2 - Q3), 1.0/3.0);
    //		double B = Q / A ;
    //		*x0 = A + B - a / 3;
    //		return 1;
    //    }
    //}
    ////########################################################################
  }
}





//______________________________________________________________________________
using namespace cass::ACQIRIS;

void HitCorrector::loadSettings(CASSSettings &s)
{
  using namespace std;
  s.beginGroup("Corrections");
  _t0 = s.value("T0",0).toDouble();
  _pos0 = make_pair(s.value("CorrectX",0).toDouble(),
                    s.value("CorrectY",0).toDouble());
  _scalefactors = make_pair(s.value("ScaleX",1).toDouble(),
                            s.value("ScaleY",1).toDouble());
  _angle = s.value("Angle",0).toDouble();
  _angle = _angle *Pi()/180.;
  s.endGroup();
}

particleHit_t HitCorrector::operator()(const detectorHit_t &dethit) const
{
  particleHit_t particlehit;
  particlehit["x_mm"] = dethit["x"];
  particlehit["y_mm"] = dethit["y"];
  particlehit["tof_ns"] = dethit["t"];
  particlehit["xCor_mm"] = particlehit["x_mm"] - _pos0.first;
  particlehit["yCor_mm"] = particlehit["y_mm"] - _pos0.second;
  particlehit["xCorScal_mm"] = particlehit["xCor_mm"] - _scalefactors.first;
  particlehit["yCorScal_mm"] = particlehit["yCor_mm"] - _scalefactors.second;
  particlehit["xCorScalRot_mm"] = (particlehit["xCorScal_mm"] * cos(_angle) - particlehit["yCorScal_mm"] * sin(_angle));
  particlehit["yCorScalRot_mm"] = (particlehit["xCorScal_mm"] * sin(_angle) + particlehit["yCorScal_mm"] * cos(_angle));
  particlehit["tofCor_ns"] = particlehit["tof_ns"] - _t0;
  return particlehit;
}

MomentumCalculator* MomentumCalculator::instance(const MomCalcType &type)
{
  MomentumCalculator *momcalc(0);
  switch (type)
  {
  case PxPyWBField:
    momcalc = new PxPyCalculatorWithBField;
    break;
  case PxPyWOBField:
    momcalc = new PxPyCalculatorWithoutBField;
    break;
  case PzOneRegion:
    momcalc = new PzCalculatorDirectOneRegion;
    break;
  case PzMultipleRegions:
    momcalc = new PzCalculatorMulitpleRegions;
    break;
  default:
    throw std::invalid_argument("MomentumCalculator::instance(): No such momentum calculator type available");
    break;
  }
  return momcalc;
}

particleHit_t&  PxPyCalculatorWithoutBField::operator()(const Particle &particle, particleHit_t& particlehit)const
{
  particlehit["px"] = getDetPlaneMomentum(particlehit["xCorRot_mm"],particlehit["tofCor_ns"],particle.mass_au());
  particlehit["py"] = getDetPlaneMomentum(particlehit["yCorRot_mm"],particlehit["tofCor_ns"],particle.mass_au());
  return particlehit;
}

particleHit_t&  PxPyCalculatorWithBField::operator()(const Particle &particle, particleHit_t& particlehit)const
{
  double &px_au (particlehit["px"]);
  double &py_au (particlehit["py"]);
  getDetPlaneMomenta(particlehit["xCorRot_mm"], particlehit["yCorRot_mm"], particlehit["tofCor_ns"], particle,
                     px_au, py_au);
  return particlehit;
}

particleHit_t&  PzCalculatorDirectOneRegion::operator()(const Particle &particle, particleHit_t& particlehit)const
{
  particlehit["pz"] = getZMom(particlehit["tofCor_ns"], particle.mass_au(), particle.charge_au(), particle.spectrometer().regions()[0]);
  return particlehit;
}

particleHit_t&  PzCalculatorMulitpleRegions::operator()(const Particle &particle, particleHit_t& particlehit)const
{
  particlehit["pz"] =  getZMomIter(particlehit["tofCor_ns"], particle.mass_au(), particle.charge_au(), particle.spectrometer());
  return particlehit;
}
