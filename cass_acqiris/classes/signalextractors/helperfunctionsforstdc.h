#ifndef _HELPERFUNCTIONS_H_
#define _HELPERFUNCTIONS_H_

/**
 * @file helperfunctionsforstdc.h file contains functions that help analysing
 *                                an acqiris waveform
 *
 * @author Lutz Foucar
 */

#include <iostream>
#include <cmath>
#include <cstdlib>
#include <sstream>

#include "channel.h"
#include "cass_event.h"
#include "acqiris_device.h"
#include "signal_producer.h"

namespace cass
{
  namespace ACQIRIS
  {
    /** extracts the requested channel from the data
     *
     * @note this function is just a template function because the compiler
     *       will give errors when its a regular function. This can be avoided
     *       by making this a functor struct or class.
     *
     * retrieve the Acqiris device from the CASSEvent. Then check if the device
     * contains the requested instruemnt. If not throw an invalid_argument
     * exception.\n
     * If the requested instruement exists, check if it contains the requested
     * channelnumber by checking how many channels it has. If it contains the
     * channel return a pointer to it. If the channel number is bigger than the
     * number of channels in the instrument, throw an invalid_argument exception.
     *
     * @return const pointer to the channel we need to extract
     * @param evt the CASSEvent wich contains the channel we want to extract
     * @param instrument the instrument that contains the channel
     * @param ChannelNumber the channel number of the requested channel
     *
     * @author Lutz Foucar
     */
    template <typename T>
    const Channel* extactRightChannel(const CASSEvent &evt,
                                      const Instruments& instrument,
                                      const size_t& ChannelNumber)
    {
      using namespace std;
      const Device &device
          (*(dynamic_cast<const ACQIRIS::Device*>(evt.devices().find(CASSEvent::Acqiris)->second)));
      ACQIRIS::Device::instruments_t::const_iterator instrumentIt
          (device.instruments().find(instrument));
      if (instrumentIt == device.instruments().end())
      {
        stringstream ss;
        ss << "extactRightChannel(): The requested Instrument '"<<instrument
            <<"' is not in the datastream";
        throw invalid_argument(ss.str());
      }
      const ACQIRIS::Instrument::channels_t &instrumentChannels
          (instrumentIt->second.channels());
      if ((ChannelNumber >= instrumentChannels.size()))
      {
        stringstream ss;
        ss<< "extactRightChannel(): The requested channel '"<<ChannelNumber
            <<"' does not exist in Instrument '"<<instrument<<"'";
        throw std::invalid_argument(ss.str());
      }
      return &(instrumentChannels[ChannelNumber]);
    }

