#include "detektorhitsorter_quad.h"



//___________________________________________________________________________________________________________________________________________________________
void cass::REMI::DetectorHitSorterQuad::FillHistosBeforeShift(const cass::REMI::Detector&)
{
    ////--timesum--//
    //rm.fill2d(fHiOff+kSumVsURaw,u1-u2,u1+u2-2.*mcp);
    //rm.fill2d(fHiOff+kSumVsVRaw,v1-v2,v1+v2-2.*mcp);

    //rm.fill1d(fHiOff+kSumU,u1+u2-2.*mcp);
    //rm.fill1d(fHiOff+kSumV,v1+v2-2.*mcp);


    ////check for right timesum//
    //const bool csu =TMath::Abs( u1+u2-2.*mcp - tsU) < 10;
    //const bool csv =TMath::Abs( v1+v2-2.*mcp - tsV) < 10;

    ////draw Histograms with detektor positions//
    //if (csu && csv)
    //{
    //	rm.fill2d(fHiOff+kDetRaw_ns  ,u_ns,v_ns);
    //	rm.fill2d(fHiOff+kDetRaw_mm  ,u_mm,v_mm);
    //}
}
