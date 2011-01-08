#include <TMath.h>
#include <cmath>
#include <iostream>

#include "MyMomentaCalculator.h"
#include "../MyParticle/MySpectrometer/MySpectrometer.h"
#include "../MyParticle/MySpectrometer/MySpectrometerRegion.h"

//#define SWAP(a,b) do { double tmp = b ; b = a ; a = tmp ; } while(0) //needed for solve_cubic()

//-----------------Constants--------------------------
//#define amu_au 1836.15											//Convert Atomic Mass Unit to a.u.
//#define Vcm_au	1.9446905050536652707924548855431e-10			//1 a.u. = 5.14220642e9 V/cm
//#define ns_au 4.1341373336561364143973749949088e+7				//1 a.u. = 2.418884326505e-8 ns
//#define mm_au 1.8897261249935897655251029944769e+7				//1 a.u. = 0.5291772108e-7 mm
//#define mmns_au 0,45710289051340228274244496244648				//convert mm/ns in a.u.
//#define mmnsns 0.17588201489603770767263287993687e-1				//convert V/cm * C[a.u.]/M[a.u] to mm/ns^2



//###########################Momentum in Detektor Plane############################
//_____________________________________with magnetic field____________________________________________
void getXYMomenta(double x_mm, double y_mm, double tof_ns, double mass_au, double charge_au, const MySpectrometer& sp,
				  double &px_au, double &py_au)
{
	//--the given tgyr is for electrons--//
	double tgyr_ns = sp.CyclotronPeriod_ns();
	//--if we have recoils, calc the cyclotron frequence for the given m/q--//
	if (mass_au > 1.)
		tgyr_ns = tgyr_ns * mass_au / charge_au;

	//--convert everything to a.u.--//
	double tgyr_au = tgyr_ns * MyUnitsConv::ns2au();
	double x_au = x_mm * MyUnitsConv::mm2au();
	double y_au = y_mm * MyUnitsConv::mm2au();


	//--calculate the total momentum in Detektor Plane--//
	double wt_total = (2.*TMath::Pi()*tof_ns) / (tgyr_ns);
	double wt = std::fmod(wt_total,2.*TMath::Pi());	//if we already have more than 1 turn, take wt modulo 2*Pi
	double p_total = ( sqrt(x_au*x_au + y_au*y_au) * mass_au * TMath::Pi() ) / ( sin(0.5*wt) * tgyr_au );

	//--now we have to find out where the initial direction of emmision--//
	//--this means we need to find the angle between the x-axis and the--//
	//--emission angle phi.	The angle depends wether the cyclotron	   --//
	//--motion is ccw or cw, meaning the B-Field ist pointing into the --//
	//--detektor-plane or out of the detektor plane respectivly.	   --//

	//--theta is the angle between x-axis and position on the detektor--//
	//--the function atan2 will give values between [-PI..PI] but we need [0..2*PI]--//
	double theta = ( TMath::ATan2(y_mm,x_mm)<0 ) ? TMath::ATan2(y_mm,x_mm)+ 2.*TMath::Pi() : TMath::ATan2(y_mm,x_mm);
	
	//--if the cyc motion is ccw phi is theta - wt/2 if it cw its theta + wt/2--//
	double phi   = (sp.RotationClockWise())? theta + 0.5*wt : theta - 0.5*wt;

	//--calculate the x and y momenta knowing the initial direction of the total momentum--//
	px_au = p_total * TMath::Cos(phi);
	py_au = p_total * TMath::Sin(phi);
}

//_____________________________without magnetic field_________________________________________________
double getXMom(double x_mm, double tof_ns, double mass_au)
{
	return ((x_mm * mass_au) / tof_ns ) * MyUnitsConv::mmPns2au();
}

double getYMom(double y_mm, double tof_ns, double mass_au)
{
	return ((y_mm * mass_au) / tof_ns ) * MyUnitsConv::mmPns2au();
}
//#################################################################################






//#############################Momentum along tof####################################################
//_____________________________direct calculation_________________________________________________
double getZMom(double tof_ns, double mass_au, double charge_au, const SpecRegions &sr)
{
	//calculate the acceleration from the E-field in the region, charge and mass of the particle//
	double a = sr[0].EField_Vpcm() * charge_au/mass_au * MyUnitsConv::VPcm2mmPns();	//the acceleration in the Region in mm/ns^2
	double v = sr[0].Length_mm()/tof_ns - 0.5*a*tof_ns;							//the velocity in mm/ns
	
	double v_au = v * MyUnitsConv::mmPns2au();			//convert mm/ns to a.u.
	double p_au = v_au * mass_au;	//convert velocity to momentum

	return p_au;
}

