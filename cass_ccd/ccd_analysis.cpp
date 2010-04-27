/*
 *  Created by Lutz Foucar on 23.02.2010
 */

#include "ccd_analysis.h"
#include "cass_event.h"
#include "ccd_detector.h"
#include "ccd_device.h"


void cass::CCD::Parameter::load()
{
  //sync before loading//
  sync();
  _threshold    = value("Threshold",0).toUInt();
  _rebinfactor  = value("RebinFactor",1).toUInt();
}

void cass::CCD::Parameter::save()
{
  setValue("Threshold",_threshold);
  setValue("RebinFactor",_rebinfactor);
}






void cass::CCD::Analysis::operator()(cass::CASSEvent *cassevent)
{
  //retrieve a pointer to the ccd device we are working on//
  cass::CCD::CCDDevice* dev(dynamic_cast<cass::CCD::CCDDevice*>(cassevent->devices()[cass::CASSEvent::CCD]));
  //retrieve a reference to the pulnix detector//
  cass::CCDDetector& det((*dev->detectors())[0]);

  //clear the pixel list//
  det.pixellist().clear();

  //initialize the start values for integral and max pixel value//
  pixel_t maxpixelvalue                       = 0;
  float integral                              = 0;
  uint16_t framewidth                         = det.columns();
  uint16_t frameheight                        = det.rows();
  const cass::CCDDetector::frame_t& frame      = det.frame();

  //go through all pixels of the frame//
  for (size_t i=0; i<frame.size(); ++i)
  {
    //extract the value and coordinate from the frame//
    pixel_t pixel         = frame[i];
    uint16_t xcoordinate  = i % framewidth;
    uint16_t ycoordinate  = i / framewidth;

    //calc integral//
    integral += pixel;

    //check whether pixel is above threshold//
    //then check whether pixel is a local maximum//
    //if so add it to the pixel list//
    if (pixel > _param._threshold)
      //check wether point is not at an edge
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
        det.pixellist().push_back(Pixel(xcoordinate,ycoordinate,pixel));
      }

    //get the maximum pixel value//
    if (maxpixelvalue < pixel)
      maxpixelvalue = pixel;
  }
  //write the found integral and maximum Pixel value to the event//
  det.integral()     = static_cast<uint32_t>(integral);
  det.maxPixelValue()= maxpixelvalue;

  //rebinning the frame//
  //rebin image frame//
  if (_param._rebinfactor != 1)
  {
    //lock this section due to the access to the tmp frame//
    QMutexLocker locker(&_mutex);

    //get the new dimensions//
    const size_t newRows = framewidth  / _param._rebinfactor;
    const size_t newCols = frameheight / _param._rebinfactor;
    //set the new dimensions in the detector//
    det.rows()    = newRows;
    det.columns() = newCols;
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
    det.frame().assign(_tmp.begin(), _tmp.end());
  }
}
