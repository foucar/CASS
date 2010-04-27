/*
 *  Created by Nicola Coppola 30.03.2010 to have ROI(s) as a separate object
 */

#include <QtCore/QMutexLocker>
//#include "ccd_analysis.h"
#include "cass_event.h"
//#include "ccd_device.h"
#include "pixel_detector.h"

//void cass::TWODDetector::TWODDetector::ROIload(/*QSettings *_detROI*/)

/*
void cass::ROI::load(cass::ROI::detROI_ *_detROI)
{
  //sync before loading//
  sync();
  //sting for the container index//
  QString s;
  _detROI._ROI.clear();
  beginGroup("ROIs");
  for (size_t iROI=0; iROI<value("ROIsize",1).toUInt(); ++iROI)
  {
    beginGroup(s.setNum(static_cast<uint32_t>(iROI)));
    _detROI._ROI.push_back(ROIsimple());
    //the shape of the ROI//
    _detROI._ROI[iROI].name = value("ROIShapeName","square").toString().toStdString();
    // the size(s) along the x axis
    _detROI._ROI[iROI].xsize = value("XSize",1).toUInt();
    // the size(s) along the y axis
    _detROI._ROI[iROI].ysize = value("YSize",1).toUInt();
    // the centre(s) along the x axis
    _detROI._ROI[iROI].xcentre = value("XCentre",1).toUInt();
    // the centre(s) along the y axis
    _detROI._ROI[iROI].ycentre = value("YCentre",1).toUInt();
    // the orientation is used only in the case of a triangular shape
    _detROI._ROI[iROI].orientation = value("orientation",1).toInt();
    endGroup();
    std::cout <<"ROI loaded "<< iROI << " xsize " << _detROI._ROI[iROI].xsize 
              <<" ysize " << _detROI._ROI[iROI].ysize 
              <<" xcentre " << _detROI._ROI[iROI].xcentre
              <<" ycentre " << _detROI._ROI[iROI].ycentre 
              <<" orientation " << _detROI._ROI[iROI].orientation<<std::endl;
  }
  endGroup();
  std::cout << "done ROI load "<< _detROI._ROI.size() << " of Detector " << _detROI._detID << std::endl;
  //reset the ROImask and iterators
  _ROImask.clear();
  //_ROImask_converter.clear();
  _ROIiterator.clear();
  //_ROIiterator_converter.clear();
}

//void cass::TWODDetector::TWODDetector::ROIsave(QSettings *_detROI)
void cass::ROI::save()
{
  //sting for the container index//
  QString s;
  setValue("ROIsize",static_cast<uint32_t>(_detROI._ROI.size()));
  beginGroup("ROIs");
  for (size_t iROI=0; iROI< _detROI._ROI.size(); ++iROI)
  {
    beginGroup(s.setNum(static_cast<int>(iROI)));
    setValue("ROIShapeName",_detROI._ROI[iROI].name.c_str());
    setValue("XSize",_detROI._ROI[iROI].xsize);
    setValue("YSize",_detROI._ROI[iROI].ysize);
    setValue("XCentre",_detROI._ROI[iROI].xcentre);
    setValue("YCentre",_detROI._ROI[iROI].ycentre);
    // the orientation is used only in the case of a triangular shape
    setValue("orientation",_detROI._ROI[iROI].orientation);
    endGroup();
  }
  endGroup();
}

*/
