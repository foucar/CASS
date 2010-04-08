//Copyright (C) 2010 lmf
#include <cmath>

#include "cfd.h"
#include "helperfunctionsforstdc.h"
#include "waveform_signal.h"


//________________________________Implematation of Constant Fraction Method______________________________________________________
//________________this will be a thread that is waiting for Pulses to be added to a queue________________________________________
template <typename T>
void cfd(const cass::ACQIRIS::Channel& c, cass::ACQIRIS::ResultsBackend& result)
{
  //get reference to the signal//
  cass::ACQIRIS::Signal &s = dynamic_cast<cass::ACQIRIS::Signal&>(result);
  //now extract information from the Channel
  const double sampleInterval = c.sampleInterval()*1e9;    //convert the s to ns
  const double horpos     = c.horpos()*1.e9;
  const double vGain      = c.gain();
  const int32_t vOff      = static_cast<int32_t>(c.offset() / vGain);       //mV -> ADC Bytes

  const int32_t idxToFiPoint = 0;
  const cass::ACQIRIS::Channel::waveform_t Data = c.waveform();
  const size_t wLength    = c.waveform().size();

  //--get the right cfd settings--//
  const int32_t delay     = static_cast<int32_t>(s.delay() / sampleInterval); //ns -> sampleinterval units
  const double walk       = s.walk() / vGain;                                 //mV -> ADC Bytes
  const double threshold  = s.threshold() / vGain;                            //mV -> ADC Bytes
  const double fraction   = s.fraction();

  //--go through the waveform--//
  for (size_t i=delay+1; i<wLength-2;++i)
  {
    const double fx  = Data[i] - static_cast<double>(vOff);         //the original Point at i
    const double fxd = Data[i-delay] - static_cast<double>(vOff);   //the delayed Point    at i
    const double fsx = -fx*fraction + fxd;                          //the calculated CFPoint at i

    const double fx_1  = Data[i+1] - static_cast<double>(vOff);        //original Point at i+1
    const double fxd_1 = Data[i+1-delay] - static_cast<double>(vOff);  //delayed Point at i+1
    const double fsx_1 = -fx_1*fraction + fxd_1;                       //calculated CFPoint at i+1

    //check wether the criteria for a Peak are fullfilled
    if (((fsx-walk) * (fsx_1-walk)) <= 0 ) //one point above one below the walk
      if (fabs(fx) > threshold)              //original point above the threshold
      {
      //--it could be that the first criteria is 0 because  --//
      //--one of the Constant Fraction Signal Points or both--//
      //--are exactly where the walk is                     --//
      if (fabs(fsx-fsx_1) < 1e-8)    //both points are on the walk
      {
        //--go to next loop until at least one is over or under the walk--//
        continue;
      }
      else if ((fsx-walk) == 0)        //only first is on walk
      {
        //--Only the fist is on the walk, this is what we want--//
        //--so:do nothing--//
      }
      else if ((fsx_1-walk) == 0)        //only second is on walk
      {
        //--we want that the first point will be on the walk,--//
        //--so in the next loop this point will be the first.--//
        continue;
      }
      //does the peak have the right polarity?//
      //if two pulses are close together then the cfsignal goes through the walk//
      //three times, where only two crossings are good. So we need to check for//
      //the one where it is not good//
      if (fsx     > fsx_1)   //neg polarity
        if (Data[i] > vOff)    //but pos Puls .. skip
          continue;
      if (fsx     < fsx_1)   //pos polarity
        if (Data[i] < vOff)    //but neg Puls .. skip
          continue;


      //--later we need two more points, create them here--//
      const double fx_m1 = Data[i-1] - static_cast<double>(vOff);        //the original Point at i-1
      const double fxd_m1 = Data[i-1-delay] - static_cast<double>(vOff); //the delayed Point    at i-1
      const double fsx_m1 = -fx_m1*fraction + fxd_m1;                    //the calculated CFPoint at i-1

      const double fx_2 = Data[i+2] - static_cast<double>(vOff);         //original Point at i+2
      const double fxd_2 = Data[i+2-delay] - static_cast<double>(vOff);  //delayed Point at i+2
      const double fsx_2 = -fx_2*fraction + fxd_2;                       //calculated CFPoint at i+2


      //--find x with a linear interpolation between the two points--//
      const double m = fsx_1-fsx;                    //(fsx-fsx_1)/(i-(i+1));
      const double xLin = i + (walk - fsx)/m;        //PSF fx = (x - i)*m + cfs[i]

      //--make a linear regression to find the slope of the leading edge--//
      double mslope,cslope;
      const double xslope[3] = {i-delay,i+1-delay,i+2-delay};
      const double yslope[3] = {fxd,fxd_1,fxd_2};
      cass::ACQIRIS::linearRegression<T>(3,xslope,yslope,mslope,cslope);

      //--find x with a cubic polynomial interpolation between four points--//
      //--do this with the Newtons interpolation Polynomial--//
      const double x[4] = {i-1,i,i+1,i+2};          //x vector
      const double y[4] = {fsx_m1,fsx,fsx_1,fsx_2}; //y vector
      double coeff[4] = {0,0,0,0};                  //Newton coeff vector
      cass::ACQIRIS::createNewtonPolynomial<T>(x,y,coeff);

      //--numericaly solve the Newton Polynomial--//
      //--give the lineare approach for x as Start Value--//
      const double xPoly = cass::ACQIRIS::findXForGivenY<T>(x,coeff,walk,xLin);
      const double pos = xPoly + static_cast<double>(idxToFiPoint) + horpos;

      //--create a new peak--//
      cass::ACQIRIS::Peak p;

      //add the info//
      p.cfd()     = pos*sampleInterval;
      p.time()    = pos*sampleInterval;
      if (fsx > fsx_1) p.polarity()            = cass::ACQIRIS::Negative;       //Peak has Neg Pol
      if (fsx < fsx_1) p.polarity()            = cass::ACQIRIS::Positive;       //Peak has Pos Pol
      if (fabs(fsx-fsx_1) < 1e-8) p.polarity() = cass::ACQIRIS::Bad;            //Peak has Bad Pol

      //--start and stop of the puls--//
      cass::ACQIRIS::startstop<T>(c,p,static_cast<const int32_t>(threshold));

      //--height of peak--//
      cass::ACQIRIS::maximum<T>(c,p);

      //--width & fwhm of peak--//
      cass::ACQIRIS::fwhm<T>(c,p);

      //--the com and integral--//
      cass::ACQIRIS::CoM<T>(c,p,static_cast<const int32_t>(threshold));

      //--add peak to signal if it fits the conditions--//
      if(p.polarity() == s.polarity())  //if it has the right polarity
        if(p.time() > s.trLow() && p.time() < s.trHigh()) //if signal is in the right timerange
          s.peaks().push_back(p);
    }
  }
}

//########################## 8 Bit Version ###########################################################################
//______________________________________________________________________________________________________________________
void cass::ACQIRIS::CFD8Bit::analyze(const cass::ACQIRIS::Channel& c, cass::ACQIRIS::ResultsBackend& r)
{
  cfd<char>(c,r);
}

//########################## 16 Bit Version ###########################################################################
//______________________________________________________________________________________________________________________
void cass::ACQIRIS::CFD16Bit::analyze(const cass::ACQIRIS::Channel& c, cass::ACQIRIS::ResultsBackend& r)
{
  cfd<short>(c,r);
}
