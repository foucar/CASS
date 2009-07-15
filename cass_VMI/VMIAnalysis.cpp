/*
 *  VMIAnalysis.cpp
 *  diode
 *
 *  Created by Jochen Küpper on 20.05.09.
 *  Copyright 2009 Fritz-Haber-Institut der MPG. All rights reserved.
 *
 */

#include "VMIAnalysis.h"

void cass::VMI::Analysis::init(const cass::VMI::Parameter &param)
{
    _threshold    = param._threshold;
    _xCenterOfMcp = param._xCenterOfMcp;
    _yCenterOfMcp = param._yCenterOfMcp;
    _maxMcpRadius = param._maxMcpRadius;
}

void cass::VMI::Analysis::operator()(cass::VMI::VMIEvent &vmievent)
{
    //initialize the start values for integral and max pixel value//
    uint16_t maxpixelvalue              = 0;
    uint32_t integral                   = 0;
    uint16_t framewidth                 = vmievent.columns();
    uint16_t frameheight                = vmievent.rows();
    const std::vector<uint16_t>& frame  = vmievent.frame();

    //go through all pixels of the frame//
    for (size_t i=0; i<frame.size(); ++i)
    {
        //extract the value and coordinate from the frame//
        uint16_t pixel              = frame[i];
        uint16_t xcoordinate        = i % framewidth;
        uint16_t ycoordinate        = i / framewidth;

        //calc integral//
        integral += pixel;

        //check whether pixel is outside of maximum radius//
        //if not then add pixel to cutframe//
        uint16_t corX = xcoordinate - _xCenterOfMcp;
        uint16_t corY = ycoordinate - _yCenterOfMcp;
        if (corX*corX + corY*corY < _maxMcpRadius*_maxMcpRadius)
        {
            vmievent.cutFrame()[i] = pixel;
        }

        //check whether pixel is a local maximum//
        //if so add its coordinates to the coordinates of impact map//
        //check wether pixel is above threshold
        if (pixel > _threshold)
        //check wether point is at an edge
        if (ycoordinate > 0 &&
            ycoordinate < frameheight-1 &&
            xcoordinate > 0 &&
            xcoordinate < framewidth+1)
        // Check all surrounding pixels
        if (frame[i-framewidth-1] < pixel && //upper left
            frame[i-framewidth]   < pixel && //upper middle
            frame[i-framewidth+1] < pixel && //upper right
            frame[i-1]            < pixel && //left
            frame[i+1]            < pixel && //right
            frame[i+framewidth-1] < pixel && //lower left
            frame[i+framewidth]   < pixel && //lower middle
            frame[i+framewidth+1] < pixel)   //lower right
        {
            vmievent.coordinatesOfImpact().push_back(Coordinate(xcoordinate,ycoordinate));
        }

        //get the maximum pixel value//
        if (maxpixelvalue < pixel)
            maxpixelvalue = pixel;
    }
    //write the found integral and maximum Pixel value to the event//
    vmievent.integral(integral);
    vmievent.maxPixelValue(maxpixelvalue);
}
