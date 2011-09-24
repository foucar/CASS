//Copyright (C) 2011 Lutz Foucar

/**
 * @file pixeldetector_mask.cpp contains definition of the mask of a pixeldetector
 *
 * @author Lutz Foucar
 */

#include <tr1/functional>
#include <map>
#include <string>
#include <stdexcept>
#include <sstream>

#include "pixeldetector_mask.h"

#include "cass_settings.h"
#include "common_data.h"

using namespace cass;
using namespace pixeldetector;
using namespace std;

namespace cass
{
namespace pixeldetector
{
/** an index within a matrix */
typedef pair<size_t,size_t> index_t;

/** operates a plus on two indices
 *
 * performs \f(lhs_1+rhs_1)(lhs_2+rhs_2)\f$.
 *
 * @return the result of the operation
 * @param lhs the left hand side of the operation
 * @param rhs the right hand side of the opeation
 *
 * @author Lutz Foucar
 */
index_t operator+(const index_t& lhs, const index_t& rhs)
{
  return make_pair(lhs.first + rhs.first,
                   lhs.second + rhs.second);
}

/** operates a minus on two indices
 *
 * performs \f(lhs_1-rhs_1)(lhs_2-rhs_2)\f$.
 *
 * @return the result of the operation
 * @param lhs the left hand side of the operation
 * @param rhs the right hand side of the opeation
 *
 * @author Lutz Foucar
 */
index_t operator-(const index_t& lhs, const index_t& rhs)
{
  return make_pair(lhs.first - rhs.first,
                   lhs.second - rhs.second);
}

/** operates times on two indices
 *
 * performs \f(lhs_1*rhs_1)(lhs_2*rhs_2)\f$.
 *
 * @return the result of the operation
 * @param lhs the left hand side of the operation
 * @param rhs the right hand side of the opeation
 *
 * @author Lutz Foucar
 */
index_t operator*(const index_t& lhs, const index_t& rhs)
{
  return make_pair(lhs.first * rhs.first,
                   lhs.second * rhs.second);
}

/** operates devides on two indices
 *
 * performs \f(lhs_1/rhs_1)(lhs_2/rhs_2)\f$.
 *
 * @return the result of the operation
 * @param lhs the left hand side of the operation
 * @param rhs the right hand side of the opeation
 *
 * @author Lutz Foucar
 */
index_t operator/(const index_t& lhs, const index_t& rhs)
{
  return make_pair(lhs.first / rhs.first,
                   lhs.second / rhs.second);
}

/** operates less of an  indices to a scalar
 *
 * performs \f(lhs_1+lhs_2)<rhs\f$.
 *
 * @return the result of the operation
 * @param lhs the left hand side of the operation
 * @param rhs the right hand side of the opeation
 *
 * @author Lutz Foucar
 */
bool operator<(const index_t& lhs, const size_t rhs)
{
  return ((lhs.first + lhs.second) < rhs);
}

/** convert any type to a string
 *
 * should be used for converting numbers to strings. This function was inspired
 * by a function found at Sep, 24th 2011 here:
 * http://notfaq.wordpress.com/2006/08/30/c-convert-int-to-string/
 *
 * @tparam Type the type of the number
 * @param t the number to convert to string
 *
 * @author Lutz Foucar
 */
template <typename Type>
inline string toString (const Type& t)
{
  stringstream ss;
  ss << t;
  return ss.str();
}

/** a mask element
 *
 * each Mask element need the following "attributes": shape, xsize, ysize,
 * xcenter, ycenter, shape and orientation shapes:=circ(==circle),ellipse,
 * triangle(isosceles),square and  the orientation!!!
 *
 * The orientation is used only in the case of a triangular shape
 *
 * xsize,ysize and center are in pixel units.
 *
 * The orientation is used only in the case of a triangular shape.
 *
 * @verbatim
 /\           ----         |\           /|
/  \  ==+1    \  /  ==-1   | \  ==+2   / | == -2
----           \/          | /         \ |
                           |/           \|
   @endverbatim
 * if I rotate the plane by -pi/2: -2=>+1 1=>+2 -1=>-2  +2=>-1.
 * Please remember to use the rotated frame wrt standard-natural frame
 * orientation!!
 *
 * @todo refine the documentation by using casssettng
 *
 * @author Nicola Coppola
 */
struct MaskElement
{
  /** shape type */
  std::string type;

  /** size of shape along x-axis */
  uint32_t xsize;

  /** the size(s) along the y axis */
  uint32_t ysize;

  /** the centre(s) along the x axis */
  uint32_t xcentre;

  /** the centre(s) along the y axis */
  uint32_t ycentre;

