#include <iostream>
#include <cmath>
#include <stdexcept>

#include "delayline_detector_analyzer_simple.h"
#include "delayline_detector.h"
#include "channel.h"
#include "waveform_analyzer_backend.h"


//****************************************The Class Implementation*******************************************************
//___________________________________________________________________________________________________________________________________________________________
void cass::ACQIRIS::DelaylineDetectorAnalyzerSimple::analyze(cass::ACQIRIS::DetectorBackend& detector, const Device& dev)
{
  //std::cout << "do the sorting"<<std::endl;
  //do a type conversion to have a delayline detector//
  DelaylineDetector &d = dynamic_cast<DelaylineDetector&>(detector);
  //check what layer the user wants to use for calculating the pos//
  AnodeLayer *firstLayer=0;
  AnodeLayer *secondLayer=0;
  switch (d.delaylineType())
  {
  case Hex :
    {
      switch (d.layersToUse())
      {
      case(UV):
        {
          //std::cout << "uv"<<std::endl;
          firstLayer = &d.layers()['U'];
          secondLayer = &d.layers()['V'];
          break;
        }
      case(UW):
        {
          //std::cout << "uw"<<std::endl;
          firstLayer = &d.layers()['U'];
          secondLayer = &d.layers()['W'];
          break;
        }
      case(VW):
        {
          //std::cout << "vw"<<std::endl;
          firstLayer = &d.layers()['V'];
          secondLayer = &d.layers()['W'];
          break;
        }
      default:
        throw std::invalid_argument("the chosen layer combination does not exist");
        return;
        break;
      }
    }
  case Quad:
    firstLayer = &d.layers()['X'];
    secondLayer = &d.layers()['Y'];
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
  const Instrument::channels_t &MCPInstrChans = dev.instruments().find(MCPInstr)->second.channels();
  const Channel &MCPChan = MCPInstrChans[MCPChanNbr];


  Signal &F1 = firstLayer->wireend()['1'];
  const Instruments &F1Instr = F1.instrument();
  const size_t F1ChanNbr = F1.channelNbr();
  const WaveformAnalyzers &F1Anal= F1.analyzerType();
  const Instrument::channels_t &F1InstrChans = dev.instruments().find(F1Instr)->second.channels();
  const Channel &F1Chan = F1InstrChans[F1ChanNbr];

  Signal &F2 = firstLayer->wireend()['2'];
  const Instruments &F2Instr = F2.instrument();
  const size_t F2ChanNbr = F2.channelNbr();
  const WaveformAnalyzers &F2Anal= F2.analyzerType();
  const Instrument::channels_t &F2InstrChans = dev.instruments().find(F2Instr)->second.channels();
  const Channel &F2Chan = F2InstrChans[F2ChanNbr];


  Signal &S1 = secondLayer->wireend()['1'];
  const Instruments &S1Instr = S1.instrument();
  const size_t S1ChanNbr = S1.channelNbr();
  const WaveformAnalyzers &S1Anal= S1.analyzerType();
  const Instrument::channels_t &S1InstrChans = dev.instruments().find(S1Instr)->second.channels();
  const Channel &S1Chan = S1InstrChans[S1ChanNbr];

  Signal &S2 = secondLayer->wireend()['2'];
  const Instruments &S2Instr = S2.instrument();
  const size_t S2ChanNbr = S2.channelNbr();
  const WaveformAnalyzers &S2Anal= S2.analyzerType();
  const Instrument::channels_t &S2InstrChans = dev.instruments().find(S2Instr)->second.channels();
  const Channel &S2Chan = S2InstrChans[S2ChanNbr];

  //now extract values//
  if ((MCPChanNbr >= MCPInstrChans.size()))
  {
    std::cerr << "the requested channel for mcp \""<<MCPChanNbr<<"\" is not present. We only have \""<<MCPInstrChans.size()<<"\" channels"<<std::endl;
    return;
  }
  (*_waveformanalyzer)[MCPAnal]->analyze(MCPChan,MCP);

  //check whether the requested channel does exist//
  if ((F1ChanNbr >= F1InstrChans.size()))
  {
    std::cerr << "the requested channel for first layer one \""<<F1ChanNbr<<"\" is not present. We only have \""<<F1InstrChans.size()<<"\" channels"<<std::endl;
    return;
  }
  //  std::cerr<<"waveformanalyzertyp for first layer one "<<firstLayer->wireend()['1'].analyzerType()<<" chnbr:"<<firstLayer->wireend()['1'].channelNbr()<<std::endl;
  (*_waveformanalyzer)[F1Anal]->analyze(F1Chan,F1);

  //check whether the requested channel does exist//
  if ((F2ChanNbr >= F2InstrChans.size()))
  {
    std::cerr << "the requested channel for first layer two \""<<F2ChanNbr<<"\" is not present. We only have \""<<F2InstrChans.size()<<"\" channels"<<std::endl;
    return;
  }
  //  std::cerr<<"waveformanalyzertyp for first layer two "<<firstLayer->wireend()['2'].analyzerType()<<" chnbr:"<<firstLayer->wireend()['2'].channelNbr()<<std::endl;
  (*_waveformanalyzer)[F2Anal]->analyze(F2Chan,F2);

  //check whether the requested channel does exist//
  if ((S1ChanNbr >= S1InstrChans.size()))
  {
    std::cerr << "the requested channel for second layer one \""<<S1ChanNbr<<"\" is not present. We only have \""<<S1InstrChans.size()<<"\" channels"<<std::endl;
    return;
  }
  //  std::cerr<<"waveformanalyzertyp for second layer one "<<secondLayer->wireend()['1'].analyzerType()<<" chnbr:"<<secondLayer->wireend()['1'].channelNbr()<<std::endl;
  (*_waveformanalyzer)[S1Anal]->analyze(S1Chan,S1);

  //check whether the requested channel does exist//
  if ((S2ChanNbr >= S2InstrChans.size()))
  {
    std::cerr << "the requested channel for second layer two \""<<S2ChanNbr<<"\" is not present. We only have \""<<S2InstrChans.size()<<"\" channels"<<std::endl;
    return;
  }
  //  std::cerr<<"waveformanalyzertyp for second layer two "<<secondLayer->wireend()['2'].analyzerType()<<" chnbr:"<<secondLayer->wireend()['2'].channelNbr()<<std::endl;
  (*_waveformanalyzer)[S2Anal]->analyze(S2Chan,S2);

  ////tell the signals that you have updated it//
  //d.mcp().isNewEvent() = true;
  //firstLayer->one().isNewEvent() = true;
  //firstLayer->two().isNewEvent() = true;
  //secondLayer->one().isNewEvent() = true;
  //secondLayer->two().isNewEvent() = true;
  
  //now sort these peaks for the layers timesum//
  //  std::cout << "sort for timesum"<<std::endl;
  sortForTimesum(d,*firstLayer,*secondLayer);
  //  std::cout <<"done"<<std::endl;
}


//____________________________functions that will use a simple sorting__________________________________
void findBoundriesForSorting(const cass::ACQIRIS::Signal &anodeEnd, const double mcp, const double ts, const double rTime, int &min, int &max)
{
  //--we know two things:--//
  //-- |x1-x2|<rTimex  and--//
  //-- x1+x2-2mcp = tsx with tsx = 0.5*(tsxhigh-tsxlow)--//
  //--with this knowledge we can calculate the boundries for the anode--//
  //--given the Timesum and the Runtime--//

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

////___________________________________________________________________________________________________________________________________________________________
//void findBoundriesForSorting(pVec anode, double tMCP, double timesum, int &min, int &max)
//{
//    bool firsttime=true;
//    min=-1;max=-2;
//
//    for (int i=0;i<anode.size();i++)
//    {
//        double tAnode=anode[i]->getCFD();
//        if(tAnode<tMCP)
//            continue;
//        else if(tAnode > (tMCP+timesum))
//            break;
//        else
//        {
//            if(firsttime)
//            {
//                min=i;
//                firsttime=false;
//            }
//            max=i;
//       }
//    }
//}
//___________________________________________________________________________________________________________________________________________________________
void cass::ACQIRIS::DelaylineDetectorAnalyzerSimple::sortForTimesum(cass::ACQIRIS::DelaylineDetector &d,cass::ACQIRIS::AnodeLayer &xLayer,cass::ACQIRIS::AnodeLayer &yLayer)
{
  //--calculate the timesum from the given lower and upper boundries for it--//
  const double tsx		= xLayer.ts();
  const double tsy		= yLayer.ts();
  const double runttime	= d.runtime();
  const double tsxLow		= xLayer.tsLow();
  const double tsxHigh	= xLayer.tsHigh();
  const double tsyLow		= yLayer.tsLow();
  const double tsyHigh	= yLayer.tsHigh();
  const double radius		= d.mcpRadius();

  //  std::cout <<d.mcp().peaks().size() <<" ";
  //  std::cout <<d.u().wireend()['1'].peaks().size() <<" ";
  //  std::cout <<d.u().wireend()['2'].peaks().size() <<" ";
  //  std::cout <<d.v().wireend()['1'].peaks().size() <<" ";
  //  std::cout <<d.v().wireend()['2'].peaks().size() <<std::endl;
  for (size_t iMcp=0;iMcp<d.mcp().peaks().size();++iMcp)
  {
    //    std::cout << d.mcp().peaks()[iMcp]->isUsed()<<std::endl;
    if (d.mcp().peaks()[iMcp].isUsed()) continue;
    //--find the right indizes, only look in the right timerange--//
    const double mcp = d.mcp().peaks()[iMcp].time();
    int iX1min,iX1max,iX2min,iX2max,iY1min,iY1max,iY2min,iY2max;
    findBoundriesForSorting(xLayer.wireend()['1'],mcp,tsx,runttime,iX1min,iX1max);
    findBoundriesForSorting(xLayer.wireend()['2'],mcp,tsx,runttime,iX2min,iX2max);
    findBoundriesForSorting(yLayer.wireend()['1'],mcp,tsy,runttime,iY1min,iY1max);
    findBoundriesForSorting(yLayer.wireend()['2'],mcp,tsy,runttime,iY2min,iY2max);

    //  std::cout <<iMcp <<" ";
    //  std::cout <<"x1low:"<<iX1min <<" x1high:"<<iX1max;
    //  std::cout <<"x2low:"<<iX2min <<" x2high:"<<iX2max;
    //  std::cout <<"y1low:"<<iY1min <<" y1high:"<<iY1max;
    //  std::cout <<"y2low:"<<iY2min <<" y2high:"<<iY2max;
    //  std::cout <<std::endl;

    //go through all possible combinations//
    for (int iX1=iX1min;iX1<=iX1max;++iX1)
    {
      if (xLayer.wireend()['1'].peaks()[iX1].isUsed()) continue;
      for (int iX2=iX2min;iX2<=iX2max;++iX2)
      {
        if (xLayer.wireend()['2'].peaks()[iX2].isUsed()) continue;
        for (int iY1=iY1min;iY1<=iY1max;++iY1)
        {
          if (yLayer.wireend()['1'].peaks()[iY1].isUsed()) continue;
          for (int iY2=iY2min;iY2<=iY2max;++iY2)
          {
            if (yLayer.wireend()['2'].peaks()[iY2].isUsed()) continue;

            //            std::cout <<"checking timesum condition for combination "<< iX1<<","<< iX2<<","<< iY1<<","<< iY2<<","<< iMcp<<","<<std::endl;
            //calc the timesum//
            const double x1 = xLayer.wireend()['1'].peaks()[iX1].time();
            const double x2 = xLayer.wireend()['2'].peaks()[iX2].time();
            const double y1 = yLayer.wireend()['1'].peaks()[iY1].time();
            const double y2 = yLayer.wireend()['2'].peaks()[iY2].time();
            const double sumx = x1+x2 - 2.* mcp;
            const double sumy = y1+y2 - 2.* mcp;

            //calc pos and radius//
            const double xlay_mm = (x1-x2) * xLayer.sf();
            const double ylay_mm = (y1-y2) * yLayer.sf();
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
              //add a DetektorHit to the Detektor
              d.hits().push_back(DelaylineDetectorHit(x_mm,y_mm,mcp));

              //remember that this mcp Peak has already been used//
              d.mcp().peaks()[iMcp].isUsed()    = true;
              xLayer.wireend()['1'].peaks()[iX1].isUsed() = true;
              xLayer.wireend()['2'].peaks()[iX2].isUsed() = true;
              yLayer.wireend()['1'].peaks()[iY1].isUsed() = true;
              yLayer.wireend()['2'].peaks()[iY2].isUsed() = true;
            }
          }
        }
      }
    }
  }
}