    /** linear Regression
     *
     * this function creates a linear regression through a given amount of points
     * getting a line that follows the form: \f$y(x) = m*x + c\f$
     * it will calculate the slope m and the constant c of the line
     *
     * @param[in] nbrPoints the number of points for the regression
     * @param[in] x array of the x-values of the points
     * @param[in] y array of the y-values of the points
     * @param[out] m the slope of the line
     * @param[out] c the constant of the line (y value where it crosses the x-axis)
     *
     * @author Lutz Foucar
     */
    template <typename T>
        void linearRegression(const size_t nbrPoints, const double x[], const double y[], double &m, double &c)
    {
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

    /** A weighted linear Regression
     *
     * this function creates a weighted linear regression through a given amount of points
     * where we give a weight to the points that are farther away from the wanted x point
     * We will be getting a line that follows the form: \f$y(x) = m*x + c\f$
     * it will calculate the slope m and the constant c of the line
     *
     * @param[in] nbrPoints the number of points for the regression
     * @param[in] x array of the x-values of the points
     * @param[in] y array of the y-values of the points
     * @param[in] correctX the point where we calculate the distance from
     * @param[out] m the slope of the line
     * @param[out] c the constant of the line (y value where it crosses the x-axis)
     *
     * @author Lutz Foucar
     */
    template<typename T>
        void gewichtetlinearRegression(const size_t nbrPoints, const double x[], const double y[], const double correctX, double &m, double &c)
    {
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



    /** create Newton Polynomial
     *
     * This function creates the coefficients for Newton interpolating Polynomials.
     * Newton Polynomials are created from n Points and have the form
     * \f$p(x) = c_0 + c_1(x-x_0) + c_2(x-x_0)(x-x_1)+...+c_{n-1}(x-x_0)(x-x_1)...(x-x_{n-2})\f$
     * given that you have n Points
     * \f$(x_0,y_0), (x_1,y_1), ..., (x_{n-1},y_{n-1})\f$
     * Here we do it for 4 Points.
     *
     * @param[in] x the x-values of the points
     * @param[in] y the y-values of the points
     * @param[out] coeff the coefficients of the newton polynomial
     * @return void
     *
     * @author Lutz Foucar
     */
    template <typename T>
        void createNewtonPolynomial(const double * x, const double * y, double * coeff)
    {
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

    /** evaluate Newton Polynomial
     *
     * this function evaluates the Newton Polynomial that was created from n Points
     * \f$(x_0,y_0),..., (x_{n-1},y_{n-1}) with coefficients (c_0,...,c_{n-1})\f$
     * using Horner's Rule. This is done for an polynomial with 4 entries
     *
     * @param[in] x array of x values
     * @param[in] coeff array of coefficients
     * @param[in] X
     * @return the newton polynomial
     *
     * @author Lutz Foucar
     */
    template <typename T>
        double evalNewtonPolynomial(const double * x, const double * coeff, double X)
    {
      double returnValue = coeff[3];
      returnValue = returnValue * (X - x[2]) + coeff[2];
      returnValue = returnValue * (X - x[1]) + coeff[1];
      returnValue = returnValue * (X - x[0]) + coeff[0];

      return returnValue;
    }

    /** Achims Numerical Approximation
     *
     * this function should find x value corrosponding to a given y value
     * in a newton polynomial. It does it the following way:
     * -# create a lower and upper boundary point
     * -# create an interating x-value and initialize it with the Start value
     * -# evaluate the y-value at the x-value
     * -# if the y value is bigger than the requested value the start point
     *    is defines the new upper or lower boundary depending on the slope.
     * -# the new x-point is defined by the arithmetic mean between the tow
     *    boundary points.
     * -# do points 3-5 until the new x-value does not differ more than 0.005
     *    from the old one.
     *
     *@param[in] x two points describing upper and lower boundaries
     *@param[in] coeff the newton polynomial coefficents
     *@param[in] Y the requested y-values to find the x-value for
     *@param[in] Start the x-value we start the search with
     *
     *@author Lutz Foucar
     */
    template <typename T>
        double findXForGivenY(const double * x, const double * coeff, const double Y, const double Start)
    {
      //a point is a pair of doubles//
      typedef std::pair<double,double> punkt_t;
      //initialisiere die Grenzen//
      punkt_t Low(x[1], evalNewtonPolynomial<T>(x,coeff,x[1]));
      punkt_t Up (x[2], evalNewtonPolynomial<T>(x,coeff,x[2]));

      //initialisiere den iterierenden Punkt mit dem Startwert//
      punkt_t p (Start, evalNewtonPolynomial<T>(x,coeff,Start));

      //ist der Startpunkt schon der richtige Punkt//
      //liefere den dazugehoerigen x-Wert zurueck//
      if (p.second == Y)
        return p.first;

      //finde heraus ob es ein positiver oder ein negativer Durchgang ist//
      bool Neg = (Low.second > Up.second)?true:false;

      //der Startpunkt soll die richtige neue Grenze bilden//
      if (Neg)    //wenn es ein negativer Druchgang ist
      {
        if (p.second > Y)      //ist der y-Wert groesser als der gewollte
          Low = p;        //bildet der Punkt die neue untere Grenze
        else if (p.second < Y) //ist der y-Wert ist kleiner als der gewollte
          Up = p;         //bildet der Punkt die neue obere Grenze
        else                //ist der Punkt genau getroffen
          return p.first;   //liefer den dazugehoerigen x-Wert zurueck
      }
      else        //wenn es ein positiver Druchgang ist
      {
        if (p.second > Y)      //und der y-Wert groesser als der gewollte
          Up = p;         //bildet der Punkt die neue obere Grenze
        else if (p.second < Y) //und y-Wert ist kleiner als der gewollte
          Low = p;        //bildet der Punkt die neue untere Grenze
        else                //ist der Punkt genau getroffen
          return p.first;   //liefer den dazugehoerigen x-Wert zurueck
      }

      //iteriere solange bis der Abstand zwischen den x-Werten kleiner als 0.005
      while((Up.first-Low.first) > 0.005)
      {
        //bilde das arithmetische Mittel zwischen beiden Grenzen//
        //das ist der neue x-Wert unseres Punktes//
        p.first = 0.5 * (Up.first+Low.first);
        //finde den dazugehoerigen y-Wert//
        p.second = evalNewtonPolynomial<T>(x,coeff,p.first);

        if (Neg) //wenn es ein negativer Druchgang ist
        {
          if (p.second > Y)      //und der y-Wert groesser als der gewollte
            Low = p;        //bildet der Punkt die neue untere Grenze
          else if (p.second < Y) //und der y-Wert ist kleiner als der gewollte
            Up = p;         //bildet der Punkt die neue obere Grenze
          else                //ist der Punkt genau getroffen
            return p.first;   //liefer den dazugehoerigen x-Wert zurueck
        }
        else     //wenn es ein positiver Druchgang ist
        {
          if (p.second > Y)      //und der y-Wert groesser als der gewollte
            Up = p;         //bildet der Punkt die neue obere Grenze
          else if (p.second < Y) //und y-Wert ist kleiner als der gewollte
            Low = p;        //bildet der Punkt die neue untere Grenze
          else                //ist der Punkt genau getroffen
            return p.first;   //liefer den dazugehoerigen x-Wert zurueck
        }
        //        std::cout<<"("<<Low.x<<","<<Low.y<<")   ("<<p.x<<","<<p.y<<")   ("<<Up.x<<","<<Up.y<<") "<<Y<<std::endl;
      }
      //ist der gewuenschte Abstand zwischen den x-Werten erreicht
      //liefere das arithmetische mittel zwischen beiden zurueck
      return ((Up.first + Low.first)*0.5);
    }

    /** extract full width at half maximum (fwhm)
     *
     * @param[in] c the channel that the peak is found in
     * @param[in,out] s the peak that we found
     * @param thresh unused
     *
     * @author Lutz Foucar
     */
    template <typename T>
        void fwhm(const Channel &c, SignalProducer::signal_t &s, const double& /*thresh*/)
    {
      const waveform_t Data (c.waveform());
      const double vGain (c.gain());
      const int32_t vOff (static_cast<int32_t>(c.offset() / vGain));        //mV -> adc bytes
      const size_t wLength (c.waveform().size());
      const int maxpos (static_cast<int>(s["maxpos"]+0.1));

      //--get peak fwhm--//
      size_t fwhm_l        = 0;
      size_t fwhm_r        = 0;
      const double HalfMax = 0.5*s["maximum"];

      //--go from middle to left until 0.5*height find first point that is above 0.5 height--//
      for (int i(maxpos); i>=0; --i)
      {
        if (abs(Data[i]-vOff) < HalfMax)
        {
          fwhm_l = i+1;
          break;
        }
      }

      //--go from middle to right until 0.5*height (find last point that is still above 0.5 Height--//
      for (size_t i(maxpos); i<wLength;++i)
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

      //--set all found parameters--//
      s["fwhm"] = fwhm_R-fwhm_L;
      /** @todo make the below an own function*/
      s["width"] = s["stoppos"] - s["startpos"];
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

    /** Center of Mass
     *
     * find the center of mass of the peak by calculating also the integral of the
     * peak.
     *
     * @param[in] c the channel the peak was found in
     * @param[in,out] s the peak
     * @param[in] thresh the threshold that we used to identify the signal in V
     *
     * @author Lutz Foucar
     */
    template <typename T>
        void CoM(const Channel &c, SignalProducer::signal_t &s, const double& thresh)
    {
      //get informations from the event and the channel//
      const waveform_t Data (c.waveform());
      const double vGain (c.gain());
      const int32_t vOff (static_cast<int32_t>(c.offset() / vGain));
      const double horpos (c.horpos()*1.e9);          //s -> ns
      const double sampleInterval (c.sampleInterval()*1e9);   //s -> ns
      const int32_t threshold (static_cast<int32_t>(thresh/vGain));

      //--this function goes through the puls from start to stop and finds the center of mass--//
      double &integral (s["integral"]);
      double wichtung (0);
      const size_t start (static_cast<size_t>(s["startpos"]+0.1));
      const size_t stop (static_cast<size_t>(s["stoppos"]+0.1));

      for (size_t i = start; i<=stop;++i)
      {
        integral +=  (abs(Data[i]-vOff)-threshold);            //calc integral
        wichtung += ((abs(Data[i]-vOff)-threshold)*i);        //calc weight
      }
      s["com"] = (wichtung/integral + horpos)*sampleInterval;
    }



    /** find start and stop of pulse
     *
     * this function will find the start and the stop of the signal
     *
     * @param[in] c the channel the signal was found in
     * @param[in,out] s the signal
     * @param[in] thresh the threshold that we used to identify the signal in V
     * @author Lutz Foucar
     */
    template <typename T>
        void startstop(const Channel &c, SignalProducer::signal_t &s, const double& thresh)
    {
      const waveform_t Data (c.waveform());
      const double vGain (c.gain());
      const int32_t vOff (static_cast<int32_t>(c.offset()/vGain));
      const int32_t wLength (c.waveform().size());
      const double sampInt (c.sampleInterval()*1e9);
      const double horpos (c.horpos()*1e9);
      const int32_t threshold (static_cast<int32_t>(thresh/vGain));


      //calculate the center of peak is in the waveform coodinates//
      const int32_t center (static_cast<int32_t>((s["time"]/sampInt - horpos)+0.1));

      //go left from center until either i == 0, or the datapoint is inside the noise
      //or we go from the previous one (i+1) to the actual one (i) through the baseline
      int i=0;
      for (i = center; i>=0; --i)
        if ((abs(Data[i]-vOff) < threshold) || (((Data[i]-vOff) * (Data[i+1]-vOff)) < 0) )
          break;
      s["startpos"] = i;

      //go right form center until either i < pulslength, or the datapoint is inside the noise
      //or we go from the previous one (i-1) to the actual one (i) through the baseline
      for (i = center; i< wLength; ++i)
        if ((abs(Data[i]-vOff) < threshold) || (((Data[i]-vOff) * (Data[i-1]-vOff)) < 0) )
          break;
      s["stoppos"] = i;
    }


    /** find Maximum of puls and calcs the height
     *
     * this function will find the maximum of the peak and its position
     *
     * @param[in] c the channel the peak was found in
     * @param[in,out] s the peak
     * @param thresh unused
     *
     * @author Lutz Foucar
     */
    template <typename T>
        void maximum(const Channel &c, SignalProducer::signal_t &s, const double& /*thresh*/)
    {
      const waveform_t Data (c.waveform());
      const double vGain (c.gain());
      const int32_t vOff (static_cast<int32_t>(c.offset()/vGain));

      const size_t start (static_cast<size_t>(s["startpos"]+0.1));
      const size_t stop (static_cast<size_t>(s["stoppos"]+0.1));
      double& maximum (s["maximum"]);
      double& maxpos (s["maxpos"]);

      maximum = 0;
      for (size_t i(start); i<=stop;++i)
      {
        if (abs(Data[i]-vOff) > maximum)
        {
          maximum = abs(Data[i]-vOff);
          maxpos  = i;
        }
      }
      /** @todo make the below an own function */
      s["height"]  = maximum * vGain;        //this will set the height in mV
    }




  }//end namespace remi
}//end namespace cass

#endif
