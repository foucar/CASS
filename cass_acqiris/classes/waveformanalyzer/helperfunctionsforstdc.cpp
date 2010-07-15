/**
 * @file helperfunctionsforstdc.cpp this file is empty and contains only an
 *                                  include of the header.
 *
 * @author Lutz Foucar
 */

#include "helperfunctionsforstdc.h"



//_________________________________gib mir zurueck______________________
//double gmz(const double i, const double x0, const dvec MPuls)
//{
//    //calc the distance between the center of the Meanpuls(50) and the input puls(x0)//
//    double Abstand = x0 - 50.;
//
//    //calc the x position that is requested//
//    double x = i-Abstand;
//
//    //truncate this number and you know the bin left to the wanted x point//
//    int binLeft = (int) x;
//
//    //check wether we are still in range//
//    if (binLeft<0) return 0;
//    if (binLeft>=(MPuls.size()-1)) return 0;
//
//    //make a line through the leftbin and the right bin//
//    //give back the y for the asked x//
//    double m = MPuls[binLeft+1] - MPuls[binLeft];
//    double ret = m*(x-binLeft) + MPuls[binLeft];
//
//    return ret;
//}
//
