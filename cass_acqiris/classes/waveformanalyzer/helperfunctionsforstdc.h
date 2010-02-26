#ifndef _HELPERFUNCTIONS_H_
#define _HELPERFUNCTIONS_H_

#include <iostream>
#include <cmath>
#include <cstdlib>

#include "channel.h"
#include "peak.h"

namespace cass
{
  namespace ACQIRIS
  {
    //_________________________________helper function that does a linear Regression_____________________________________________________
    template <typename T>
        void linearRegression(const size_t nbrPoints, const double x[], const double y[], double &m, double &c)
    {
      //--this funktion does a linear regression of 4 points--//
      //--getting a line that follows the form: y(x) = m*x + c--//
      //--return the x value for a given y(x) (yb)--//
      //-- => x = (y(x)-c)/m--//
      double SumXsq=0.,SumX=0.,SumY=0.,SumXY=0.;
      for (size_t i=0;i<nbrPoints;++i)
      {
        SumX    +=  x[i];
        SumY    +=  y[i];
        SumXY   += (x[i]*y[i]);
        SumXsq  += (x[i]*x[i]);
      }

      double a1 = ((SumX*SumX) - (nbrPoints*SumXsq));

      m = ((SumX*SumY) - (nbrPoints*SumXY)) / a1;
      c = ((SumX*SumXY) - (SumY*SumXsq)) / a1;
    }

    //_______________________________________helper function that does a weighted linear Regression__________________________________________
    template<typename T>
        void gewichtetlinearRegression(const size_t nbrPoints, const double x[], const double y[], const double correctX, double &m, double &c)
    {
      //--this funktion does a linear regression of 4 points--//
      //--getting a line that follows the form: y(x) = m*x + c--//
      //--return the x value for a given y(x) (yb)--//
      //-- => x = (y(x)-c)/m--//
      double SumXsq=0.,SumX=0.,SumY=0.,SumXY=0.,SumWeight=0.;
      for (size_t i=0;i<nbrPoints;++i)
      {
        double weight = (fabs(x[i]-correctX) > 1e-10) ? 1./fabs(x[i]-correctX): 100.;
        SumWeight += weight;
        SumX      += (x[i]*weight);
        SumY      += (y[i]*weight);
        SumXY     += (x[i]*y[i]*weight);
        SumXsq    += (x[i]*x[i]*weight);
      }

      double a1 = ((SumX*SumX) - (SumWeight*SumXsq));

      m = ((SumX*SumY) - (SumWeight*SumXY)) / a1;
      c = ((SumX*SumXY) - (SumY*SumXsq)) / a1;
    }



    //_________________________________create Newton Polynomial__________________________________________________________________________
    template <typename T>
        void createNewtonPolynomial(const double * x, const double * y, double * coeff)
    {
      //**this function creates the coefficients for Newton interpolating Polynomials  **//
          //**Newton Polynomial are Created from n Points and have the form                **//
          //**p(x) = c0 + c1(x-x0) + c2(x-x0)(x-x1)+...+c_(n-1)(x-x0)(x-x1)...(x-x_(n-2))  **//
          //**given that you have n Points (x0,y0), (x1,y1), ..., (x_(n-1),y_(n-1))        **//

          double f_x0_x1 = (y[1]-y[0]) / (x[1]-x[0]);
      double f_x1_x2 = (y[2]-y[1]) / (x[2]-x[1]);
      double f_x2_x3 = (y[3]-y[2]) / (x[3]-x[2]);

      double f_x0_x1_x2 = (f_x1_x2 - f_x0_x1) / (x[2]-x[0]);
      double f_x1_x2_x3 = (f_x2_x3 - f_x1_x2) / (x[3]-x[1]);

      double f_x0_x1_x2_x3 = (f_x1_x2_x3 - f_x0_x1_x2) / (x[3]-x[0]);

      coeff[0] = y[0];
      coeff[1] = f_x0_x1;
      coeff[2] = f_x0_x1_x2;
      coeff[3] = f_x0_x1_x2_x3;
    }

    //_________________________________evaluate Newton Polynomial________________________________________________________________________
    template <typename T>
        double evalNewtonPolynomial(const double * x, const double * coeff, double X)
    {
      //**this function evaluates the Newton Polynomial that was created from n Points**//
          //** (x0,y0),..., (x(n-1),y(n-1)) with coefficients (c0,...,c(n-1))             **//
          //**using Horner's Rule                                                         **//

          double returnValue = coeff[3];
      returnValue = returnValue * (X - x[2]) + coeff[2];
      returnValue = returnValue * (X - x[1]) + coeff[1];
      returnValue = returnValue * (X - x[0]) + coeff[0];

      return returnValue;
    }