//_____________________________helper function for endless SpectrometerRegions___________________________
double evalFunc(double v0, double mass_au, double charge_au, const SpecRegions &sr)
{
	double tges=0;
	double v = v0;
	for (size_t nReg=0; nReg<sr.size(); ++nReg)
	{
		//calculate the accereration from the E-field in the region, charge and mass of the particle//
		double a = sr[nReg].EField_Vpcm() * charge_au/mass_au * MyUnitsConv::VPcm2mmPns();	//the acceleration in Region 1 in mm/ns^2
		double s = sr[nReg].Length_mm();											//the length of this region in mm
		
		//calc how long one particle will fly with the given initial velocity//
		double tt=0;
		if (TMath::Abs(a) > 1e-8)		//if there is an accerlartion
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

//_____________________________find it iterativly_________________________________________________
double getZMomIter(double tof_ns, double mass_au, double charge_au, const SpecRegions &sr)
{
	//when a negative time was given return immediatly because there will be no good solution//
	if (tof_ns < 0) return 1e15;

	//calculate the acceleration from the E-field in the region, charge and mass of the particle//
	double a = sr[0].EField_Vpcm() * charge_au/mass_au * MyUnitsConv::VPcm2mmPns();	//the acceleration in Region 1 in mm/ns^2

	//--iterativly find the right v_mmns to get the right tof_ns--//
	//--we have a function t(v), but we want the inverse of the function v(t)--//
	//--since this cannot be done we have to find the right v iterativly--//
	//--so we have a function f(x) (t(v)) with the function value fx0 (t) at point x0 (v)--//
	//--we don't know which x0 (v) will give us the fx0 (t) that we want--//
	//--that means we have to iterate x0 until fx0 is the same that we want--//

	//begin with a v when there would be only one Region//
	double x0  = sr[0].Length_mm()/tof_ns - 0.5*a*tof_ns;
	double fx0 = evalFunc(x0,mass_au,charge_au,sr);

	//use Newtons Approximation//
	while(TMath::Abs(fx0 - tof_ns) > 0.01)
	{
		//we need to find the slope of the function at point x0 therefore we need//
		//a second point which is close to x0//
		double x1  = 1.1 * x0;
		double fx1 = evalFunc(x1,mass_au,charge_au,sr);

		//calculate the slope//
		double m = (fx0-fx1)/(x0-x1);

		//the next starting point is the point where the slopeline crosses the wanted fx0 value//
		x0  = x0 + 0.7*(tof_ns-fx0)/m;
		fx0 = evalFunc(x0,mass_au,charge_au,sr);
	}

	//now calc momentum from the velocity that was found
	double v_au = x0 * MyUnitsConv::mmPns2au();				//convert mm/ns -> a.u.
	double p_au = v_au * mass_au;		//convert velocity -> momentum

	return p_au;
}
//#################################################################################






















////###########################Momentum along Tof analytical version#####################################
////_________________________________________________________________________________
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
////#################################################################################




//_________________________________________________________________________________
double MyMomentaCalculator::px(double x_mm, double y_mm, double tof_ns, double mass_au, double charge_au, const MySpectrometer& sp)
{
	//--there are two possibilities, with Magnetic Field or without--//
	double px_au=0;
	double py_au=0;
	if (sp.MagneticFieldIsOn())	//we have a magnetic field (more complex)
		getXYMomenta(x_mm, y_mm, tof_ns, mass_au, charge_au, sp, px_au, py_au);
	else						//this is when we don't have a magnetic field
		px_au = getXMom(x_mm,tof_ns,mass_au);

	return px_au;
}

//_________________________________________________________________________________
double MyMomentaCalculator::py(double x_mm, double y_mm, double tof_ns, double mass_au, double charge_au, const MySpectrometer& sp)
{
	//--there are two possibilities, with Magnetic Field or without--//
	double px_au=0;
	double py_au=0;
	if (sp.MagneticFieldIsOn())	//we have a magnetic field (more complex)
		getXYMomenta(x_mm, y_mm, tof_ns, mass_au, charge_au, sp, px_au, py_au);
	else						//this is when we don't have a magnetic field
		py_au = getYMom(y_mm,tof_ns,mass_au);

	return py_au;
}

//_________________________________________________________________________________
double MyMomentaCalculator::pz(double tof_ns, double mass_au, double charge_au, const MySpectrometer& sp)
{
	if (sp.GetSpectrometerRegions().size() > 1)		//if there are many regions
		return getZMomIter(tof_ns, mass_au, charge_au, sp.GetSpectrometerRegions());
	else if (sp.GetSpectrometerRegions().size() == 1)	//if there is only one region
		return getZMom(tof_ns, mass_au, charge_au, sp.GetSpectrometerRegions());
	else
	{
		std::cout<< "No Spectrometer Region has been set. Please set it, otherwise one cannot calculate the momentum"<<std::endl;
		exit(1);
		return 0;
	}
}

