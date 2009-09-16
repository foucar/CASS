#include <iostream>

#include "softtdc_com.h"
#include "helperfunctionsforstdc.h"
#include "channel.h"
#include "peak.h"


//______________________Implimentation of simple Version__________________________________________________________
template <typename T>
void com(cass::REMI::REMIEvent& e)
{
    //go through all channels of the event//
    for (size_t iChan=0;iChan<e.nbrOfChannels();++iChan)
    {
        //extract info from the event//
        cass::REMI::Channel &c  = e.channel(iChan);
        const T *Data           = static_cast<const T*>(c.waveform());
        const long vOffset      = static_cast<long>(c.vertOffset() / c.vertGain());    //mV -> ADC Bytes
        const size_t wLength    = c.waveformLength();
        const double threshold  = c.threshold() / c.vertGain();    //mV -> ADC Bytes

        //initialize values for finding peaks//
        bool risingEdge         = false;
        bool firsttime          = true;
        long startpos           = -1;

        //--go through the waveform--//
        for (size_t i=3; i<wLength;++i)
        {
            //check wether we have an indication of a peak in the signal//
            if (   (abs(Data[i] - vOffset) <= threshold)                    //checking for inside noise
                || ( i == wLength-1)                                        //check if we are at the second to last index
                || ( ( (Data[i]-vOffset)*(Data[i-1]-vOffset) ) < 0.))       //check wether we go through the zero
                {
                if (risingEdge)            //if we had a rising edge before we know that it was a real peak
                {
                    //--add a new peak to the puls--//
                    cass::REMI::Peak &p = c.addPeak();
                    //--set all known settings--//
                    p.startpos(startpos);
                    p.stoppos(i-1);

                    //--height stuff--//
                    cass::REMI::maximum<T>(c,p);

                    //--fwhm stuff--//
                    cass::REMI::fwhm<T>(c,p);

                    //--center of mass stuff--//
                    cass::REMI::CoM<T>(e,c,p);

                    //--Time is the Center of Mass--//
                    p.time(p.com());

                    //--check the polarity--//
                    if (Data[p.maxpos()]-vOffset == p.maximum())       //positive
                        p.polarity(cass::REMI::Peak::kPositive);
                    else if (Data[p.maxpos()]-vOffset == -p.maximum()) //negative
                        p.polarity(cass::REMI::Peak::kNegative);
                    else                                               //error: polarity not found
                        p.polarity(cass::REMI::Peak::kBad);
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
    }
}



//########################## 8 Bit Version ###########################################################################
//______________________________________________________________________________________________________________________
void cass::REMI::SoftTDCCoM8Bit::FindPeaksIn(cass::REMI::REMIEvent& e)
{
    com<char>(e);
}
//########################## 16 Bit Version ###########################################################################
//______________________________________________________________________________________________________________________
void cass::REMI::SoftTDCCoM16Bit::FindPeaksIn(cass::REMI::REMIEvent& e)
{
    com<short>(e);
}