  /** the orientation of the triangle */
  int32_t orientation;
};

/** convert matrix index to linearised index
 *
 * @return index in the linearised array
 * @param index in the matrix
 * @param width the width of the matrix
 *
 * @author Lutz Foucar
 */
size_t TwoD2OneD(const index_t& matrixIndex, const size_t width)
{
  return matrixIndex.second * width + matrixIndex.first;
}

/** convert linearised index to  matrixindex
 *
 * @return index in the matrix
 * @param index in the matrix
 * @param width the width of the matrix
 *
 * @author Lutz Foucar
 */
index_t OneD2TwoD(const size_t linearisedIndex, const size_t width)
{
  return make_pair(linearisedIndex % width,
                   linearisedIndex / width);
}

/** add a circle to the mask
 *
 * goes through the sqare that conatins the cirlce and checks whether the index
 * is covered by the circle. If so the mask at that index will be set to false.
 *
 * @cassttng PixelDetectors/\%name\%/CorrectionMaps/Mask/\%index\%/{CenterX|CenterY}\n
 *           The center of the circle. Default is 500|500.
 * @cassttng PixelDetectors/\%name\%/CorrectionMaps/Mask/\%index\%/{Radius}\n
 *           The radius of the circle. Default is 2.
 *
 * @param data the container containing the mask where the element should be added
 * @param s the settings element to read the mask element parameters from
 *
 * @author Nicola Coppola
 * @author Lutz Foucar
 */
void addCircle(CommonData &data, CASSSettings &s)
{
  QWriteLocker lock(&data.lock);
  const index_t center(make_pair(s.value("CenterX",500).toUInt(),
                                 s.value("CenterY",500).toUInt()));
  const size_t radius(s.value("Radius",2).toUInt());

  if ((center.first < radius) ||
      (center.second < radius) ||
      ((center.first + radius) > data.columns) ||
      ((center.second + radius) > data.rows))
  {
    throw invalid_argument("addCircle(): The radius '" + toString(radius) +
                           "' is choosen to big and does not fit with center ("
                           + toString(center.first) +","
                           + toString(center.second)+")");
  }
  const size_t radius_sq(radius*radius);
  const index_t lowerLeft(make_pair(center.first-radius, center.second-radius));
  const index_t upperRight(make_pair(center.first+radius, center.second+radius));
  const size_t width(data.columns);

  for (size_t row(lowerLeft.second); row < upperRight.second; ++row)
  {
    for (size_t column(lowerLeft.first); column < upperRight.first; ++column)
    {
      const index_t idx(make_pair(column,row));
      const index_t idx_sq((idx - center)*(idx - center));
      data.mask[TwoD2OneD(idx,width)] *=  !(idx_sq < radius_sq);
    }
  }
}

/** add a square element to the mask
 *
 * sets all pixels covered by the square to 0.
 *
 * @cassttng PixelDetectors/\%name\%/CorrectionMaps/Mask/\%index\%/{LowerLeftX|LowerY}\n
 *           The lower left pixel of the square element. The indizes given are
 *.          included in the square. Default is 0|0.
 * @cassttng PixelDetectors/\%name\%/CorrectionMaps/Mask/\%index\%/{LowerLeftX|LowerY}\n
 *           The upper right pixel of the square element. The indizes given are
 *.          included in the square. Default is 1023|1023.
 *
 * @param data the container containing the mask where the element should be added
 * @param s the settings element to read the mask element parameters from
 *
 * @author Nicola Coppola
 * @author Lutz Foucar
 */
void addSquare(CommonData &data, CASSSettings &s)
{
  QWriteLocker lock(&data.lock);
  const index_t lowerLeft(make_pair(s.value("LowerLeftX",0).toUInt(),
                                    s.value("LowerLeftY",0).toUInt()));
  const index_t upperRight(make_pair(s.value("UpperRightX",1024).toUInt(),
                                     s.value("UpperRightY",1024).toUInt()));
  if ((data.columns < upperRight.first) ||
      (data.rows < upperRight.second) ||
      (upperRight.first < lowerLeft.first) ||
      (upperRight.second < lowerLeft.second))
  {
    throw invalid_argument("addSquare(): Either the upper right coordinate ("
                           + toString(upperRight.first) +","
                           + toString(upperRight.second)+") "+
                           "is too big for the mask that has a size of ("
                           + toString(data.columns) +","
                           + toString(data.rows)+") "+
                           "or the lowerLeft corner ("
                           + toString(lowerLeft.first) +","
                           + toString(lowerLeft.second)+") "+
                           "is not really at the lower left");
  }
  const size_t width(data.columns);
  for (size_t row(lowerLeft.second); row <= upperRight.second; ++row)
  {
    for (size_t column(lowerLeft.first); column <= upperRight.first; ++column)
    {
      const index_t idx(make_pair(column,row));
      data.mask[TwoD2OneD(idx,width)] = 0;
    }
  }
}

/** add a ellipsodial element to the mask
 *
 * adds an ellipsodial to the mask. Will iterate trhough the sqare that contains
 * the ellipse and checks which pixels should be masked.
 *
 * @cassttng PixelDetectors/\%name\%/CorrectionMaps/Mask/\%index\%/{CenterX|CenterY}\n
 *           The central point of the ellipse. Default is 500|500.
 * @cassttng PixelDetectors/\%name\%/CorrectionMaps/Mask/\%index\%/{SemiAxisX|SemiAxisY}\n
 *           The semi axis along x and y of the ellipse. By definition the
 *           longer one defines the major axis and the smaller on the minor axis.
 *           Default is 5|4.
 *
 * @param data the container containing the mask where the element should be added
 * @param s the settings element to read the mask element parameters from
 *
 * @author Nicola Coppola
 * @author Lutz Foucar
 */
void addEllipse(CommonData &data, CASSSettings &s)
{
  QWriteLocker lock(&data.lock);
  const index_t center(make_pair(s.value("CenterX",500).toUInt(),
                                 s.value("CenterY",500).toUInt()));
  const size_t a(s.value("SemiAxisX",5).toUInt());
  const size_t b(s.value("SemiAxisY",2).toUInt());
  const size_t width(data.columns);

  if ((center.first < a) ||
      (center.second < b) ||
      ((center.first + a) > data.columns) ||
      ((center.second + b) > data.rows))
  {
    throw invalid_argument("addCircle(): The semi axis x '" + toString(a) +
                           "' and b '" + toString(b) +
                           "' are choosen to big and do not fit with center ("
                           + toString(center.first) +","
                           + toString(center.second)+")");
  }

  const index_t lowerLeft(make_pair(center.first-a, center.second-b));
  const index_t upperRight(make_pair(center.first+a, center.second+b));
  const index_t axis_sq(make_pair(a,b)*make_pair(a,b));

  for (size_t row(lowerLeft.second); row <= upperRight.second; ++row)
  {
    for (size_t column(lowerLeft.first); column <= upperRight.first; ++column)
    {
      const index_t idx(make_pair(column,row));
      const index_t idx_sq((idx - center)*(idx - center));
      const index_t idx_tmp(idx_sq / axis_sq);
      data.mask[TwoD2OneD(idx,width)] *=  !(idx_tmp < 1);
    }
  }
}

/** add a triangluar element to the mask
 *
 * @param data the container containing the mask where the element should be added
 * @param s the settings element to read the mask element parameters from
 *
 * @author Nicola Coppola
 * @author Lutz Foucar
 */
void addTriangle(CommonData &data, CASSSettings &s)
{
#warning "implement this"

}

}
}

