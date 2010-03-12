#include <iostream>
#include <cmath>

#include "delayline_detector_analyzer_simple.h"
#include "delayline_detector.h"
#include "channel.h"
#include "waveform_analyzer_backend.h"


//****************************************The Class Implementation*******************************************************
//___________________________________________________________________________________________________________________________________________________________
void cass::ACQIRIS::DelaylineDetectorAnalyzerSimple::analyze(cass::ACQIRIS::DetectorBackend& detector, const std::vector<cass::ACQIRIS::Channel>& channels)
{
  //std::cout << "do the sorting"<<std::endl;
  //do a type conversion to have a delayline detector//
  DelaylineDetector &d = dynamic_cast<DelaylineDetector&>(detector);
  //check what layer the user wants to use for calculating the pos//
  AnodeLayer *firstLayer=0;
  AnodeLayer *secondLayer=0;
  switch (d.layersToUse())
  {
  case(UV):
    {
      //std::cout << "uv"<<std::endl;
      firstLayer = &d.layers()[U];
      secondLayer = &d.layers()[V];
      break;
    }
  case(UW):
    {
      //std::cout << "uw"<<std::endl;
      firstLayer = &d.layers()[U];
      secondLayer = &d.layers()[W];
      break;
    }
  case(VW):
    { 
      //std::cout << "vw"<<std::endl;
      firstLayer = &d.layers()[V];
      secondLayer = &d.layers()[W];
      break;
    }
  default: break;
  }
  //extract the peaks for the signals of the detector from the channels//
//  std::cerr<<"waveformanalyzertyp mcp "<<firstLayer->two().analyzerType()<<" chnbr:"<<d.mcp().channelNbr()<<std::endl;
  (*_waveformanalyzer)[d.mcp().analyzerType()]->analyze(channels[d.mcp().channelNbr()],d.mcp());

  //std::cerr<<"waveformanalyzertyp for first layer one "<<firstLayer->one().analyzerType()<<" chnbr:"<<firstLayer->one().channelNbr()<<std::endl;
  (*_waveformanalyzer)[firstLayer->one().analyzerType()]->analyze(channels[firstLayer->one().channelNbr()],firstLayer->one());

  //std::cerr<<"waveformanalyzertyp for first layer two "<<firstLayer->two().analyzerType()<<" chnbr:"<<firstLayer->two().channelNbr()<<std::endl;
  (*_waveformanalyzer)[firstLayer->two().analyzerType()]->analyze(channels[firstLayer->two().channelNbr()],firstLayer->two());

  //std::cerr<<"waveformanalyzertyp for second layer one "<<secondLayer->one().analyzerType()<<" chnbr:"<<secondLayer->one().channelNbr()<<std::endl;
  (*_waveformanalyzer)[secondLayer->one().analyzerType()]->analyze(channels[secondLayer->one().channelNbr()],secondLayer->one());

  //std::cerr<<"waveformanalyzertyp for second layer two "<<secondLayer->two().analyzerType()<<" chnbr:"<<secondLayer->two().channelNbr()<<std::endl;
  (*_waveformanalyzer)[secondLayer->two().analyzerType()]->analyze(channels[secondLayer->two().channelNbr()],secondLayer->two());

  //now sort these peaks for the layers timesum//
  //std::cout << "sort for timesum"<<std::endl;
  sortForTimesum(d,*firstLayer,*secondLayer);
  //std::cout <<"done"<<std::endl;
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
//  std::cout <<d.u().one().peaks().size() <<" ";
//  std::cout <<d.u().two().peaks().size() <<" ";
//  std::cout <<d.v().one().peaks().size() <<" ";
//  std::cout <<d.v().two().peaks().size() <<std::endl;
  for (size_t iMcp=0;iMcp<d.mcp().peaks().size();++iMcp)
  {
//    std::cout << d.mcp().peaks()[iMcp]->isUsed()<<std::endl;
    if (d.mcp().peaks()[iMcp].isUsed()) continue;
    //--find the right indizes, only look in the right timerange--//
    const double mcp = d.mcp().peaks()[iMcp].time();
    int iX1min,iX1max,iX2min,iX2max,iY1min,iY1max,iY2min,iY2max;
    findBoundriesForSorting(xLayer.one(),mcp,tsx,runttime,iX1min,iX1max);
    findBoundriesForSorting(xLayer.two(),mcp,tsx,runttime,iX2min,iX2max);
    findBoundriesForSorting(yLayer.one(),mcp,tsy,runttime,iY1min,iY1max);
    findBoundriesForSorting(yLayer.two(),mcp,tsy,runttime,iY2min,iY2max);

//  std::cout <<iMcp <<" ";
//  std::cout <<"x1low:"<<iX1min <<" x1high:"<<iX1max;
//  std::cout <<"x2low:"<<iX2min <<" x2high:"<<iX2max;
//  std::cout <<"y1low:"<<iY1min <<" y1high:"<<iY1max;
//  std::cout <<"y2low:"<<iY2min <<" y2high:"<<iY2max;
//  std::cout <<std::endl;

    //go through all possible combinations//
    for (int iX1=iX1min;iX1<=iX1max;++iX1)
    {
      if (xLayer.one().peaks()[iX1].isUsed()) continue;
      for (int iX2=iX2min;iX2<=iX2max;++iX2)
      {
        if (xLayer.two().peaks()[iX2].isUsed()) continue;
        for (int iY1=iY1min;iY1<=iY1max;++iY1)
        {
          if (yLayer.one().peaks()[iY1].isUsed()) continue;
          for (int iY2=iY2min;iY2<=iY2max;++iY2)
          {
            if (yLayer.two().peaks()[iY2].isUsed()) continue;

//            std::cout <<"checking timesum condition for combination "<< iX1<<","<< iX2<<","<< iY1<<","<< iY2<<","<< iMcp<<","<<std::endl;
            //calc the timesum//
            const double x1 = xLayer.one().peaks()[iX1].time();
            const double x2 = xLayer.two().peaks()[iX2].time();
            const double y1 = yLayer.one().peaks()[iY1].time();
            const double y2 = yLayer.two().peaks()[iY2].time();
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
              xLayer.one().peaks()[iX1].isUsed() = true;
              xLayer.two().peaks()[iX2].isUsed() = true;
              yLayer.one().peaks()[iY1].isUsed() = true;
              yLayer.two().peaks()[iY2].isUsed() = true;
            }
          }
        }
      }
    }
  }
}
