#ifndef __MySpectrometer_H_
#define __MySpectrometer_H_

#include <vector>


class SpectrometerRegion
{
public:
  SpectrometerRegion():fL(0),fF(0)	{}
  SpectrometerRegion(double length_mm, double efield_Vpcm):
		fL(length_mm),
		fF(efield_Vpcm)	{}

	double  EField_Vpcm()const			{return fF;}
	double &EField_Vpcm()				{return fF;}
	double  Length_mm()const			{return fL;}
	double &Length_mm()					{return fL;}

private:
	double  fL;					//the length of the spectrometer region
	double  fF;					//the electric field inside this region
};


typedef std::vector<SpectrometerRegion> SpecRegions;
class Spectrometer	//-------------Class describing a Spectrometer-Part ---------------------
{
public:
  Spectrometer():fMFieldOn(false),fCw(false),fCycFreq(0)								{}

	double				 CyclotronPeriod_ns()const											{return fCycFreq;}
	double				&CyclotronPeriod_ns()												{return fCycFreq;}
	bool				 MagneticFieldIsOn()const											{return fMFieldOn;}
	bool				&MagneticFieldIsOn()												{return fMFieldOn;}
	bool				 RotationClockWise()const											{return fCw;}
	bool				&RotationClockWise()												{return fCw;}

  void				 AddSpectrometerRegion(double Length_mm, double EField_Vpcm)		{fSr.push_back(SpectrometerRegion(Length_mm,EField_Vpcm));}
	const SpecRegions	&GetSpectrometerRegions() const										{return fSr;}
	SpecRegions			&GetSpectrometerRegions() 											{return fSr;}
	void				 Clear()															{fSr.clear();}

private:
	SpecRegions			 fSr;																//the spectrometer has several regions
	bool				 fMFieldOn;															//flag showing wether a magnetic field was turned on
	double				 fCycFreq;															//the cyclotron frequency of electrons, defines the strength of magnetic field
	bool				 fCw;																//flag showing the direction of the magnetic field (by saying wether electron turn clock or counterclockwise
};
#endif
