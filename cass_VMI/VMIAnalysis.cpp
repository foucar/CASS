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
    uint16_t maxpixelvalue=0;
    uint32_t integral=0;

    //go through all pixels of the frame//
    for (size_t i=0; i<vmievent.frame().size(); ++i)
    {
        //extract the value and coordinate from the frame//
        uint16_t pixel = vmievent.frame()[i];
        uint32_t xcoordinate = i % vmievent.columns();
        uint32_t ycoordinate = i / vmievent.columns();

        //calc integral//
        integral += pixel;

        //check wether pixel is outside of maximum radius//
        //if not then add pixel to cutframe//

        //check whether pixel is a local maximum//
        //if so add its coordinates to the coordinates of impact map//

        //get the maximum pixel value//
        if (maxpixelvalue < pixel)
            maxpixelvalue = pixel;
    }
    //write the found integral and maximum Pixel value to the event//
    vmievent.integral(integral);
    vmievent.maxPixelValue(maxpixelvalue);
}