void createCASSMask(CommonData &data, CASSSettings &s)
{
  map<string,tr1::function<void(CommonData&, CASSSettings&)> > functions;
  functions["circle"] = &addCircle;
  functions["circ"] = &addCircle;
  functions["square"] = &addSquare;
  functions["triangle"] = &addTriangle;
  functions["ellipse"] = &addEllipse;

  int size = s.beginReadArray("Mask");
  for (int i = 0; i < size; ++i)
  {
    s.setArrayIndex(i);
    string type(s.value("MaskElementType","square").toString().toStdString());
    if (functions.find(type) == functions.end())
      throw invalid_argument("createCASSMask(): Unknown Mask Element Type '" +type+ "'");
    functions[type](data,s);
  }
  s.endArray();

//  // "enable" all  pixel as default
//  dp._ROImask.assign(dp._ROImask.size(),1);
//  dp._ROImask.assign(dp._ROImask_converter.size(),1);
//  //I need to reset the number of masked pixel per frame
//  number_of_pixelsettozero=0;
//  for(size_t iROI=0;iROI<dp._detROI._ROI.size(); ++iROI)
//  {
//    const size_t half_pnCCD_def_size=pnCCD::default_size/2;
//    const int32_t sign_pnCCD_def_size=static_cast<int32_t>(pnCCD::default_size);
//    const int32_t sign_pnCCD_def_size_sq=static_cast<int32_t>(pnCCD::default_size_sq);
//    const int32_t signed_ROI_xcentre=static_cast<int32_t>(dp._detROI._ROI[iROI].xcentre);
//    const int32_t signed_ROI_ycentre=static_cast<int32_t>(dp._detROI._ROI[iROI].ycentre);
//    const int32_t signed_ROI_xsize=static_cast<int32_t>(dp._detROI._ROI[iROI].xsize);
//    const int32_t signed_ROI_ysize=static_cast<int32_t>(dp._detROI._ROI[iROI].ysize);

//    int32_t index_of_centre=signed_ROI_xcentre + sign_pnCCD_def_size * signed_ROI_ycentre;
//    int32_t index_min;
//    int32_t this_index;

//    int32_t signed_index_min=signed_ROI_xcentre - signed_ROI_xsize
//      + sign_pnCCD_def_size * (signed_ROI_ycentre - signed_ROI_ysize);
//    int diff_Xsize_Xcentre_to_boundary=signed_ROI_xcentre - signed_ROI_xsize;
//    int diff_Ysize_Ycentre_to_boundary=signed_ROI_ycentre - signed_ROI_ysize;
//    if( diff_Xsize_Xcentre_to_boundary<0 || diff_Ysize_Ycentre_to_boundary<0 )
//      std::cout << printoutdef <<  "too small distances x;y "<< diff_Xsize_Xcentre_to_boundary
//                <<" "<< diff_Ysize_Ycentre_to_boundary<< " for ROI " << iROI
//                <<" "<< signed_index_min<<" " <<index_of_centre <<std::endl;
//    index_min=signed_ROI_xcentre - signed_ROI_xsize
//      + sign_pnCCD_def_size * (signed_ROI_ycentre - signed_ROI_ysize);
//    if(index_min>sign_pnCCD_def_size_sq) std::cout<<"What? "<<dp._detROI._ROI[iROI].xcentre << " "<<
//                              dp._detROI._ROI[iROI].xsize << " "<<
//                              dp._detROI._ROI[iROI].ycentre << " "<< dp._detROI._ROI[iROI].ysize<<std::endl;
//#ifdef debug
//    size_t index_max=dp._detROI._ROI[iROI].xcentre + dp._detROI._ROI[iROI].xsize
//      + pnCCD::default_size * (dp._detROI._ROI[iROI].ycentre + dp._detROI._ROI[iROI].ysize);
//    std::cout << printoutdef << "indexes "<< index_of_centre<<" "<<index_min<<" "<<index_max<<std::endl;
//#endif
//    int32_t indexROI_min=0;
//    int32_t indexROI_max=(2 * signed_ROI_xsize + 1)
//        * (2 * signed_ROI_ysize + 1);
//    size_t u_indexROI_min=static_cast<size_t>(indexROI_min);
//    size_t u_indexROI_max=static_cast<size_t>(indexROI_max);
//#ifdef debug
//    std::cout << printoutdef <<  "indexes "<< index_of_centre<<" "<<indexROI_min<<" "<<indexROI_max<<std::endl;
//#endif

//    //------------------------------------------------------------------------

//    if(dp._detROI._ROI[iROI].name=="circ" || dp._detROI._ROI[iROI].name=="circle"  )
//    {
//      int32_t  xlocal,ylocal;
//      const int32_t radius2 =  static_cast<int32_t>(pow(dp._detROI._ROI[iROI].xsize,2) );
//#ifdef debug
//      std::cout << printoutdef << "circ seen with radius^2= " <<radius2 <<std::endl;
//#endif
//      for(size_t iFrame=u_indexROI_min;iFrame<u_indexROI_max; ++iFrame)
//      {
//        xlocal=iFrame%(2* dp._detROI._ROI[iROI].xsize + 1);
//        ylocal=iFrame/(2* dp._detROI._ROI[iROI].xsize + 1);
//        if( ( pow(xlocal-signed_ROI_xsize,2) +
//              pow(ylocal-signed_ROI_ysize,2) ) <= radius2 )
//        {
//          this_index=index_min+xlocal+ sign_pnCCD_def_size * (ylocal);
//          //I do not need to set again to zero a pixel that was already masked!
//          //I have also to check that I have not landed on the other side of the CHIP
//          if (this_index>=0 && (this_index < sign_pnCCD_def_size_sq) && dp._ROImask[this_index]!=0)
//          {
//            if( dp._detROI._ROI[iROI].xcentre<=half_pnCCD_def_size )
//            {
//              if( (this_index%sign_pnCCD_def_size)<= signed_ROI_xsize+signed_ROI_xcentre )
//              {
//                dp._ROImask[this_index]=0;
//                //remember how many pixels I have masked
//                number_of_pixelsettozero++;
//              }
//            }
//            else
//            {
//              if( (signed_ROI_xcentre-signed_ROI_xsize) < (this_index%sign_pnCCD_def_size) )
//              {
//                dp._ROImask[this_index]=0;
//                //remember how many pixels I have masked
//                number_of_pixelsettozero++;
//              }
//            }
//          }
//        }
//      }
//    }


//    //------------------------------------------------------------------------


//    if(dp._detROI._ROI[iROI].name=="ellipse" )
//    {
//      int32_t xlocal,ylocal;
//      const float a2 =  static_cast<float>(pow(dp._detROI._ROI[iROI].xsize,2));
//      const float b2 =  static_cast<float>(pow(dp._detROI._ROI[iROI].ysize,2));
//#ifdef debug
//      std::cout << printoutdef << "circ ellipse with semi-axis^2= " << a2 << " and " << b2 <<std::endl;
//#endif
//      for(size_t iFrame=u_indexROI_min;iFrame<u_indexROI_max; ++iFrame)
//      {
//        xlocal=iFrame%(2* dp._detROI._ROI[iROI].xsize + 1);
//        ylocal=iFrame/(2* dp._detROI._ROI[iROI].xsize + 1);
//#ifdef debug
//        std::cout<<"local "<<xlocal<<" "<<ylocal<<" "<<dp._detROI._ROI[iROI].xsize<< " "
//                 <<pow(xlocal-signed_ROI_xsize,2) <<std::endl;
//#endif
//        if( ( pow(static_cast<float>(xlocal)-static_cast<float>(dp._detROI._ROI[iROI].xsize),2)/a2 +
//              pow(static_cast<float>(ylocal)-static_cast<float>(dp._detROI._ROI[iROI].ysize),2)/b2 ) <= 1 )
//        {
//          this_index=index_min+xlocal+ sign_pnCCD_def_size * (ylocal);
//          //I do not need to set again to zero a pixel that was already masked!
//          //I have also to check that I have not landed on the other side of the CHIP
//          if (this_index>=0 && (this_index < sign_pnCCD_def_size_sq) && dp._ROImask[this_index]!=0)
//          {
//            if( dp._detROI._ROI[iROI].xcentre<=half_pnCCD_def_size )
//            {
//              if( (this_index%sign_pnCCD_def_size)<= signed_ROI_xsize+signed_ROI_xcentre )
//              {
//                dp._ROImask[this_index]=0;
//                //remember how many pixels I have masked
//                number_of_pixelsettozero++;
//              }
//            }
//            else
//            {
//              if( (signed_ROI_xcentre-signed_ROI_xsize) < (this_index%sign_pnCCD_def_size) )
//              {
//                dp._ROImask[this_index]=0;
//                //remember how many pixels I have masked
//                number_of_pixelsettozero++;
//              }
//            }
//          }
//        }
//      }
//    }


//    //------------------------------------------------------------------------


//    if(dp._detROI._ROI[iROI].name=="specialline")
//    {
//      float  xlocal1_f,ylocal1_f,xlocal2_f,ylocal2_f;
//      int32_t  xlocal,ylocal;
//      const int32_t radius_min2 =  static_cast<int32_t>(pow(dp._detROI._ROI[iROI].xsize,2));
//      const int32_t radius_max2 =  static_cast<int32_t>(pow(dp._detROI._ROI[iROI].ysize,2));
//      indexROI_max=sign_pnCCD_def_size*140/100/2;
//      const float slope=static_cast<float>(dp._detROI._ROI[iROI].orientation);
//      if(std::abs(slope)<200.) std::cout<<"slope "<<slope<<std::endl;
//      else std::cout<<"vertical slope"<<std::endl;
//      for(int iFrame=0;iFrame<indexROI_max; ++iFrame)
//      {
//        if(std::abs(slope)<200.)
//        {
//          xlocal1_f=static_cast<float>(signed_ROI_xcentre + iFrame);
//          xlocal2_f=static_cast<float>(signed_ROI_xcentre - iFrame);
//          ylocal1_f=static_cast<float>(signed_ROI_ycentre) + static_cast<float>(iFrame) * slope ;
//          ylocal2_f=static_cast<float>(signed_ROI_ycentre) - static_cast<float>(iFrame) * slope ;
//        }
//        else
//        {
//          xlocal1_f=static_cast<float>(signed_ROI_xcentre);
//          xlocal2_f=static_cast<float>(signed_ROI_xcentre);
//          ylocal1_f=static_cast<float>(signed_ROI_ycentre + iFrame);
//          ylocal2_f=static_cast<float>(signed_ROI_ycentre - iFrame);
//        }
//#ifdef thisdebug
//        std::cout<<"local "<<xlocal1_f<<" "<<ylocal1_f
//                 << " " << " "<< xlocal2_f<<" "<<ylocal2_f <<std::endl;
//#endif
//        //Inside the first radius
//        if( ( pow(xlocal1_f-signed_ROI_xcentre,2) +
//              pow(ylocal1_f-signed_ROI_ycentre,2) ) < radius_min2 )
//        {
//#ifdef thisdebug
//        std::cout<<"locali "<<xlocal1_f<<" "<<ylocal1_f
//                 << " " << " "<< xlocal2_f<<" "<<ylocal2_f <<std::endl;
//#endif
//          xlocal=static_cast<int32_t>(xlocal1_f);
//          ylocal=static_cast<int32_t>(ylocal1_f);
//          this_index=xlocal + sign_pnCCD_def_size * (ylocal);
//          //              std::cout<<"local1a "<<this_index;
//          //I do not need to set again to zero a pixel that was already masked!
//          //I have also to check that I have not landed on the other side of the CHIP
//          if (this_index>=0 && (this_index < sign_pnCCD_def_size_sq) && dp._ROImask[this_index]!=0)
//          {
//            dp._ROImask[this_index]=0;
//            //remember how many pixels I have masked
//            number_of_pixelsettozero++;
//          }
//          if (this_index+1>=0 && (this_index+1 < sign_pnCCD_def_size_sq) && dp._ROImask[this_index+1]!=0)
//          {
//            //std::cout<<"local1f "<<this_index+1;
//            dp._ROImask[this_index+1]=0;
//            //remember how many pixels I have masked
//            number_of_pixelsettozero++;
//          }
//          if (this_index-1>=0 && (this_index-1 < sign_pnCCD_def_size_sq) && dp._ROImask[this_index-1]!=0)
//          {
//            //std::cout<<"local1f "<<this_index+1;
//            dp._ROImask[this_index-1]=0;
//            //remember how many pixels I have masked
//            number_of_pixelsettozero++;
//          }

//          xlocal=static_cast<int32_t>(xlocal2_f);
//          ylocal=static_cast<int32_t>(ylocal2_f);
//          this_index=xlocal + sign_pnCCD_def_size * (ylocal);
//          //I do not need to set again to zero a pixel that was already masked!
//          //I have also to check that I have not landed on the other side of the CHIP
//          if (this_index>=0 && (this_index < sign_pnCCD_def_size_sq) && dp._ROImask[this_index]!=0)
//          {
//            //std::cout<<"local1f "<<this_index;
//            dp._ROImask[this_index]=0;
//            //remember how many pixels I have masked
//            number_of_pixelsettozero++;
//          }

//          if (this_index+1>=0 && (this_index+1 < sign_pnCCD_def_size_sq) && dp._ROImask[this_index+1]!=0)
//          {
//            //std::cout<<"local1f "<<this_index+1;
//            dp._ROImask[this_index+1]=0;
//            //remember how many pixels I have masked
//            number_of_pixelsettozero++;
//          }
//          if (this_index-1>=0 && (this_index-1 < sign_pnCCD_def_size_sq) && dp._ROImask[this_index-1]!=0)
//          {
//            //std::cout<<"local1f "<<this_index+1;
//            dp._ROImask[this_index-1]=0;
//            //remember how many pixels I have masked
//            number_of_pixelsettozero++;
//          }
//        }
//        else
//        {
//          //Outside the second radius
//          if( ( pow(xlocal1_f-signed_ROI_xcentre,2) +
//                pow(ylocal1_f-signed_ROI_ycentre,2) ) > radius_max2 )
//          {
//#ifdef thisdebug
//            std::cout<<"local2 "<<xlocal1_f<<" "<<ylocal1_f
//                     << " " << " "<< xlocal2_f<<" "<<ylocal2_f <<std::endl;
//#endif
//            xlocal=static_cast<int32_t>(xlocal1_f);
//            ylocal=static_cast<int32_t>(ylocal1_f);
//            this_index=xlocal + sign_pnCCD_def_size * (ylocal);
//#ifdef thisdebug
//            std::cout<<"local2a "<<this_index;
//#endif
//            //I do not need to set again to zero a pixel that was already masked!
//            //I have also to check that I have not landed on the other side of the CHIP
//            if (this_index>=0 && (this_index < sign_pnCCD_def_size_sq) && dp._ROImask[this_index]!=0)
//            {
//              //std::cout<<"local2f "<<this_index;
//              dp._ROImask[this_index]=0;
//              //remember how many pixels I have masked
//              number_of_pixelsettozero++;
//            }
//            if (this_index+1>=0 && (this_index+1 < sign_pnCCD_def_size_sq) && dp._ROImask[this_index+1]!=0)
//            {
//              //std::cout<<"local1f "<<this_index+1;
//              dp._ROImask[this_index+1]=0;
//              //remember how many pixels I have masked
//              number_of_pixelsettozero++;
//            }
//            if (this_index-1>=0 && (this_index-1 < sign_pnCCD_def_size_sq) && dp._ROImask[this_index-1]!=0)
//            {
//              //std::cout<<"local1f "<<this_index+1;
//              dp._ROImask[this_index-1]=0;
//              //remember how many pixels I have masked
//              number_of_pixelsettozero++;
//            }

//            xlocal=static_cast<int32_t>(xlocal2_f);
//            ylocal=static_cast<int32_t>(ylocal2_f);
//            this_index=xlocal + sign_pnCCD_def_size * (ylocal);
//            //I do not need to set again to zero a pixel that was already masked!
//            //I have also to check that I have not landed on the other side of the CHIP
//            if (this_index>=0 && (this_index < sign_pnCCD_def_size_sq) && dp._ROImask[this_index]!=0)
//            {
//              dp._ROImask[this_index]=0;
//              //remember how many pixels I have masked
//              number_of_pixelsettozero++;
//            }
//            if (this_index+1>=0 && (this_index+1 < sign_pnCCD_def_size_sq) && dp._ROImask[this_index+1]!=0)
//            {
//              //std::cout<<"local1f "<<this_index+1;
//              dp._ROImask[this_index+1]=0;
//              //remember how many pixels I have masked
//              number_of_pixelsettozero++;
//            }
//            if (this_index-1>=0 && (this_index-1 < sign_pnCCD_def_size_sq) && dp._ROImask[this_index-1]!=0)
//            {
//              //std::cout<<"local1f "<<this_index+1;
//              dp._ROImask[this_index-1]=0;
//              //remember how many pixels I have masked
//              number_of_pixelsettozero++;
//            }
//          }
//        }
//      }
//    }


//    //------------------------------------------------------------------------


//    if(dp._detROI._ROI[iROI].name=="square")
//    {
//      int32_t  xlocal,ylocal;
//#ifdef debug
//      std::cout << printoutdef <<  "square seen" <<std::endl;
//#endif
//      for(size_t iFrame=u_indexROI_min;iFrame<u_indexROI_max; ++iFrame)
//      {
//        xlocal=iFrame%(2* dp._detROI._ROI[iROI].xsize +1);
//        ylocal=iFrame/(2* dp._detROI._ROI[iROI].xsize +1);
//        this_index=index_min+xlocal+ sign_pnCCD_def_size * (ylocal);
//        //I do not need to set again to zero a pixel that was already masked!
//        //I have also to check that I have not landed on the other side of the CHIP
//        if (this_index>=0 && (this_index < sign_pnCCD_def_size_sq) && dp._ROImask[this_index]!=0)
//        {
//          if( dp._detROI._ROI[iROI].xcentre<=half_pnCCD_def_size )
//          {
//            if( (this_index%sign_pnCCD_def_size)<= signed_ROI_xsize+signed_ROI_xcentre )
//            {
//              dp._ROImask[this_index]=0;
//              //remember how many pixels I have masked
//              number_of_pixelsettozero++;
//            }
//          }
//          else
//          {
//            if( (signed_ROI_xcentre-signed_ROI_xsize) < (this_index%sign_pnCCD_def_size) )
//            {
//              dp._ROImask[this_index]=0;
//              //remember how many pixels I have masked
//              number_of_pixelsettozero++;
//            }
//          }
//        }
//#ifdef debug
//        std::cout<<"local "<<xlocal<<" "<<ylocal<<" "<<dp._detROI._ROI[iROI].ycentre
//                 << " "<<dp._detROI._ROI[iROI].ycentre - dp._detROI._ROI[iROI].ysize <<std::endl;
//#endif
//      }
//    }


//    //------------------------------------------------------------------------


//    if(dp._detROI._ROI[iROI].name=="triangle")
//    {
//      int32_t  xlocal,ylocal;
//      float xlocal_f,ylocal_f;
//      float xsize,ysize;
//      xsize=static_cast<float>(dp._detROI._ROI[iROI].xsize);
//      ysize=static_cast<float>(dp._detROI._ROI[iROI].ysize);

//#ifdef debug
//      std::cout << printoutdef <<  "triangle seen" <<std::endl;
//#endif
//      if(dp._detROI._ROI[iROI].orientation==+1)
//      {
//#ifdef debug
//        std::cout << printoutdef <<  "triangle seen vertex upwards" <<std::endl;
//#endif
//        //the triangle is at least isosceles
//        for(size_t iFrame=u_indexROI_min;iFrame<u_indexROI_max; ++iFrame)
//        {
//          xlocal=iFrame%(2* dp._detROI._ROI[iROI].xsize + 1);
//          ylocal=iFrame/(2* dp._detROI._ROI[iROI].xsize + 1);
//          xlocal_f=static_cast<float>(xlocal);
//          ylocal_f=static_cast<float>(ylocal);

//#ifdef debug
//          std::cout<<"local "<<xlocal<<" "<<ylocal
//                   << " " <<2 * ysize/xsize*xlocal_f << " " <<4*ysize - 2* ysize/xsize*xlocal_f
//                   <<std::endl;
//#endif
//          if(ylocal-1<(2 * ysize/xsize*xlocal_f)
//             && xlocal< (signed_ROI_xsize+1) )
//          {
//            this_index=index_min+xlocal+ sign_pnCCD_def_size * (ylocal);
//            //I do not need to set again to zero a pixel that was already masked!
//            //I have also to check that I have not landed on the other side of the CHIP
//            if (this_index>=0 && (this_index < sign_pnCCD_def_size_sq) && dp._ROImask[this_index]!=0)
//            {
//              if( dp._detROI._ROI[iROI].xcentre<=half_pnCCD_def_size )
//              {
//                if( (this_index%sign_pnCCD_def_size)<= signed_ROI_xsize+signed_ROI_xcentre )
//                {
//                  dp._ROImask[this_index]=0;
//                  //remember how many pixels I have masked
//                  number_of_pixelsettozero++;
//                }
//              }
//              else
//              {
//                if( (signed_ROI_xcentre-signed_ROI_xsize) < (this_index%sign_pnCCD_def_size) )
//                {
//                  dp._ROImask[this_index]=0;
//                  //remember how many pixels I have masked
//                  number_of_pixelsettozero++;
//                }
//              }
//            }
//          }
//          else if(ylocal-1<(4*ysize - 2* ysize/xsize*xlocal_f)
//                  && xlocal>signed_ROI_xsize)
//          {
//            this_index=index_min+xlocal+ sign_pnCCD_def_size * (ylocal);
//            //I do not need to set again to zero a pixel that was already masked!
//            //I have also to check that I have not landed on the other side of the CHIP
//            if (this_index>=0 && (this_index < sign_pnCCD_def_size_sq) && dp._ROImask[this_index]!=0)
//            {
//              if( dp._detROI._ROI[iROI].xcentre<=half_pnCCD_def_size )
//              {
//                if( (this_index%sign_pnCCD_def_size)<= signed_ROI_xsize+signed_ROI_xcentre )
//                {
//                  dp._ROImask[this_index]=0;
//                  //remember how many pixels I have masked
//                  number_of_pixelsettozero++;
//                }
//              }
//              else
//              {
//                if( (signed_ROI_xcentre-signed_ROI_xsize) < (this_index%sign_pnCCD_def_size) )
//                {
//                  dp._ROImask[this_index]=0;
//                  //remember how many pixels I have masked
//                  number_of_pixelsettozero++;
//                }
//              }
//            }
//          }
//        }
//      }
//      if(dp._detROI._ROI[iROI].orientation==-1)
//      {
//#ifdef debug
//        std::cout << printoutdef <<  "triangle seen vertex downwards" <<std::endl;
//#endif
//        for(size_t iFrame=u_indexROI_min;iFrame<u_indexROI_max; ++iFrame)
//        {
//          xlocal=iFrame%(2* dp._detROI._ROI[iROI].xsize + 1);
//          ylocal=iFrame/(2* dp._detROI._ROI[iROI].xsize + 1);
//          xlocal_f=static_cast<float>(xlocal);
//          ylocal_f=static_cast<float>(ylocal);
//#ifdef debug
//          std::cout<<"local "<<xlocal<<" "<<ylocal
//                   << " " <<(-2 * ysize/xsize*xlocal_f) + 2 * ysize
//                   << " "<<-2*ysize + 2 *  ysize/xsize*xlocal <<std::endl;
//#endif

//          if(ylocal+1>((-2 * ysize/xsize*xlocal_f)
//                     + 2 * ysize)
//             && xlocal< (signed_ROI_xsize+1))
//          {
//            this_index=index_min+xlocal+ sign_pnCCD_def_size * (ylocal);
//            //I do not need to set again to zero a pixel that was already masked!
//            //I have also to check that I have not landed on the other side of the CHIP
//            if (this_index>=0 && (this_index < sign_pnCCD_def_size_sq) && dp._ROImask[this_index]!=0)
//            {
//              if( dp._detROI._ROI[iROI].xcentre<=half_pnCCD_def_size )
//              {
//                if( (this_index%sign_pnCCD_def_size)<= signed_ROI_xsize+signed_ROI_xcentre )
//                {
//                  dp._ROImask[this_index]=0;
//                  //remember how many pixels I have masked
//                  number_of_pixelsettozero++;
//                }
//              }
//              else
//              {
//                if( (signed_ROI_xcentre-signed_ROI_xsize) < (this_index%sign_pnCCD_def_size) )
//                {
//                  dp._ROImask[this_index]=0;
//                  //remember how many pixels I have masked
//                  number_of_pixelsettozero++;
//                }
//              }
//            }
//          }
//          else if(ylocal+1>(-2*ysize +
//                          2 * ysize/xsize*xlocal_f)
//                  && xlocal>signed_ROI_xsize)
//          {
//            this_index=index_min+xlocal+ sign_pnCCD_def_size * (ylocal);
//            //I do not need to set again to zero a pixel that was already masked!
//            //I have also to check that I have not landed on the other side of the CHIP
//            if (this_index>=0 && (this_index < sign_pnCCD_def_size_sq) && dp._ROImask[this_index]!=0)
//            {
//              if( dp._detROI._ROI[iROI].xcentre<=half_pnCCD_def_size )
//              {
//                if( (this_index%sign_pnCCD_def_size)<= signed_ROI_xsize+signed_ROI_xcentre )
//                {
//                  dp._ROImask[this_index]=0;
//                  //remember how many pixels I have masked
//                  number_of_pixelsettozero++;
//                }
//              }
//              else
//              {
//                if( (signed_ROI_xcentre-signed_ROI_xsize) < (this_index%sign_pnCCD_def_size) )
//                {
//                  dp._ROImask[this_index]=0;
//                  //remember how many pixels I have masked
//                  number_of_pixelsettozero++;
//                }
//              }
//            }
//          }
//        }
//      }
//      if(dp._detROI._ROI[iROI].orientation==+2)
//      {
//#ifdef debug
//        std::cout << printoutdef <<  "triangle seen vertex towards right" <<std::endl;
//#endif
//        for(size_t iFrame=u_indexROI_min;iFrame<u_indexROI_max; ++iFrame)
//        {
//          // not debugged
//          xlocal=iFrame%(2* dp._detROI._ROI[iROI].xsize + 1);
//          ylocal=iFrame/(2* dp._detROI._ROI[iROI].xsize + 1);
//          xlocal_f=static_cast<float>(xlocal);
//          ylocal_f=static_cast<float>(ylocal);
//#ifdef debug
//          std::cout<<"local "<<xlocal<<" "<<ylocal
//                   << " " <<(ysize/xsize*xlocal_f) << " "<<- ysize/xsize*xlocal_f + 4 * ylocal_f
//                   <<std::endl;
//#endif
//          if(ylocal_f>(ysize/(2*xsize) * xlocal_f) && ylocal_f< (-ysize/(2*xsize)*xlocal_f + 2 * ysize) )
//          {
//            this_index=index_min+xlocal+ sign_pnCCD_def_size * (ylocal);
//            //I do not need to set again to zero a pixel that was already masked!
//            //I have also to check that I have not landed on the other side of the CHIP
//            if (this_index>=0 && (this_index < sign_pnCCD_def_size_sq) && dp._ROImask[this_index]!=0)
//            {
//              if( dp._detROI._ROI[iROI].xcentre<=half_pnCCD_def_size )
//              {
//                if( (this_index%sign_pnCCD_def_size)<= signed_ROI_xsize+signed_ROI_xcentre )
//                {
//                  dp._ROImask[this_index]=0;
//                  //remember how many pixels I have masked
//                  number_of_pixelsettozero++;
//                }
//              }
//              else
//              {
//                if( (signed_ROI_xcentre-signed_ROI_xsize) < (this_index%sign_pnCCD_def_size) )
//                {
//                  dp._ROImask[this_index]=0;
//                  //remember how many pixels I have masked
//                  number_of_pixelsettozero++;
//                }
//              }
//            }
//          }
//        }
//      }
//      if(dp._detROI._ROI[iROI].orientation==-2)
//      {
//#ifdef debug
//        std::cout << printoutdef <<  "triangle seen vertex towards left" <<std::endl;
//#endif
//        for(size_t iFrame=u_indexROI_min;iFrame<u_indexROI_max; ++iFrame)
//        {
//          xlocal=iFrame%(2* dp._detROI._ROI[iROI].xsize);
//          ylocal=iFrame/(2* dp._detROI._ROI[iROI].xsize);
//          xlocal_f=static_cast<float>(xlocal);
//          ylocal_f=static_cast<float>(ylocal);
//          if(ylocal>(- ysize/(2*xsize) * xlocal_f + ysize) && ylocal<( ysize/(2*xsize) * xlocal_f + ysize) )
//          {
//            this_index=index_min+xlocal+ sign_pnCCD_def_size * (ylocal);
//            //I do not need to set again to zero a pixel that was already masked!
//            //I have also to check that I have not landed on the other side of the CHIP
//            if (this_index>=0 && (this_index < sign_pnCCD_def_size_sq) && dp._ROImask[this_index]!=0)
//            {
//              if( dp._detROI._ROI[iROI].xcentre<=half_pnCCD_def_size )
//              {
//                if( (this_index%sign_pnCCD_def_size)<= signed_ROI_xsize+signed_ROI_xcentre )
//                {
//                  dp._ROImask[this_index]=0;
//                  //remember how many pixels I have masked
//                  number_of_pixelsettozero++;
//                }
//              }
//              else
//              {
//                if( (signed_ROI_xcentre-signed_ROI_xsize) < (this_index%sign_pnCCD_def_size) )
//                {
//                  dp._ROImask[this_index]=0;
//                  //remember how many pixels I have masked
//                  number_of_pixelsettozero++;
//                }
//              }
//            }
//          }
//        }
//      }
//    }
//  } // end iROI loop

}
