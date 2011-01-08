//Copyright (C) 2003-2010 Lutz Foucar

/**
 * @file delayline_detector_analyzer_simple.cpp file contains the definition of
 *                                              classes and functions that
 *                                              analyzses a delayline detector.
 *
 * @author Lutz Foucar
 */

#include <iostream>
#include <cmath>
#include <stdexcept>

#include "delayline_detector_analyzer_simple.h"
#include "delayline_detector.h"
#include "channel.h"
#include "waveform_analyzer_backend.h"

namespace cass
{
  namespace DelaylineDetectorAnalyzers
  {
    /** extract the signals of a wavefrom
     *
     * will extract all signals of the indicated wavefrom. The describtion which
     * wavefrom to analyze and how to analyze it is described in signal. We need
     * to know in which instrument and channel within the instrument the
     * waveform that contains the detector signals is. Also we need to know how
     * the wavefrom should be analyzed. After extracting this information it
     * checks whether the requested instrument is present in the datastream and
     * also whether the requested channel within this instrument exists.
     *
     * @param signal describtion where the to be analyzed waveform is
     * @param device the device that contains the waveforms
     * @param wavefromanalyzers the container for all wavefromanalyzers
     */
    void extractSignals(ACQIRIS::Signal& signal,
                        const ACQIRIS::Device& device,
                        ACQIRIS::DetectorAnalyzerBackend::waveformanalyzers_t& waveformanalyzers)
    {
      using namespace ACQIRIS;
      const Instruments &instrument (signal.instrument());
      ACQIRIS::Device::instruments_t::const_iterator instrumentIt
          (device.instruments().find(instrument));
      if (instrumentIt == device.instruments().end())
      {
        /** @todo instead of making an output to the console throw an error,
         *        which should be treated right
         */
        std::cerr<< "DelaylineDetectorSimple::the requested Instrument "
            <<instrument<<" for signal is not in the datastream"<<std::endl;
        return;
      }
      const size_t ChannelNumber (signal.channelNbr());
      const Instrument::channels_t &instrumentChannels (instrumentIt->second.channels());
      if ((ChannelNumber >= instrumentChannels.size()))
      {
        /** @todo instead of making an output to the console throw an error,
         *        which should be treated right
         */
        std::cerr << "DelaylineDetectorAnalyzerSimple: the requested channel \""
            <<ChannelNumber<<"\" is not present. We only have \""
            <<instrumentChannels.size()<<"\" channels."
            <<" Instrument:"<<instrument
            <<std::endl;
        return;
      }
      const Channel &channel (instrumentChannels[ChannelNumber]);
      const WaveformAnalyzers &analyzertype (signal.analyzerType());
      (*waveformanalyzers[analyzertype])(channel,signal);
    }

    /** find the boundaries for sorting.
     *
     * For a given Mcp time there are only a few singal on the wire ends that can
     * come with the Mcp Signal. This function will find the indexs of the list
     * of signals (peaks) which might come together with the mcp signal. This is because
     * we know two things (ie. for the x-layer):
     * \f$|x_1-x_2|<rTime_x\f$
     * and
     * \f$x_1+x_2-2*mcp = ts_x\f$
     * with this knowledge we can calculate the boundries for the anode
     * given the Timesum and the Runtime
     *
     * @return void
     * @param anodeEnd the wire end that we want too check for indizes
     * @param mcp the Mcp Signal for which to find the right wire end signals
     * @param ts The timesum of the Anode
     * @param rTime The runtime of a Signal over the whole wire of the anode
     * @param[out] min the minimum list index that can belong to the mcp signal
     * @param[out] max the maximum list index that can belong to the mcp signal
     *
     * @author Lutz Foucar
     */
    void findBoundriesForSorting(const cass::ACQIRIS::Signal &anodeEnd, const double mcp, const double ts, const double rTime, int &min, int &max)
    {
      //set min and max to 0//
      min = -2;
      max = -1;

      //--find useful boundries where to search for good timesums--//
      for (size_t i=0;i<anodeEnd.peaks().size();++i)
      {
        //check wether the current anode value will fall into the boundries
        if ( fabs(2.*anodeEnd.peaks()[i].time() - 2.*mcp - ts) <= rTime )
        {
          //if the min value is not set, set it now
          if(min == -2) min = i;
          //set the max value (the last time this will be set is if we are still inside the boundries
          max = i;
        }
        //if the min value has been set it means that we are now outside the boundries => quit here
        else if (min != -2)
          break;
      }

      if (min == -2) min = 0;
    }

