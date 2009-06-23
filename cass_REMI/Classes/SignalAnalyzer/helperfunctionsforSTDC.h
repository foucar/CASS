#ifndef __helperfunctions_h__
#define __helperfunctions_h__

#include <iostream>

#include "REMIEvent.h"
#include "Channel.h"
#include "Peak.h"

namespace cass
{
	namespace REMI
	{
        //_________________________________helper function that does a linear Regression_____________________________________________________
        void linearRegression(int nbrPoints, const double x[], const double y[], double &m, double &c);
        //_________________________________helper function that does a linear Regression_____________________________________________________
        void gewichtetlinearRegression(const int nbrPoints, const double x[], const double y[], const double correctX, double &m, double &c);
        //_________________________________create Newton Polynomial__________________________________________________________________________
        void createNewtonPolynomial(const double * x, const double * y, double * coeff);
        //_________________________________evaluate Newton Polynomial________________________________________________________________________
        double evalNewtonPolynomial(const double * x, const double * coeff, double X);
        //_________________________________Achims Numerical Approximation____________________________________________________________________
        double findXForGivenY(const double * x, const double * coeff, double Y, double Start);
        //_________________________________gib mir zurück____________________________________________________________________________________
        //double gmz(const double i, const double x0, const dvec MPuls);

		//______________________________fwhm________________________________________________________________________________________
		template <typename T>
		void fwhm(const Channel &c, Peak &p)
		{
			//get information from the channel//

			const long vOff	 	 = static_cast<long>(c.vertOffset() / c.vertGain());		//mV -> adc bytes
			const T *Data	 	 = static_cast<const T*>(c.waveform());
			const size_t wLength = c.waveformLength();

			//--get peak fwhm--//
			size_t fwhm_l		 = 0;
			size_t fwhm_r		 = 0;
			const double HalfMax = 0.5*p.maximum();

			////--go from middle to left until 0.5*height find first point that is above 0.5 height--//
			for (int i=p.maxpos(); i>=0; --i)
			{
				if (abs(Data[i]-vOff) < HalfMax)
				{
					fwhm_l = i+1;
					break;
				}
			}

			//--go from middle to right until 0.5*height (find last point that is still above 0.5 Height--//
			for (size_t i=p.maxpos(); i<wLength;++i)
			{
				if (abs(Data[i]-vOff) < HalfMax)
				{
					fwhm_r = i-1;
					break;
				}
			}

			//--if we found a right side and a left side, then--//
			//--compute the fwhm with a linear interpolation--//
			//--between the points that are left and right from--//
			//--where the fwhm is, else return here--//
			if (!fwhm_r || !fwhm_l)
				return;

			double lx[4];
			double ly[4];
			lx[0] = fwhm_l-2;	ly[0] = abs(Data[fwhm_l-2]-vOff);
			lx[1] = fwhm_l-1;	ly[1] = abs(Data[fwhm_l-1]-vOff);
			lx[2] = fwhm_l-0;	ly[2] = abs(Data[fwhm_l-0]-vOff);
			lx[3] = fwhm_l+1;	ly[3] = abs(Data[fwhm_l+1]-vOff);

			double rx[4];
			double ry[4];
			rx[0] = fwhm_r-1;	ry[0] = abs(Data[fwhm_r-1]-vOff);
			rx[1] = fwhm_r-0;	ry[1] = abs(Data[fwhm_r-0]-vOff);
			rx[2] = fwhm_r+1;	ry[2] = abs(Data[fwhm_r+1]-vOff);
			rx[3] = fwhm_r+2;	ry[3] = abs(Data[fwhm_r+2]-vOff);

			double mLeft,cLeft,mRight,cRight;
            linearRegression(4,lx,ly,mLeft,cLeft);
            linearRegression(4,rx,ry,mRight,cRight);

			//y = m*x+c => x = (y-c)/m;
			const double fwhm_L = (HalfMax-cLeft)/mLeft;
			const double fwhm_R = (HalfMax-cRight)/mRight;

			const double fwhm = fwhm_R-fwhm_L;
			//--set all found parameters--//
			p.fwhm(fwhm);
			p.width(p.stoppos()-p.startpos());
		}
		////_______________________________________________________________________________________________________________________________
		//template <typename T>
		//void extractVoltage(const MyOriginalChannel &oc, const MyPuls &p, const MyChannelSection &cs, MySignalAnalyzedChannel &sac)
		//{
		//    double volt					= 0;
		//	int count					= 0;
		//	const double gain			= oc.GetVertGain();
		//	const double offset			= oc.GetOffset();
		//	const T *d					= static_cast<const T*>(oc.GetDataPointerForPuls(p));
		//	
		//	for (int j=10;j<p.GetLength()-10;++j)
		//	{
		//		volt += (d[j]*gain)-offset;
		//		++count;
		//	}
		//	volt /= count;
		//
		//	sac.AddVoltage(volt,cs.GetChannelSectionNbr());
		//}

