// Copyright (C) 2010 Nicola Coppola
// Copyright (C) 2010 Lutz Foucar

#include <QtCore/QSettings>
#include <stdexcept>

#include "region_of_interest.h"

/** A Region of Interest Shape.
 * this is a simple shape the region of interest mask we
 * be created by a list of these shapes.
 * @author Nicola Coppola
 */
struct ROIShape
{
  /** name describing what type this shape is*/
  std::string shapetype;
  /** the size(s) along the x axis*/
  uint32_t xsize;
  //! the size(s) along the y axis
  uint32_t ysize;
  //! the centre(s) along the x axis
  uint32_t xcentre;
  //! the centre(s) along the y axis
  uint32_t ycentre;
  /** the orientation @see ROI*/
  int32_t orientation;
};

cass::ROI::create(const std::string detectorname, entity_t entity)const
{
  //Create a parameter from which we will read the attibutes
  QSettings par;
  par.sync();
  par.beginGroup(detectorname.c_str());

  //find out how many shapes this region of interest is created from//
  int nbrShapes = par.beginReadArray("Shapes");
  //create the shape container to fit all shapes//
  std::vector<ROIShape> shapes(nbrShapes);
  for (int iShape=0; iShape < nbrShapes; ++iShape)
  {
    par.setArrayIndex(iShape);
    //retrieve this shapes attributes//
    shapes[iShape].shapetype    = par.value("Type","square").toString().toStdString();
    shapes[iShape].xsize        = value("XSize",1).toUInt();
    shapes[iShape].ysize        = value("YSize",1).toUInt();
    shapes[iShape].xcentre      = value("XCentre",1).toUInt();
    shapes[iShape].ycentre      = value("YCentre",1).toUInt();
    shapes[iShape].orientation  = value("Orientation",1).toInt();
  }
  par.endArray();

  //create the mask//
  roi_t roi(size,1);

  /*
    code that creates the roi mask from the shapes comes here
  */

  //now that the roi mask has been created we can create the other entities if requested
  roi_t en;
  switch (entity)
  {
  case ROIMask:
    //just copy the created mask
    en=roi;
    break;
  case ROIIterator:
    {
      /*place code to create the ROI Iterator from the mask here*/
    }
    break;
  case ROIMaskConverter:
    {
      /*place code here*/
    }
    break;
  case ROIIteratorConverter:
    {
      /*place code here*/
    }
    break;
  default:
    throw std::invalid_argument("This is an unknown Region of Interest Entity");
  }
  //once we created the entity we return it by value//
  //since its a stl vector it will has the copy on write//
  //which makes this really efficent//
  return en;
}