    /** timesum sorter.
     *
     * Function that will sort the signals of two layers for timesum. When the timesum
     * is fullfilled then we found a hit.
     *
     * @return void
     * @param[in,out] d The detector that we are working on
     * @param[in] anode The anodelayers that we should reconstruct the detectorhits from
     *
     * @author Lutz Foucar
     */
    void sortForTimesum(cass::ACQIRIS::DelaylineDetector &d,
                        std::pair<cass::ACQIRIS::AnodeLayer*,cass::ACQIRIS::AnodeLayer*> & anode)
    {
      using namespace cass::ACQIRIS;
      //--calculate the timesum from the given lower and upper boundries for it--//
      AnodeLayer &f         = *(anode.first);
      AnodeLayer &s         = *(anode.second);
      Signal &f1            = f.wireend()['1'];
      Signal &f2            = f.wireend()['2'];
      Signal &s1            = s.wireend()['1'];
      Signal &s2            = s.wireend()['2'];
      const double tsx      = f.ts();
      const double tsy      = s.ts();
      const double runttime	= d.runtime();
      const double tsxLow		= f.tsLow();
      const double tsxHigh	= f.tsHigh();
      const double tsyLow		= s.tsLow();
      const double tsyHigh	= s.tsHigh();
      const double radius		= d.mcpRadius();
      Signal::peaks_t &mcpp = d.mcp().peaks();
      Signal::peaks_t &f1p  = f1.peaks();
      Signal::peaks_t &f2p  = f2.peaks();
      Signal::peaks_t &s1p  = s1.peaks();
      Signal::peaks_t &s2p  = s2.peaks();
      const double angle    = d.angle();

      //  std::cout <<"mcp: "<<mcpp.size() <<" ";
      //  std::cout <<"f1: "<<f1.peaks().size() <<" ";
      //  std::cout <<"f2: "<<f2.peaks().size() <<" ";
      //  std::cout <<"s1: "<<s1.peaks().size() <<" ";
      //  std::cout <<"s2: "<<s2.peaks().size() <<std::endl;

      for (size_t iMcp=0;iMcp<mcpp.size();++iMcp)
      {
        //    std::cout << mcpp[iMcp]->isUsed()<<std::endl;
        if (mcpp[iMcp].isUsed()) continue;
        //--find the right indizes, only look in the right timerange--//
        const double mcp = mcpp[iMcp].time();
        int iX1min,iX1max,iX2min,iX2max,iY1min,iY1max,iY2min,iY2max;
        findBoundriesForSorting(f1,mcp,tsx,runttime,iX1min,iX1max);
        findBoundriesForSorting(f2,mcp,tsx,runttime,iX2min,iX2max);
        findBoundriesForSorting(s1,mcp,tsy,runttime,iY1min,iY1max);
        findBoundriesForSorting(s2,mcp,tsy,runttime,iY2min,iY2max);

        //    std::cout <<iMcp <<" ";
        //    std::cout <<" x1low:"<<iX1min <<" x1high:"<<iX1max;
        //    std::cout <<" x2low:"<<iX2min <<" x2high:"<<iX2max;
        //    std::cout <<" y1low:"<<iY1min <<" y1high:"<<iY1max;
        //    std::cout <<" y2low:"<<iY2min <<" y2high:"<<iY2max;
        //    std::cout <<std::endl;

        //go through all possible combinations//
        for (int iX1=iX1min;iX1<=iX1max;++iX1)
        {
          if (f1p[iX1].isUsed()) continue;
          for (int iX2=iX2min;iX2<=iX2max;++iX2)
          {
            if (f2p[iX2].isUsed()) continue;
            for (int iY1=iY1min;iY1<=iY1max;++iY1)
            {
              if (s1p[iY1].isUsed()) continue;
              for (int iY2=iY2min;iY2<=iY2max;++iY2)
              {
                if (s2p[iY2].isUsed()) continue;

                //std::cout <<"checking timesum condition for combination "<< iX1<<","<< iX2<<","<< iY1<<","<< iY2<<","<< iMcp<<","<<std::endl;
                //calc the timesum//
                const double x1 = f1p[iX1].time();
                const double x2 = f2p[iX2].time();
                const double y1 = s1p[iY1].time();
                const double y2 = s2p[iY2].time();
                const double sumx = x1+x2 - 2.* mcp;
                const double sumy = y1+y2 - 2.* mcp;

                //calc pos and radius//
                const double xlay_mm = (x1-x2) * f.sf();
                const double ylay_mm = (y1-y2) * s.sf();
                double x_mm=xlay_mm;
                double y_mm=ylay_mm;
                if (d.delaylineType() == Hex)
                {
                  switch (d.layersToUse())
                  {
                  case(UV):
                    {
                      y_mm = 1/std::sqrt(3.) * (xlay_mm - 2.*ylay_mm);
                      break;
                    }
                  case(UW):
                    {
                      y_mm = 1/std::sqrt(3.) * (2.*ylay_mm - xlay_mm);
                      break;
                    }
                  case(VW):
                    {
                      x_mm = xlay_mm+ylay_mm;
                      y_mm = 1/std::sqrt(3.) * (ylay_mm - xlay_mm);
                      break;
                    }
                  default: break;
                  }
                }

                const double radius_mm = sqrt(x_mm*x_mm + y_mm*y_mm);

                //check wether the timesum is correct//
                if ( (sumx > tsxLow) && (sumx < tsxHigh) )
                {
                  if ( (sumy > tsyLow) && (sumy < tsyHigh) )
                  {
                    //check wether the hit is inside the radius of the MCP//
                    if (radius_mm < radius)
                    {
                      //rotate x and y with angle
                      const double rot_x_mm (x_mm * std::cos(angle) - y_mm * std::sin(angle));
                      const double rot_y_mm (x_mm * std::sin(angle) + y_mm * std::cos(angle));
                      //add a DetektorHit to the Detektor
                      d.hits().push_back(DelaylineDetectorHit(rot_x_mm,rot_y_mm,mcp));
                      //remember that this mcp Peak has already been used//
                      mcpp[iMcp].isUsed()    = true;
                      f1p[iX1].isUsed() = true;
                      f2p[iX2].isUsed() = true;
                      s1p[iY1].isUsed() = true;
                      s2p[iY2].isUsed() = true;
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
  }
}


//****************************************The Class Implementation*******************************************************
//___________________________________________________________________________________________________________________________________________________________
void cass::ACQIRIS::DelaylineDetectorAnalyzerSimple::operator()(cass::ACQIRIS::DetectorBackend& detector, const Device& dev)
{
  //std::cout << "DelaylineDetectorAnalyzerSimple: entering"<<std::endl;
  //do a type conversion to have a delayline detector//
  DelaylineDetector &d = dynamic_cast<DelaylineDetector&>(detector);
  //check what layer the user wants to use for calculating the pos//
  std::pair<AnodeLayer*,AnodeLayer*> anode
      (std::make_pair(&d.layers()[_usedLayers.first],&d.layers()[_usedLayers.second]));

//  switch (d.delaylineType())
//  {
//  case Hex :
//    {
//      switch (d.layersToUse())
//      {
//      case(UV):
//        //std::cout <<"hex det using uv layers"<<std::endl;
//        anode = std::make_pair(&d.layers()['U'],&d.layers()['V']);
//        break;
//      case(UW):
//        //std::cout <<"hex det using uw layers"<<std::endl;
//        anode = std::make_pair(&d.layers()['U'],&d.layers()['W']);
//        break;
//      case(VW):
//        ///std::cout <<"hex det using vw layers"<<std::endl;
//        anode = std::make_pair(&d.layers()['V'],&d.layers()['W']);
//      default:
//        throw std::invalid_argument("the chosen layer combination does not exist");
//        return;
//        break;
//      }
//    }
//    break;
//  case Quad:
//    //std::cout <<"quad det using xy layers"<<std::endl;
//    anode = std::make_pair(&d.layers()['X'],&d.layers()['Y']);
//    break;
//  default:
//    throw std::invalid_argument("chosen delaylinetype doesn't exist");
//  }

  //extract the peaks for the signals of the detector from the channels//
  //check whether the requested channel does exist//
  //first retrieve the right Instruments / Channels for the signals
  Signal & MCP (d.mcp());
  DelaylineDetectorAnalyzers::extractSignals(MCP,dev,*_waveformanalyzer);

  Signal &F1 (anode.first->wireend()['1']);
  DelaylineDetectorAnalyzers::extractSignals(F1,dev,*_waveformanalyzer);

  Signal &F2 (anode.first->wireend()['2']);
  DelaylineDetectorAnalyzers::extractSignals(F2,dev,*_waveformanalyzer);

  Signal &S1 = anode.second->wireend()['1'];
  DelaylineDetectorAnalyzers::extractSignals(S1,dev,*_waveformanalyzer);

  Signal &S2 = anode.second->wireend()['2'];
  DelaylineDetectorAnalyzers::extractSignals(S2,dev,*_waveformanalyzer);

  ////tell the signals that you have updated it//
  //d.mcp().isNewEvent() = true;
  //firstLayer->one().isNewEvent() = true;
  //firstLayer->two().isNewEvent() = true;
  //secondLayer->one().isNewEvent() = true;
  //secondLayer->two().isNewEvent() = true;

  //now sort these peaks for the layers timesum//
  //  std::cout << "sort for timesum"<<std::endl;
  DelaylineDetectorAnalyzers::sortForTimesum(d,anode);
  //std::cout <<"DelaylineDetectorAnalyzerSimple:"<<d.hits().size()<<std::endl;
  //std::cout << "DelaylineDetectorAnalyzerSimple: leaving"<<std::endl;
}