		//__________________________________________Center of Mass_______________________________________
		template <typename T>
        void CoM(const REMIEvent &e, const Channel &c, Peak &p)
		{
			//get informations from the event and the channel//
			const T * Data				= static_cast<const T*>(c.waveform());
			const long vOff				= static_cast<long>(c.vertOffset() / c.vertGain());
			const long threshold		= c.threshold();
			const long timestamp		= c.idxToFirstPoint();
			const double horpos			= e.horpos()*1.e9;			//s -> ns
			const double sampleInterval = e.sampleInterval()*1e9;	//s -> ns

			//--this function goes through the puls from start to stop and finds the center of mass--//
			double integral = 0;
			double wichtung = 0;
			const size_t start = p.startpos();
			const size_t stop  = p.stoppos();
			
			for (size_t i = start; i<=stop;++i)
			{
				integral +=  (abs(Data[i]-vOff)-threshold);			//calc integral
				wichtung += ((abs(Data[i]-vOff)-threshold)*i);		//calc weight
			}
			p.integral(integral);
			p.com((wichtung/integral + timestamp + horpos)*sampleInterval);
		}



		//______________________________________find start and stop of pulse___________________________________________
		template <typename T>
        void startstop(const REMIEvent &e, const Channel &c, Peak &p)
		{
			//--this function will find the start and the stop of the peak--//
			const T * Data			= static_cast<const T*>(c.waveform());
			const long vOff			= static_cast<long>(c.vertOffset()/c.vertGain());
			const long threshold	= c.threshold();
			const long wLength		= c.waveformLength();
			const double sampInt	= e.sampleInterval()*1e9;
			const double horpos		= e.horpos()*1e9;
			const long timestamp	= c.idxToFirstPoint();

			//calculate the center of peak is in the waveform coodinates//
			const long center		= static_cast<long>(p.time()/sampInt - timestamp - horpos);
			

			//go left from center until either i == 0, or the datapoint is inside the noise
			//or we go from the previous one (i+1) to the actual one (i) through the baseline
			int i=0;
			for (i = center; i>=0; --i)				
				if ((abs(Data[i]-vOff) < threshold) || (((Data[i]-vOff) * (Data[i+1]-vOff)) < 0) )
					break;
			p.startpos(i);

			//go right form center until either i < pulslength, or the datapoint is inside the noise
			//or we go from the previous one (i-1) to the actual one (i) through the baseline
			for (i = center; i< wLength; ++i)				
				if ((abs(Data[i]-vOff) < threshold) || (((Data[i]-vOff) * (Data[i-1]-vOff)) < 0) )
					break;
			p.stoppos(i);
		}


		//___________________________________find Maximum of puls and calcs the height_____________________________________________
		template <typename T>
		void maximum(const Channel &c, Peak &p)
		{
			//--this function will find the maximum of the peak and its position--//
			const T *Data		= static_cast<const T*>(c.waveform());
			const long vOff		= c.vertOffset();
			const size_t start	= p.startpos();
			const size_t stop	= p.stoppos();
			const double vGain	= c.vertGain();
			long maximum		= 0;
			int maxpos			= 0;

            for (size_t i = start; i<=stop;++i)
			{
				if (abs(Data[i]-vOff) > maximum)
				{
					maximum = abs(Data[i]-vOff);
					maxpos = i;
				}
			}
			p.maxpos(maxpos);
			p.maximum(maximum);
			p.height(static_cast<double>(maximum) * vGain);		//this will set the height in mV
		}




	}//end namespace remi
}//end namespace cass

#endif
