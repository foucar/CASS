//Copyright (C) 2003-2010 Lutz Foucar

/**
 * @file cfd.cpp file contains definition of class that does a constant fraction
 *               descrimination like analysis of a waveform
 *
 * @author Lutz Foucar
 */

#include <typeinfo>
#include <cmath>
#include <limits>

#include "cfd.h"

#include "channel.h"
#include "cass_event.h"
#include "cass_settings.h"
#include "helperfunctionsforstdc.h"

using namespace cass::ACQIRIS;
namespace cass
{
  namespace ACQIRIS
  {
    namespace ConstantFraction
    {
      /** Implematation of Constant Fraction Method
       *
       * @tparam T type of a wavform point
       * @param[in] c the channel that contains the waveform to analyze
       * @param[in] param the user defined parameters for extracting signal in the
       *        waveform
       * @param[out] sig the container with all the found signals
       *
       * @author Lutz Foucar
       */
      template <typename T>
          void cfd(const Channel& c, const CFDParameters &param, SignalProducer::signals_t& sig)
      {
        using namespace cass::ACQIRIS;
        //make sure that we are the right one for the waveform_t//
        assert(typeid(waveform_t::value_type) == typeid(T));

        //now extract information from the Channel
        const double sampleInterval = c.sampleInterval()*1e9;    //convert the s to ns
        const double horpos     = c.horpos()*1.e9;
        const double vGain      = c.gain();
        const int32_t vOff      = static_cast<int32_t>(c.offset() / vGain);       //V -> ADC Bytes

        const int32_t idxToFiPoint = 0;
        const waveform_t Data = c.waveform();
        const size_t wLength    = c.waveform().size();

        //--get the right cfd settings--//
        const int32_t delay     = static_cast<int32_t>(param._delay / sampleInterval); //ns -> sampleinterval units
        const double walk       = param._walk / vGain;                                 //V -> ADC Bytes
        const double threshold  = param._threshold / vGain;                            //V -> ADC Bytes
        const double fraction   = param._fraction;

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
            linearRegression<T>(3,xslope,yslope,mslope,cslope);

            //--find x with a cubic polynomial interpolation between four points--//
            //--do this with the Newtons interpolation Polynomial--//
            const double x[4] = {i-1,i,i+1,i+2};          //x vector
            const double y[4] = {fsx_m1,fsx,fsx_1,fsx_2}; //y vector
            double coeff[4] = {0,0,0,0};                  //Newton coeff vector
            createNewtonPolynomial<T>(x,y,coeff);

            //--numericaly solve the Newton Polynomial--//
            //--give the lineare approach for x as Start Value--//
            const double xPoly = cass::ACQIRIS::findXForGivenY<T>(x,coeff,walk,xLin);
            const double pos = xPoly + static_cast<double>(idxToFiPoint) + horpos;

            //--create a new signal--//
            SignalProducer::signal_t signal;

            //add the info//
            signal["time"] = pos*sampleInterval;
            signal["cfd"]  = pos*sampleInterval;
            if (fsx > fsx_1) signal["polarity"]            = Negative;
            if (fsx < fsx_1) signal["polarity"]            = Positive;
            if (fabs(fsx-fsx_1) < std::sqrt(std::numeric_limits<double>::epsilon())) signal["polarity"] = Bad;

            //--start and stop of the puls--//
            startstop<T>(c,signal,param._threshold);

            //--height of peak--//
            maximum<T>(c,signal,param._threshold);

            //--width & fwhm of peak--//
            fwhm<T>(c,signal,param._threshold);

            //--the com and integral--//
            CoM<T>(c,signal,param._threshold);

            //--add peak to signal if it fits the conditions--//
            /** @todo make sure that is works right, since we get back a double */
            if(fabs(signal["polarity"]-param._polarity) < std::sqrt(std::numeric_limits<double>::epsilon()))  //if it has the right polarity
            {
              for (CFDParameters::timeranges_t::const_iterator it (param._timeranges.begin());
              it != param._timeranges.end();
              ++it)
              {
                if(signal["time"] > it->first && signal["time"] < it->second) //if signal is in the right timerange
                {
                  sig.push_back(signal);
                  break;
                }
              }
            }
          }
        }
      }

      void loadSettings(CASSSettings &s,CFDParameters &p, Instruments &instrument, size_t & channelNbr)
      {
        s.beginGroup("ConstantFraction");
        instrument   = static_cast<Instruments>(s.value("AcqirisInstrument",Camp1).toInt());
        channelNbr   = s.value("ChannelNumber",0).toInt();
        p._polarity  = static_cast<Polarity>(s.value("Polarity",Negative).toInt());
        p._threshold = s.value("Threshold",0.05).toDouble();
        p._delay     = s.value("Delay",5).toInt();
        p._fraction  = s.value("Fraction",0.6).toDouble();
        p._walk      = s.value("Walk",0.).toDouble();
        int size = s.beginReadArray("Timeranges");
        for (int i = 0; i < size; ++i)
        {
          p._timeranges.push_back(std::make_pair(s.value("LowerLimit",0.).toDouble(),
                                                 s.value("UpperLimit",1000).toDouble()));
        }
        s.endArray();
        s.endGroup();
      }
    }
  }
}

//########################## 8 Bit Version ###########################################################################
//______________________________________________________________________________________________________________________
SignalProducer::signals_t& CFD8Bit::operator()(SignalProducer::signals_t& sig)
{
  ConstantFraction::cfd<char>(*_chan,_parameters,sig);
  return sig;
}

void CFD8Bit::loadSettings(CASSSettings &s)
{
  ConstantFraction::loadSettings(s,_parameters,_instrument,_chNbr);
}

void CFD8Bit::associate(const CASSEvent &evt)
{
  _chan = extactRightChannel(evt,_instrument,_chNbr);
}

//########################## 16 Bit Version ###########################################################################
//______________________________________________________________________________________________________________________
SignalProducer::signals_t& CFD16Bit::operator()(SignalProducer::signals_t& sig)
{
  ConstantFraction::cfd<short>(*_chan,_parameters,sig);
  return sig;
}

void CFD16Bit::loadSettings(CASSSettings &s)
{
  ConstantFraction::loadSettings(s,_parameters,_instrument,_chNbr);
}

void CFD16Bit::associate(const CASSEvent &evt)
{
  _chan = extactRightChannel(evt,_instrument,_chNbr);
}
