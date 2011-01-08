#ifndef __MyMomentaCalculator_H_
#define __MyMomentaCalculator_H_


namespace MyUnitsConv
{
	//Atomic Units -> SI Units
	inline const double au2m()			{return 5.29177210818E-11;}
	inline const double au2mm()			{return au2m()*1E3;}
	inline const double au2s()			{return 2.41888432650516E-17;}
	inline const double au2ns()			{return au2s()*1E9;}
	inline const double au2mPs()		{return 2.187691263373E6;}
	inline const double au2mmPns()		{return au2mPs()*1E-6;}
	inline const double au2kg()			{return 9.10938215E-31;}
	//SI Units -> Atomic Units
	inline const double mm2au()			{return 1/au2mm();}
	inline const double ns2au()			{return 1/au2ns();}
	inline const double mmPns2au()		{return 1/au2mmPns();}
	inline const double kg2au()			{return 1/au2kg();}
	//mixed units
	inline const double VPcm2mmPns()	{return 0.17588201489603770767263287993687e-1;}	//convert V/cm * C[a.u.]/M[a.u] to mm/ns^2
	//Atomic mass unit -> SI Unit
	inline const double amu2kg()		{return 1.66053878283E-27;}
	//Atomic mass unit -> Atomic Units
	inline const double amu2au()		{return (amu2kg()*kg2au());}
	//Atomic Units ->Atomic mass unit
	inline const double au2amu()		{return 1./amu2au();}
}

class MySpectrometer;
namespace MyMomentaCalculator  
{
	double px(double x_mm, double y_mm, double tof_ns, double mass_au, double charge_au, const MySpectrometer&);
	double py(double x_mm, double y_mm, double tof_ns, double mass_au, double charge_au, const MySpectrometer&);
	double pz(double tof_ns, double mass_au, double charge_au, const MySpectrometer&);
}

#endif