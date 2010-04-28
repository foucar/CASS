//Copyright (C) 2003-2010 Lutz Foucar

#include <iostream>
#include <cmath>
#include <stdexcept>

#include "delayline_detector_analyzer_simple.h"
#include "delayline_detector.h"
#include "channel.h"
#include "waveform_analyzer_backend.h"


/** find the boundaries for sorting.
 * For a given Mcp time there are only a few singal on the wire ends that can
 * come with the Mcp Signal. This function will find the indexs of the list
 * of signals (peaks) which might come together with the mcp signal. This is because
 * we know two things (ie. for the x-layer):
 * \f$|x_1-x_2|<rTime_x\f$
 * and
 * \f$x_1+x_2-2*mcp = ts_x\f$
 * with this knowledge we can calculate the boundries for the anode
 * given the Timesum and the Runtime
 * @return void
 * @param anodeEnd the wire end that we want too check for indizes
 * @param mcp the Mcp Signal for which to find the right wire end signals
 * @param ts The timesum of the Anode
 * @param rTime The runtime of a Signal over the whole wire of the anode
 * @param[out] min the minimum list index that can belong to the mcp signal
 * @param[out] max the maximum list index that can belong to the mcp signal
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
 * Function that will sort the signals of two layers for timesum. When the timesum
 * is fullfilled then we found a hit.
 * @return void
 * @param[in,out] d The detector that we are working on
 * @param[in] anode The anodelayers that we should reconstruct the detectorhits from
 * @author Lutz Foucar
 */
