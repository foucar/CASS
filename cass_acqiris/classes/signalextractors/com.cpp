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
#include "cass_settings.h"
#include "acqiris_device.h"
#include "cass_event.h"

using namespace cass::ACQIRIS;

namespace cass
{
  namespace ACQIRIS
  {
    namespace CenterOfMass
    {
      //______________________Implementation of simple Version__________________________________________________________
      template <typename T>
          void com(const Channel& c, const CoMParameters &param, SignalProducer::signals_t& sig)
      {
        using namespace cass::ACQIRIS;
        //  std::cout<<"com"<<sizeof(T)*8<<": entering"<<std::endl;
        //make sure that we are the right one for the waveform_t//
        assert(typeid(waveform_t::value_type) == typeid(T));
        //extract infos from channel//
        const waveform_t &Data (c.waveform());
        const int32_t vOffset (static_cast<int32_t>(c.offset() / c.gain()));    //mV -> ADC Bytes
        const size_t wLength (Data.size());
        //extract info how to analyse from signal
        double thres (param._threshold);
        const double threshold (thres / c.gain());    //mV -> ADC Bytes
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
              SignalProducer::signal_t signal;
              //std::cout << "usedflag at start "<<p.isUsed()<<std::endl;
              //--set all known settings--//
              signal["startpos"] = startpos;
              signal["stoppos"]  = i-1;

//              //--height stuff--//
//              maximum<T>(c,p);
//
//              //--fwhm stuff--//
//              fwhm<T>(c,p);
//
//              //--center of mass stuff--//
//              CoM<T>(c,p,static_cast<const int32_t>(threshold));

              //--Time is the Center of Mass--//
              signal["time"] = signal["com"];

              //--check the polarity--//
              if (Data[signal["maxpos"]]-vOffset == signal["maximum"])       //positive
                signal["polarity"] = Positive;
              else if (Data[signal["maxpos"]]-vOffset == -signal["maximum"]) //negative
                signal["polarity"] = Negative;
              else
              {
                std::cout << "error: polarity not found"<<std::endl;
                signal["polarity"] = Bad;
              }
              //        std::cout<<"com: found peak has polarity"<<p.polarity()
              //            <<" should have polarity "<<s.polarity()<<std::endl;
              //--add peak to signal if it fits the conditions--//
              if(signal["polarity"] == param._polarity)  //if it has the right polarity
              {
                for (CoMParameters::timeranges_t::const_iterator it (param._timeranges.begin());
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

      void loadSettings(CASSSettings &s,CoMParameters &p, Instruments &instrument, size_t & channelNbr)
      {
        s.beginGroup("CenterOfMass");
        instrument   = static_cast<Instruments>(s.value("AcqirisInstrument",Camp1).toInt());
        channelNbr   = s.value("ChannelNumber",0).toInt();
        p._polarity  = static_cast<Polarity>(s.value("Polarity",Negative).toInt());
        p._threshold = s.value("Threshold",0.05).toDouble();
        int size = s.beginReadArray("Timeranges");
        for (int i = 0; i < size; ++i)
        {
          p._timeranges.push_back(std::make_pair(s.value("LowerLimit",0.).toDouble(),
                                                 s.value("UpperLimit",1000).toDouble()));
        }
        s.endArray();
        s.endGroup();
      }

      const Channel* extactRightChannel(const CASSEvent &evt, Instruments instrument, size_t ChannelNumber)
      {
        const Device &device
            (*(dynamic_cast<const ACQIRIS::Device*>(evt.devices().find(CASSEvent::Acqiris)->second)));
        ACQIRIS::Device::instruments_t::const_iterator instrumentIt
            (device.instruments().find(instrument));
        if (instrumentIt == device.instruments().end())
          throw std::invalid_argument("extactRightChannel::the requested Instrument for signal is not in the datastream");
        const ACQIRIS::Instrument::channels_t &instrumentChannels
            (instrumentIt->second.channels());
        if ((ChannelNumber >= instrumentChannels.size()))
          throw std::invalid_argument("DelaylineDetectorAnalyzerSimple: the requested channel is not present.");
        return &(instrumentChannels[ChannelNumber]);
      }
    }
  }
}

//########################## 8 Bit Version ###########################################################################
//______________________________________________________________________________________________________________________
SignalProducer::signals_t& CoM8Bit::operator()(SignalProducer::signals_t& sig)
{
  CenterOfMass::com<char>(*_chan, _parameters, sig);
  return sig;
}

void CoM8Bit::loadSettings(CASSSettings &s)
{
  CenterOfMass::loadSettings(s,_parameters,_instrument,_chNbr);
}

void CoM8Bit::associate(const CASSEvent &evt)
{
  _chan = CenterOfMass::extactRightChannel(evt,_instrument,_chNbr);
}

//########################## 16 Bit Version ###########################################################################
//______________________________________________________________________________________________________________________
SignalProducer::signals_t& CoM16Bit::operator()(SignalProducer::signals_t& sig)
{
  CenterOfMass::com<short>(*_chan, _parameters, sig);
  return sig;
}

void CoM16Bit::loadSettings(CASSSettings &s)
{
  CenterOfMass::loadSettings(s,_parameters,_instrument,_chNbr);
}

void CoM16Bit::associate(const CASSEvent &evt)
{
  _chan = CenterOfMass::extactRightChannel(evt,_instrument,_chNbr);
}
