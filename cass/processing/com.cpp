//Copyright (C) 2003-2010 Lutz Foucar

/**
 * @file com.cpp file contains defintion of class that does a center of mass
 *               analysis of a waveform
 *
 * @author Lutz Foucar
 */

#include <typeinfo>
#include <limits>

#include "com.h"

#include "helperfunctionsforstdc.hpp"
#include "channel.hpp"
#include "cass_settings.h"
#include "acqiris_device.hpp"
#include "cass_event.h"

using namespace cass::ACQIRIS;
using namespace std;

namespace cass
{
namespace ACQIRIS
{
namespace CenterOfMass
{
/** Implementation of Center of Mass
 *
 * go through the puls and find all points above a given threshold.
 * Calculate the center of mass of all the points above the threshold, which
 * is treated as the time of the identified peak.
 *
 * @todo maybe just find the rising slope and let all other things be done
 *       by helper functions. (starting with findstartstop)
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
void getcom(const Channel& c, const CoMParameters &param, SignalProducer::signals_t& sig)
{
  //make sure that we are the right one for the waveform_t//
  assert(typeid(waveform_t::value_type) == typeid(T));
  //extract infos from channel//
  const Channel::waveform_t &Data (c.waveform());
  const int32_t vOffset (static_cast<int32_t>(c.offset() / c.gain()));    //mV -> ADC Bytes
  const size_t wLength (Data.size());
  //extract info how to analyse from signal
  double thres (param._threshold);
  const double threshold (thres / c.gain());    //mV -> ADC Bytes

  //initialize values for finding peaks//
  bool risingEdge         = false;
  bool firsttime          = true;
  int32_t startposval     = -1;

  //--go through the waveform--//
  for (size_t i=3; i<wLength;++i)
  {
    //check wether we have an indication of a peak in the signal//
    if (   (abs(Data[i] - vOffset) <= threshold)                  //checking for inside noise
           || ( i == wLength-1)                                        //check if we are at the second to last index
           || ( ( (Data[i]-vOffset)*(Data[i-1]-vOffset) ) < 0.))       //check wether we go through the zero
    {
      if (risingEdge)            //if we had a rising edge before we know that it was a real peak
      {
        //--create a new signal--//
        SignalProducer::signal_t signal(NbrSignalDefinitions,0);

        //--set all known settings--//
        signal[startpos] = startposval;
        signal[stoppos]  = i-1;

        //--height stuff--//
        getmaximum<T>(c,signal,param._threshold);

        //--fwhm stuff--//
        getfwhm<T>(c,signal,param._threshold);

        //--center of mass stuff--//
        CoM<T>(c,signal,param._threshold);

        //--Time is the Center of Mass--//
        signal[time] =
            signal[com];

        //--check the polarity--//
        if (Data[static_cast<int>(signal[maxpos]+0.1)]-vOffset == signal[maximum])       //positive
          signal[polarity] = Positive;
        else if (Data[static_cast<int>(signal[maxpos]+0.1)]-vOffset == -signal[maximum]) //negative
          signal[polarity] = Negative;
        else
        {
          cout << "error: polarity not found"<<endl;
          signal[polarity] = Bad;
        }
        //--add peak to signal if it fits the conditions--//
        if(fabs(signal[polarity]-param._polarity) < std::sqrt(std::numeric_limits<double>::epsilon()))  //if it has the right polarity
        {
          for (CoMParameters::timeranges_t::const_iterator it (param._timeranges.begin());
               it != param._timeranges.end();
               ++it)
          {
            if(signal[time] > it->first && signal[time] < it->second) //if signal is in the right timerange
            {
              signal[isUsed] = false;
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
        startposval= i;            //remember the position
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
}

/** implementation of loading settings for both Center of Mass classes
 *
 * this function implements the retrieval of the settings for the CoM
 * signal extractors. For a description on the settings see decription of
 * CoM8Bit class.\n
 * It opens the group "CenterOfMass" and retrieves the settings for the
 * Center of Mass algorithm from the CASSSettings object.
 *
 * @param[in] s the CASSSettings object we retrieve the information from
 * @param[out] p the container for the Center of Mass Parameters
 * @param [out] instrument the instrument that contains the channel the
 *              constant fraction signal extractor should anlyze.
 * @param [out] channelNbr the channel number of the channel that conatins
 *              the singals that this extractor should extract.
 *
 * @author Lutz Foucar
 */
void loadSettings(CASSSettings &s,CoMParameters &p, uint32_t &instrument, size_t & channelNbr)
{
  s.beginGroup("CenterOfMass");
  instrument   = s.value("AcqirisInstrument",0).toUInt();
  channelNbr   = s.value("ChannelNumber",0).toInt();
  p._polarity  = static_cast<Polarity>(s.value("Polarity",Negative).toInt());
  p._threshold = fabs(s.value("Threshold",0.05).toDouble());
  int size = s.beginReadArray("Timeranges");
  for (int i = 0; i < size; ++i)
  {
    s.setArrayIndex(i);
    p._timeranges.push_back(std::make_pair(s.value("LowerLimit",0.).toDouble(),
                                           s.value("UpperLimit",1000).toDouble()));
  }
  s.endArray();
  s.endGroup();
}

}//end namespace centerofmass
}//end namespace acqiris
}//end namespace cass

//########################## 8 Bit Version ###########################################################################
//______________________________________________________________________________________________________________________
SignalProducer::signals_t& CoM8Bit::operator()(SignalProducer::signals_t& sig)
{
  CenterOfMass::getcom<char>(*_chan, _parameters, sig);
  return sig;
}

void CoM8Bit::loadSettings(CASSSettings &s)
{
  CenterOfMass::loadSettings(s,_parameters,_instrument,_chNbr);
}

void CoM8Bit::associate(const CASSEvent &evt)
{
  _chan = extactRightChannel<char>(evt,_instrument,_chNbr);
}

//########################## 16 Bit Version ###########################################################################
//______________________________________________________________________________________________________________________
SignalProducer::signals_t& CoM16Bit::operator()(SignalProducer::signals_t& sig)
{
  CenterOfMass::getcom<short>(*_chan, _parameters, sig);
  return sig;
}

void CoM16Bit::loadSettings(CASSSettings &s)
{
  CenterOfMass::loadSettings(s,_parameters,_instrument,_chNbr);
}

void CoM16Bit::associate(const CASSEvent &evt)
{
  _chan = extactRightChannel<short>(evt,_instrument,_chNbr);
}
