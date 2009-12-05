/*
 *  VMIAnalysis.cpp
 *  diode
 *
 *  Created by Jochen Küpper on 20.05.09.
 *  Copyright 2009 Fritz-Haber-Institut der MPG. All rights reserved.
 * filled with live by lmf
 */
#include <iostream>
#include "vmi_analysis.h"
#include "cass_event.h"
#include "vmi_event.h"


void cass::VMI::Parameter::load()
{
  //sync before loading//
  sync();
  _threshold    = value("Threshold",0).toUInt();
  _centerOfMcp  = value("CenterOfMcp",QPoint(200, 200)).toPoint();
  _maxMcpRadius = value("MaxMcpRadius",200).toUInt();
  _rebinfactor  = value("RebinFactor",4).toUInt();
}

void cass::VMI::Parameter::save()
{
  setValue("Threshold",_threshold);
  setValue("CenterOfMcp",_centerOfMcp);
  setValue("MaxMcpRadius",_maxMcpRadius);
  setValue("RebinFactor",_rebinfactor);
}







void cass::VMI::Analysis::operator()(cass::CASSEvent *cassevent)
{
  cass::VMI::VMIEvent& vmievent = cassevent->VMIEvent();

  //clear the vector of impacts//
  vmievent.coordinatesOfImpact().clear();

  //initialize the start values for integral and max pixel value//
  uint16_t maxpixelvalue                     = 0;
  uint32_t integral                          = 0;
  uint16_t framewidth                        = vmievent.columns();
  uint16_t frameheight                       = vmievent.rows();
  const cass::VMI::VMIEvent::frame_t& frame  = vmievent.frame();

  //go through all pixels of the frame//
  for (size_t i=0; i<frame.size(); ++i)
  {
    //extract the value and coordinate from the frame//
    uint16_t pixel          = frame[i];
    uint16_t xcoordinate    = i % framewidth;
    uint16_t ycoordinate    = i / framewidth;

    //calc integral//
    integral += pixel;

    /*//check whether pixel is outside of maximum radius//
    //if not then add pixel to cutframe//
    uint16_t corX = xcoordinate - _param._centerOfMcp.x();
    uint16_t corY = ycoordinate - _param._centerOfMcp.y();
    if (corX*corX + corY*corY < _param._maxMcpRadius*_param._maxMcpRadius)
    {
      vmievent.cutFrame()[i] = pixel;
    }*/

    /*//check whether pixel is a local maximum//
    //if so add its coordinates to the coordinates of impact map//
    //check wether pixel is above threshold
    if (pixel > _param._threshold)
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
      }*/

    //get the maximum pixel value//
    if (maxpixelvalue < pixel)
      maxpixelvalue = pixel;
  }
  //write the found integral and maximum Pixel value to the event//
  vmievent.integral()     = integral;
  vmievent.maxPixelValue()= maxpixelvalue;

  //rebinning the frame//
  //rebin image frame//
  if (_param._rebinfactor != 1)
  {
    //get the new dimensions//
    const size_t newRows = framewidth  / _param._rebinfactor;
    const size_t newCols = frameheight / _param._rebinfactor;
    //set the new dimensions in the detector//
    vmievent.rows()    = newRows;
    vmievent.columns() = newCols;
    //resize the temporary container to fit the rebinned image
    //initialize it with 0
    _tmp.assign(newRows * newCols,0);
    //rebin the frame//
    for (size_t iIdx=0; iIdx<frame.size() ;++iIdx)
    {
      //calculate the row and column of the current Index//
      const size_t row = iIdx / framewidth;
      const size_t col = iIdx % framewidth;
      //calculate the index of the rebinned frame//
      const size_t newRow = row / _param._rebinfactor;
      const size_t newCol = col / _param._rebinfactor;
      //calculate the index in the rebinned frame//
      //that newRow and newCol belongs to//
      const size_t newIndex = newRow*newCols + newCol;
      //add this index value to the newIndex value//
      _tmp[newIndex] += frame[iIdx];
    }
    //copy the temporary frame to the right place
    vmievent.frame().assign(_tmp.begin(), _tmp.end());

  }
}