    //_________________________________Achims Numerical Approximation____________________________________________________________________
    class MyPunkt
    {
        public:
      MyPunkt(double xx, double yy):X(xx),Y(yy){}
      double &x() {return X;}
      double &y() {return Y;}
        private:
      double X;
      double Y;
    };
    template <typename T>
        double findXForGivenY(const double * x, const double * coeff, const double Y, const double Start)
    {
      //initialisiere die Grenzen//
      MyPunkt Low(x[1], evalNewtonPolynomial<T>(x,coeff,x[1]));
      MyPunkt Up (x[2], evalNewtonPolynomial<T>(x,coeff,x[2]));

      //initialisiere den iterierenden Punkt mit dem Startwert//
      MyPunkt p (Start, evalNewtonPolynomial<T>(x,coeff,Start));

      //ist der Startpunkt schon der richtige Punkt//
      //liefere den dazugehoerigen x-Wert zurueck//
      if (p.y() == Y)
        return p.x();

      //finde heraus ob es ein positiver oder ein negativer Durchgang ist//
      bool Neg = (Low.y() > Up.y())?true:false;

      //der Startpunkt soll die richtige neue Grenze bilden//
      if (Neg)    //wenn es ein negativer Druchgang ist
      {
        if (p.y() > Y)      //ist der y-Wert groesser als der gewollte
          Low = p;        //bildet der Punkt die neue untere Grenze
        else if (p.y() < Y) //ist der y-Wert ist kleiner als der gewollte
          Up = p;         //bildet der Punkt die neue obere Grenze
        else                //ist der Punkt genau getroffen
          return p.x();   //liefer den dazugehoerigen x-Wert zurueck
      }
      else        //wenn es ein positiver Druchgang ist
      {
        if (p.y() > Y)      //und der y-Wert groesser als der gewollte
          Up = p;         //bildet der Punkt die neue obere Grenze
        else if (p.y() < Y) //und y-Wert ist kleiner als der gewollte
          Low = p;        //bildet der Punkt die neue untere Grenze
        else                //ist der Punkt genau getroffen
          return p.x();   //liefer den dazugehoerigen x-Wert zurueck
      }

      //iteriere solange bis der Abstand zwischen den x-Werten kleiner als 0.005
      while((Up.x()-Low.x()) > 0.005)
      {
        //bilde das arithmetische Mittel zwischen beiden Grenzen//
        //das ist der neue x-Wert unseres Punktes//
        p.x() = 0.5 * (Up.x()+Low.x());
        //finde den dazugehoerigen y-Wert//
        p.y() = evalNewtonPolynomial<T>(x,coeff,p.x());

        if (Neg) //wenn es ein negativer Druchgang ist
        {
          if (p.y() > Y)      //und der y-Wert groesser als der gewollte
            Low = p;        //bildet der Punkt die neue untere Grenze
          else if (p.y() < Y) //und der y-Wert ist kleiner als der gewollte
            Up = p;         //bildet der Punkt die neue obere Grenze
          else                //ist der Punkt genau getroffen
            return p.x();   //liefer den dazugehoerigen x-Wert zurueck
        }
        else     //wenn es ein positiver Druchgang ist
        {
          if (p.y() > Y)      //und der y-Wert groesser als der gewollte
            Up = p;         //bildet der Punkt die neue obere Grenze
          else if (p.y() < Y) //und y-Wert ist kleiner als der gewollte
            Low = p;        //bildet der Punkt die neue untere Grenze
          else                //ist der Punkt genau getroffen
            return p.x();   //liefer den dazugehoerigen x-Wert zurueck
        }
        //        std::cout<<"("<<Low.x<<","<<Low.y<<")   ("<<p.x<<","<<p.y<<")   ("<<Up.x<<","<<Up.y<<") "<<Y<<std::endl;
      }
      //ist der gewuenschte Abstand zwischen den x-Werten erreicht
      //liefere das arithmetische mittel zwischen beiden zurueck
      return ((Up.x() + Low.x())*0.5);
    }

    //_______extract full width at half maximum________________________________________________________________________________________
    template <typename T>
        void fwhm(const Channel &c, Peak &p)
    {
      //get information from the channel//

      const int32_t vOff   = static_cast<int32_t>(c.offset() / c.gain());        //mV -> adc bytes
      const Channel::waveform_t Data  = c.waveform();
      const size_t wLength = c.waveform().size();

      //--get peak fwhm--//
      size_t fwhm_l        = 0;
      size_t fwhm_r        = 0;
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
      lx[0] = fwhm_l-2;    ly[0] = abs(Data[fwhm_l-2]-vOff);
      lx[1] = fwhm_l-1;    ly[1] = abs(Data[fwhm_l-1]-vOff);
      lx[2] = fwhm_l-0;    ly[2] = abs(Data[fwhm_l-0]-vOff);
      lx[3] = fwhm_l+1;    ly[3] = abs(Data[fwhm_l+1]-vOff);

      double rx[4];
      double ry[4];
      rx[0] = fwhm_r-1;    ry[0] = abs(Data[fwhm_r-1]-vOff);
      rx[1] = fwhm_r-0;    ry[1] = abs(Data[fwhm_r-0]-vOff);
      rx[2] = fwhm_r+1;    ry[2] = abs(Data[fwhm_r+1]-vOff);
      rx[3] = fwhm_r+2;    ry[3] = abs(Data[fwhm_r+2]-vOff);

      double mLeft,cLeft,mRight,cRight;
      linearRegression<T>(4,lx,ly,mLeft,cLeft);
      linearRegression<T>(4,rx,ry,mRight,cRight);

      //y = m*x+c => x = (y-c)/m;
      const double fwhm_L = (HalfMax-cLeft)/mLeft;
      const double fwhm_R = (HalfMax-cRight)/mRight;

      const double fwhm = fwhm_R-fwhm_L;
      //--set all found parameters--//
      p.fwhm()  = fwhm;
      p.width() = p.stoppos() - p.startpos();
    }


