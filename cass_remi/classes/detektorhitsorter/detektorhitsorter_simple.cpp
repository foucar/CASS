#include <iostream>
#include <cmath>

#include "detektorhitsorter_simple.h"
#include "detector.h"
#include "remi_analysis.h"
#include "remi_event.h"
#include "channel.h"


//****************************************The Class Implementation*******************************************************
cass::REMI::DetektorHitSorterSimple::DetektorHitSorterSimple(const cass::REMI::DetectorParameter& param):
        DetektorHitSorterQuad(param)
{
    if (param.fIsHex) std::cout << "using simple sorting for Hex-Anode.. Only U-Layer and V-Layer will be used!!!" <<std::endl;
}

//___________________________________________________________________________________________________________________________________________________________
void cass::REMI::DetektorHitSorterSimple::sort(cass::REMI::REMIEvent& e, cass::REMI::Detector& d)
{
    d.extractFromChannels(e.channels());
//    FillHistosBeforeShift(d);
    sortForTimesum(d);
}


//____________________________functions that will use a simple sorting__________________________________
void findBoundriesForSorting(const cass::REMI::Signal &anodeEnd, const double mcp, const double ts, const double rTime, int &min, int &max)
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
    for (size_t i=0;i<anodeEnd.nbrPeaks();++i)
    {
        //check wether the current anode value will fall into the boundries
        if ( fabs(2.*anodeEnd.t(i) - 2.*mcp - ts) <= rTime )
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
void cass::REMI::DetektorHitSorterSimple::sortForTimesum(cass::REMI::Detector &d)
{
    //--calculate the timesum from the given lower and upper boundries for it--//
    const double tsx		= d.u().ts();
    const double tsy		= d.v().ts();
    const double runttime	= d.runtime();
    const double tsxLow		= d.u().tsLow();
    const double tsxHigh	= d.u().tsHigh();
    const double tsyLow		= d.v().tsLow();
    const double tsyHigh	= d.v().tsHigh();
    const double radius		= d.mcpRadius();

    for (size_t iMcp=0;iMcp<d.mcp().nbrPeaks();++iMcp)
    {
        if (d.mcp().peak(iMcp).isUsed()) continue;
        //--find the right indizes, only look in the right timerange--//
        int iX1min,iX1max,iX2min,iX2max,iY1min,iY1max,iY2min,iY2max;
        findBoundriesForSorting(d.u().one(),d.mcp().t(iMcp),tsx,runttime,iX1min,iX1max);
        findBoundriesForSorting(d.u().two(),d.mcp().t(iMcp),tsx,runttime,iX2min,iX2max);
        findBoundriesForSorting(d.v().one(),d.mcp().t(iMcp),tsy,runttime,iY1min,iY1max);
        findBoundriesForSorting(d.v().two(),d.mcp().t(iMcp),tsy,runttime,iY2min,iY2max);

        //go through all possible combinations//
        for (int iX1=iX1min;iX1<=iX1max;++iX1)
        {
            if (d.u().one().peak(iX1).isUsed()) continue;
            for (int iX2=iX2min;iX2<=iX2max;++iX2)
            {
                if (d.u().two().peak(iX2).isUsed()) continue;
                for (int iY1=iY1min;iY1<=iY1max;++iY1)
                {
                    if (d.v().one().peak(iY1).isUsed()) continue;
                    for (int iY2=iY2min;iY2<=iY2max;++iY2)
                    {
                        if (d.v().two().peak(iY2).isUsed()) continue;

                        //calc the timesum//
                        const double sumx = d.u().sum(iX1,iX2) - 2.* d.mcp().t(iMcp);
                        const double sumy = d.v().sum(iY1,iY2) - 2.* d.mcp().t(iMcp);

                        //calc pos and radius//
                        const double x_mm = d.u().pos_mm(iX1,iX2);
                        const double y_mm = (d.isHexAnode())? 1./sqrt(3.) * (x_mm - 2*d.v().pos_mm(iY1,iY2)) : d.v().pos_mm(iY1,iY2);
                        const double t = d.mcp().t(iMcp);
                        const double radius_mm = sqrt(x_mm*x_mm + y_mm*y_mm);


                        //check wether the timesum is correct//
                        if ( (sumx > tsxLow) && (sumx < tsxHigh) )
                        if ( (sumy > tsyLow) && (sumy < tsyHigh) )
                        //check wether the hit is inside the radius of the MCP//
                        if (radius_mm < radius)
                        {
                            //add a DetektorHit to the Detektor
                            d.addHit(x_mm, y_mm, t);

                            //remember that this mcp Peak has already been used//
                            d.mcp().peak(iMcp).isUsed(true);
                            d.u().one().peak(iX1).isUsed(true);
                            d.u().two().peak(iX2).isUsed(true);
                            d.v().one().peak(iY1).isUsed(true);
                            d.v().two().peak(iY2).isUsed(true);
                        }
                    }
                }
            }
        }
    }
}
