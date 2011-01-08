//Copyright (C) 2003-2010 Lutz Foucar

/**
 * @file com.cpp file contains defintion of class that does a center of mass
 *               analysis of a waveform
 *
 * @author Lutz Foucar
 */

#include <typeinfo>

#include "com.h"

#include "helperfunctionsforstdc.h"
#include "channel.h"
#include "signal_producer.h"


//______________________Implementation of simple Version__________________________________________________________
template <typename T>
    void com(const cass::ACQIRIS::Channel& c, cass::ACQIRIS::ResultsBackend& result)
{
  using namespace cass::ACQIRIS;
//  std::cout<<"com"<<sizeof(T)*8<<": entering"<<std::endl;
  //make sure that we are the right one for the waveform_t//
  assert(typeid(waveform_t::value_type) == typeid(T));
  //get reference to the signal//
  SignalProducer &s = dynamic_cast<SignalProducer&>(result);
  //extract infos from channel//
  const waveform_t Data = c.waveform();
  const int32_t vOffset   = static_cast<int32_t>(c.offset() / c.gain());    //mV -> ADC Bytes
  const size_t wLength    = c.waveform().size();
  //extract info how to analyse from signal
  double thres = s.threshold();
  const double threshold  = thres / c.gain();    //mV -> ADC Bytes
//  std::cout << "com: values of channel "<<c.channelNbr()<<" is "
//      <<s.threshold()<<" "
//      <<c.gain()<<" "
//      <<10/2e-5<<" "
//      <<threshold<<" "
//      <<c.fullscale()<<" "
//      <<vOffset<<" "
//      <<c.offset()<<" "
//      <<wLength<<" "
//      <<std::endl;

  //initialize values for finding peaks//
  bool risingEdge         = false;
  bool firsttime          = true;
  int32_t startpos        = -1;

  //--go through the waveform--//
  for (size_t i=3; i<wLength;++i)
  {
    //std::cout <<i<<" "<< Data[i]*c.gain() << " " <<(Data[i]-vOffset)*c.gain()<<std::endl;
    //std::cout <<i<<" "<<(Data[i]-vOffset)<<" "<<threshold<<std::endl;
    //check wether we have an indication of a peak in the signal//
    if (   (abs(Data[i] - vOffset) <= threshold)                  //checking for inside noise
      || ( i == wLength-1)                                        //check if we are at the second to last index
      || ( ( (Data[i]-vOffset)*(Data[i-1]-vOffset) ) < 0.))       //check wether we go through the zero
      {
      if (risingEdge)            //if we had a rising edge before we know that it was a real peak
      {
        //--create a new peak--//
        Peak p;
        //std::cout << "usedflag at start "<<p.isUsed()<<std::endl;
        //--set all known settings--//
        p.startpos() = startpos;
        p.stoppos()  = i-1;

        //--height stuff--//
        maximum<T>(c,p);

        //--fwhm stuff--//
        fwhm<T>(c,p);

        //--center of mass stuff--//
        CoM<T>(c,p,static_cast<const int32_t>(threshold));

        //--Time is the Center of Mass--//
        p.time()= p.com();

        //--check the polarity--//
        if (Data[p.maxpos()]-vOffset == p.maximum())       //positive
          p.polarity() = Positive;
        else if (Data[p.maxpos()]-vOffset == -p.maximum()) //negative
          p.polarity() = Negative;
        else
        {
          std::cout << "error: polarity not found"<<std::endl;
          p.polarity() = Bad;
        }
//        std::cout<<"com: found peak has polarity"<<p.polarity()
//            <<" should have polarity "<<s.polarity()<<std::endl;
        //--add peak to signal if it fits the conditions--//
        if(p.polarity() == s.polarity())  //if it has the right polarity
          if(p.time() > s.trLow() && p.time() < s.trHigh()) //if signal is in the right timerange
            s.peaks().push_back(p);
      }
      risingEdge = false;
      firsttime=true;
    }
    //if the above is not true then we are outside the noise
    else
    {
      if(firsttime)    //if it is the firsttime we are outside the noise
      {
        firsttime = false;
        startpos= i;            //remember the position
      }

      //--if haven't found a risingEdge (3 consecutive higher points) before check if we have--//
      if (!risingEdge)
      {
        if ((abs(Data[i-3]-vOffset) < abs(Data[i-2]-vOffset)) &&
            (abs(Data[i-2]-vOffset) < abs(Data[i-1]-vOffset)) &&
            (abs(Data[i-1]-vOffset) < abs(Data[i  ]-vOffset)) )
        {
          risingEdge=true;
        }
      }
    }
  }
//  std::cout<<"com"<<sizeof(T)*8<<": leaving"<<std::endl;

}



//########################## 8 Bit Version ###########################################################################
//______________________________________________________________________________________________________________________
void cass::ACQIRIS::CoM8Bit::operator()(const cass::ACQIRIS::Channel& c, cass::ACQIRIS::ResultsBackend& r)
{
  com<char>(c,r);
}
//########################## 16 Bit Version ###########################################################################
//______________________________________________________________________________________________________________________
void cass::ACQIRIS::CoM16Bit::operator()(const cass::ACQIRIS::Channel& c, cass::ACQIRIS::ResultsBackend& r)
{
  com<short>(c,r);
}