    //        //_________________extract the voltage of a channel______________________________________________________________________________________________________________
    //        template <typename T>
    //        void extractVoltage(const MyOriginalChannel &oc, const MyPuls &p, const MyChannelSection &cs, MySignalAnalyzedChannel &sac)
    //        {
    //            double volt           = 0;
    //            int count             = 0;
    //            const double gain     = oc.GetVertGain();
    //            const double offset   = oc.GetOffset();
    //            const T *d            = static_cast<const T*>(oc.GetDataPointerForPuls(p));
    //
    //            for (int j=10;j<p.GetLength()-10;++j)
    //            {
    //                volt += (d[j]*gain)-offset;
    //                ++count;
    //            }
    //            volt /= count;
    //
    //            sac.AddVoltage(volt,cs.GetChannelSectionNbr());
    //        }

    //__________________________________________Center of Mass_______________________________________
    template <typename T>
        void CoM(const Channel &c, Peak &p, const int32_t threshold)
    {
      //get informations from the event and the channel//
      const Channel::waveform_t Data  = c.waveform();
      const int32_t vOff          = static_cast<int32_t>(c.offset() / c.gain());
      const double horpos         = c.horpos()*1.e9;          //s -> ns
      const double sampleInterval = c.sampleInterval()*1e9;   //s -> ns

      //--this function goes through the puls from start to stop and finds the center of mass--//
      double integral = 0;
      double wichtung = 0;
      const size_t start = p.startpos();
      const size_t stop  = p.stoppos();

      for (size_t i = start; i<=stop;++i)
      {
        integral +=  (abs(Data[i]-vOff)-threshold);            //calc integral
        wichtung += ((abs(Data[i]-vOff)-threshold)*i);        //calc weight
      }
      p.integral()    = integral;
      p.com()         = (wichtung/integral + horpos)*sampleInterval;
    }



    //______________________________________find start and stop of pulse___________________________________________
    template <typename T>
        void startstop(const Channel &c, Peak &p, const int32_t threshold)
    {
      //--this function will find the start and the stop of the peak--//
      const Channel::waveform_t Data = c.waveform();
      const int32_t vOff      = static_cast<int32_t>(c.offset()/c.gain());
      const int32_t wLength   = c.waveform().size();
      const double sampInt    = c.sampleInterval()*1e9;
      const double horpos     = c.horpos()*1e9;

      //calculate the center of peak is in the waveform coodinates//
      const int32_t center        = static_cast<int32_t>(p.time()/sampInt - horpos);


      //go left from center until either i == 0, or the datapoint is inside the noise
      //or we go from the previous one (i+1) to the actual one (i) through the baseline
      int i=0;
      for (i = center; i>=0; --i)
        if ((abs(Data[i]-vOff) < threshold) || (((Data[i]-vOff) * (Data[i+1]-vOff)) < 0) )
          break;
      p.startpos() = i;

      //go right form center until either i < pulslength, or the datapoint is inside the noise
      //or we go from the previous one (i-1) to the actual one (i) through the baseline
      for (i = center; i< wLength; ++i)
        if ((abs(Data[i]-vOff) < threshold) || (((Data[i]-vOff) * (Data[i-1]-vOff)) < 0) )
          break;
      p.stoppos() = i;
    }


    //___________________________________find Maximum of puls and calcs the height_____________________________________________
    template <typename T>
        void maximum(const Channel &c, Peak &p)
    {
      //--this function will find the maximum of the peak and its position--//
      const Channel::waveform_t Data = c.waveform();
      const double vGain   = c.gain();
      const int32_t vOff   = static_cast<int32_t>(c.offset()/vGain);
      const size_t start   = p.startpos();
      const size_t stop    = p.stoppos();
      int32_t maximum      = 0;
      int maxpos           = 0;

      for (size_t i = start; i<=stop;++i)
      {
        if (abs(Data[i]-vOff) > maximum)
        {
          maximum = abs(Data[i]-vOff);
          maxpos  = i;
        }
      }
      p.maxpos()  = maxpos;
      p.maximum() = maximum;
      p.height()  = static_cast<double>(maximum) * vGain;        //this will set the height in mV
    }




  }//end namespace remi
}//end namespace cass

#endif
