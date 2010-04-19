//Copyright (C) 2010 lmf
#include "com.h"

#include "helperfunctionsforstdc.h"
#include "channel.h"
#include "waveform_signal.h"


//______________________Implementation of simple Version__________________________________________________________
template <typename T>
    void com(const cass::ACQIRIS::Channel& c, cass::ACQIRIS::ResultsBackend& result)
{
//  std::cout<<"com"<<sizeof(T)<<": entering"<<std::endl;
  //get reference to the signal//
  cass::ACQIRIS::Signal &s = dynamic_cast<cass::ACQIRIS::Signal&>(result);
  //extract infos from channel//
  const cass::ACQIRIS::Channel::waveform_t Data = c.waveform();
  const int32_t vOffset   = static_cast<int32_t>(c.offset() / c.gain());    //mV -> ADC Bytes
  const size_t wLength    = c.waveform().size();
  //extract info how to analyse from signal
  const double threshold  = s.threshold() / c.gain();    //mV -> ADC Bytes
  //    std::cout << "in waveformanalyzer threshold of channel "<<c.channelNbr()<<" is "<<c.threshold()<<" "<<c.offset()<<std::endl;

  //initialize values for finding peaks//
  bool risingEdge         = false;
  bool firsttime          = true;
  int32_t startpos        = -1;

  //--go through the waveform--//
  for (size_t i=3; i<wLength;++i)
  {
    //      if (c.channelNbr()== 17)std::cout <<i<<" "<< Data[i]*c.gain() << " " <<(Data[i]-vOffset)*c.gain()<<std::endl;
    //check wether we have an indication of a peak in the signal//
    if (   (abs(Data[i] - vOffset) <= threshold)                    //checking for inside noise
      || ( i == wLength-1)                                        //check if we are at the second to last index
      || ( ( (Data[i]-vOffset)*(Data[i-1]-vOffset) ) < 0.))       //check wether we go through the zero
      {
      if (risingEdge)            //if we had a rising edge before we know that it was a real peak
      {
        //--create a new peak--//
        cass::ACQIRIS::Peak p;
        //                std::cout << "usedflag at start "<<p.isUsed()<<std::endl;
        //--set all known settings--//
        p.startpos() = startpos;
        p.stoppos()  = i-1;

        //--height stuff--//
        cass::ACQIRIS::maximum<T>(c,p);

        //--fwhm stuff--//
        cass::ACQIRIS::fwhm<T>(c,p);

        //--center of mass stuff--//
        cass::ACQIRIS::CoM<T>(c,p,static_cast<const int32_t>(threshold));

        //--Time is the Center of Mass--//
        p.time()= p.com();

        //--check the polarity--//
        if (Data[p.maxpos()]-vOffset == p.maximum())       //positive
          p.polarity() = cass::ACQIRIS::Positive;
        else if (Data[p.maxpos()]-vOffset == -p.maximum()) //negative
          p.polarity() = cass::ACQIRIS::Negative;
        else
        {
          std::cout << "error: polarity not found"<<std::endl;
          p.polarity() = cass::ACQIRIS::Bad;
        }
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
//  std::cout<<"com"<<sizeof(T)<<": leaving"<<std::endl;

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