void sortForTimesum(cass::ACQIRIS::DelaylineDetector &d,std::pair<cass::ACQIRIS::AnodeLayer*,cass::ACQIRIS::AnodeLayer*> & anode)
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
  Signal::peaks_t &s2p =  s2.peaks();

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
            if ( (sumy > tsyLow) && (sumy < tsyHigh) )
            //check wether the hit is inside the radius of the MCP//
            if (radius_mm < radius)
            {
              //std::cout << "found hit"<<std::endl;
              //add a DetektorHit to the Detektor
              d.hits().push_back(DelaylineDetectorHit(x_mm,y_mm,mcp));
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



//****************************************The Class Implementation*******************************************************
//___________________________________________________________________________________________________________________________________________________________
void cass::ACQIRIS::DelaylineDetectorAnalyzerSimple::operator()(cass::ACQIRIS::DetectorBackend& detector, const Device& dev)
{
  //std::cout << "DelaylineDetectorAnalyzerSimple: entering"<<std::endl;
  //do a type conversion to have a delayline detector//
  DelaylineDetector &d = dynamic_cast<DelaylineDetector&>(detector);
  //check what layer the user wants to use for calculating the pos//
  std::pair<AnodeLayer*,AnodeLayer*> anode;
  switch (d.delaylineType())
  {
  case Hex :
    {
      switch (d.layersToUse())
      {
      case(UV):
        //std::cout <<"hex det using uv layers"<<std::endl;
        anode = std::make_pair(&d.layers()['U'],&d.layers()['V']);
        break;
      case(UW):
        //std::cout <<"hex det using uw layers"<<std::endl;
        anode = std::make_pair(&d.layers()['U'],&d.layers()['W']);
        break;
      case(VW):
        ///std::cout <<"hex det using vw layers"<<std::endl;
        anode = std::make_pair(&d.layers()['V'],&d.layers()['W']);
      default:
        throw std::invalid_argument("the chosen layer combination does not exist");
        return;
        break;
      }
    }
    break;
  case Quad:
    //std::cout <<"quad det using xy layers"<<std::endl;
    anode = std::make_pair(&d.layers()['X'],&d.layers()['Y']);
    break;
  default:
    throw std::invalid_argument("chosen delaylinetype doesn't exist");
  }

  //extract the peaks for the signals of the detector from the channels//
  //check whether the requested channel does exist//
  //first retrieve the right Instruments / Channels for the signals
  Signal & MCP = d.mcp();
  const Instruments &MCPInstr = MCP.instrument();
  const size_t MCPChanNbr = MCP.channelNbr();
  const WaveformAnalyzers &MCPAnal= MCP.analyzerType();
  Device::instruments_t::const_iterator MCPInstrIt = dev.instruments().find(MCPInstr);
  if (MCPInstrIt == dev.instruments().end())
  {
    std::cerr<< "DelaylineDetectorSimple::the requested Instrument "
        <<MCPInstr<<" for MCP is not in the datastream"<<std::endl;
    return;
  }
  const Instrument::channels_t &MCPInstrChans = MCPInstrIt->second.channels();
  const Channel &MCPChan = MCPInstrChans[MCPChanNbr];


  Signal &F1 = anode.first->wireend()['1'];
  const Instruments &F1Instr = F1.instrument();
  const size_t F1ChanNbr = F1.channelNbr();
  const WaveformAnalyzers &F1Anal= F1.analyzerType();
  Device::instruments_t::const_iterator F1InstrIt = dev.instruments().find(F1Instr);
  if (F1InstrIt == dev.instruments().end())
  {
    std::cerr<< "DelaylineDetectorSimple::the requested Instrument "
        <<F1Instr<<" for First Anode Signal 1"
        <<" is not in the datastream"<<std::endl;
    return;
  }
  const Instrument::channels_t &F1InstrChans = F1InstrIt->second.channels();
  const Channel &F1Chan = F1InstrChans[F1ChanNbr];

  Signal &F2 = anode.first->wireend()['2'];
  const Instruments &F2Instr = F2.instrument();
  const size_t F2ChanNbr = F2.channelNbr();
  const WaveformAnalyzers &F2Anal= F2.analyzerType();
  Device::instruments_t::const_iterator F2InstrIt = dev.instruments().find(F2Instr);
  if (F2InstrIt == dev.instruments().end())
  {
    std::cerr<< "DelaylineDetectorSimple::the requested Instrument "
        <<F1Instr<<" for First Anode Signal 2"
        <<" is not in the datastream"<<std::endl;
    return;
  }
  const Instrument::channels_t &F2InstrChans = F2InstrIt->second.channels();
  const Channel &F2Chan = F2InstrChans[F2ChanNbr];


  Signal &S1 = anode.second->wireend()['1'];
  const Instruments &S1Instr = S1.instrument();
  const size_t S1ChanNbr = S1.channelNbr();
  const WaveformAnalyzers &S1Anal= S1.analyzerType();
  Device::instruments_t::const_iterator S1InstrIt = dev.instruments().find(S1Instr);
  if (S1InstrIt == dev.instruments().end())
  {
    std::cerr<< "DelaylineDetectorSimple::the requested Instrument "
        <<F1Instr<<" for Second Anode Signal 1"
        <<" is not in the datastream"<<std::endl;
    return;
  }
  const Instrument::channels_t &S1InstrChans = S1InstrIt->second.channels();
  const Channel &S1Chan = S1InstrChans[S1ChanNbr];

  Signal &S2 = anode.second->wireend()['2'];
  const Instruments &S2Instr = S2.instrument();
  const size_t S2ChanNbr = S2.channelNbr();
  const WaveformAnalyzers &S2Anal= S2.analyzerType();
  Device::instruments_t::const_iterator S2InstrIt = dev.instruments().find(S2Instr);
  if (S2InstrIt == dev.instruments().end())
  {
    std::cerr<< "DelaylineDetectorSimple::the requested Instrument "
        <<F1Instr<<" for Second Anode Signal 2"
        <<" is not in the datastream"<<std::endl;
    return;
  }
  const Instrument::channels_t &S2InstrChans = S2InstrIt->second.channels();
  const Channel &S2Chan = S2InstrChans[S2ChanNbr];

  //now extract values//
  if ((MCPChanNbr >= MCPInstrChans.size()))
  {
    std::cerr << "DelaylineDetectorAnalyzerSimple: the requested channel for mcp \""
        <<MCPChanNbr<<"\" is not present. We only have \""<<MCPInstrChans.size()<<"\" channels"
        <<" Detector:"<<d.name()
        <<" Instrument:"<<MCPInstr
        <<" Ana type:"<< MCPAnal<<std::endl;
    return;
  }
  (*(*_waveformanalyzer)[MCPAnal])(MCPChan,MCP);

  //check whether the requested channel does exist//
  if ((F1ChanNbr >= F1InstrChans.size()))
  {
    std::cerr << "DelaylineDetectorAnalyzerSimple: the requested channel for first layer one \""
        <<F1ChanNbr<<"\" is not present. We only have \""<<F1InstrChans.size()<<"\" channels"<<std::endl;
    return;
  }
  //  std::cerr<<"waveformanalyzertyp for first layer one "<<firstLayer->wireend()['1'].analyzerType()<<" chnbr:"<<firstLayer->wireend()['1'].channelNbr()<<std::endl;
  (*(*_waveformanalyzer)[F1Anal])(F1Chan,F1);

  //check whether the requested channel does exist//
  if ((F2ChanNbr >= F2InstrChans.size()))
  {
    std::cerr << "DelaylineDetectorAnalyzerSimple: the requested channel for first layer two \""
        <<F2ChanNbr<<"\" is not present. We only have \""<<F2InstrChans.size()<<"\" channels"<<std::endl;
    return;
  }
  //  std::cerr<<"waveformanalyzertyp for first layer two "<<firstLayer->wireend()['2'].analyzerType()<<" chnbr:"<<firstLayer->wireend()['2'].channelNbr()<<std::endl;
  (*(*_waveformanalyzer)[F2Anal])(F2Chan,F2);

  //check whether the requested channel does exist//
  if ((S1ChanNbr >= S1InstrChans.size()))
  {
    std::cerr << "DelaylineDetectorAnalyzerSimple: the requested channel for second layer one \""
        <<S1ChanNbr<<"\" is not present. We only have \""<<S1InstrChans.size()<<"\" channels"<<std::endl;
    return;
  }
  //  std::cerr<<"waveformanalyzertyp for second layer one "<<secondLayer->wireend()['1'].analyzerType()<<" chnbr:"<<secondLayer->wireend()['1'].channelNbr()<<std::endl;
  (*(*_waveformanalyzer)[S1Anal])(S1Chan,S1);

  //check whether the requested channel does exist//
  if ((S2ChanNbr >= S2InstrChans.size()))
  {
    std::cerr << "DelaylineDetectorAnalyzerSimple: the requested channel for second layer two \""
        <<S2ChanNbr<<"\" is not present. We only have \""<<S2InstrChans.size()<<"\" channels"<<std::endl;
    return;
  }
  //  std::cerr<<"waveformanalyzertyp for second layer two "<<secondLayer->wireend()['2'].analyzerType()<<" chnbr:"<<secondLayer->wireend()['2'].channelNbr()<<std::endl;
  (*(*_waveformanalyzer)[S2Anal])(S2Chan,S2);

  ////tell the signals that you have updated it//
  //d.mcp().isNewEvent() = true;
  //firstLayer->one().isNewEvent() = true;
  //firstLayer->two().isNewEvent() = true;
  //secondLayer->one().isNewEvent() = true;
  //secondLayer->two().isNewEvent() = true;

  //now sort these peaks for the layers timesum//
  //  std::cout << "sort for timesum"<<std::endl;
  sortForTimesum(d,anode);
  //std::cout<<"DelaylineDetectorAnalyzerSimple:"<<d.hits().size()<<std::endl;
  //std::cout << "DelaylineDetectorAnalyzerSimple: leaving"<<std::endl;
}
