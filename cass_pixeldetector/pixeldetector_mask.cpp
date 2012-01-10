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
typedef pair<int,int> index_t;

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
bool operator<(const index_t& lhs, const index_t::first_type rhs)
{
  return ((lhs.first + lhs.second) < rhs);
}

/** calculate the scalar product of two indices
 *
 * perform operation \f$ lhs_1*rhs_1 + lhs_2*rhs_2\f$
 *
 * @return the result of the operation
 * @param lhs the left hand side of the operation
 * @param rhs the right hand side of the opeation
 *
 * @author Lutz Foucar
 */
index_t::first_type dot(const index_t& lhs, const index_t& rhs)
{
  return ((lhs.first*rhs.first)+(lhs.second*lhs.second));
}


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
  const index_t::first_type radius(s.value("Radius",2).toUInt());

  if ((center.first < radius) ||
      (center.second < radius) ||
      (static_cast<int>(data.columns) <= (center.first + radius)) ||
      (static_cast<int>(data.rows) <= (center.second + radius)))
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

  for (index_t::first_type row(lowerLeft.second); row < upperRight.second; ++row)
  {
    for (index_t::first_type column(lowerLeft.first); column < upperRight.first; ++column)
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
 * @cassttng PixelDetectors/\%name\%/CorrectionMaps/Mask/\%index\%/{LowerLeftX|LowerLeftY}\n
 *           The lower left pixel of the square element. The indizes given are
 *.          included in the square. Default is 0|0.
 * @cassttng PixelDetectors/\%name\%/CorrectionMaps/Mask/\%index\%/{UpperRightX|UpperRightY}\n
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
  if ((static_cast<int>(data.columns) <= upperRight.first) ||
      (static_cast<int>(data.rows) <= upperRight.second) ||
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
  for (index_t::first_type row(lowerLeft.second); row <= upperRight.second; ++row)
  {
    for (index_t::first_type column(lowerLeft.first); column <= upperRight.first; ++column)
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
  const index_t::first_type a(s.value("SemiAxisX",5).toUInt());
  const index_t::first_type b(s.value("SemiAxisY",2).toUInt());
  const size_t width(data.columns);

  if ((center.first < a) ||
      (center.second < b) ||
      (static_cast<int>(data.columns) <= (center.first + a)) ||
      (static_cast<int>(data.rows) <= (center.second + b)))
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

  for (index_t::first_type row(lowerLeft.second); row <= upperRight.second; ++row)
  {
    for (index_t::first_type column(lowerLeft.first); column <= upperRight.first; ++column)
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
 * To see whether a point is within a triangle on just creates a new coordinate
 * sytem where the origin is one point of the triangle defined by the three
 * points A B C. The two vectors AB and AC define the coordiante system. Then
 * any point P can be described as
 * \f[ P = A + u * (C - A) + v * (B - A)\f]
 * where u is the distance along the vector AC and v is the distance along AB.
 * once one has determined u and v for a given point P one has just to check
 * whether u and v are positive and whether their sum is smaller than 1 to see
 * whether the P is inside the triangle defined by Points ABC.
 * This is because "if u or v < 0 then we've walked in the wrong direction and
 * must be outside the triangle. Also if u or v > 1 then we've walked too far in
 * a direction and are outside the triangle. Finally if u + v > 1 then we've
 * crossed the edge BC again leaving the triangle."
 * One can rearrange the above equation to solve for u and v and the result is
 * \f{eqnarray*}{
 * u &=& \frac{(v1 \cdot v1)(v2 \cdot v0)-(v1 \cdot v0)(v2 \cdot v1)}
 *            {(v0 \cdot v0)(v1 \cdot v1) - (v0 \cdot v1)(v1 \cdot v0)} \\
 * v &=& \frac{(v0 \cdot v0)(v2 \cdot v1)-(v0 \cdot v1)(v2 \cdot v0)}
 *            {(v0 \cdot v0)(v1 \cdot v1) - (v0 \cdot v1)(v1 \cdot v0)}
 * \f}
 * where the dot between two vectors mark that it is a scalar product and the
 * vectors are defined as follows:
 * \f{eqnarray*}{
 * v0 $=$ AC \\
 * v1 $=$ AB \\
 * v2 $=$ AP
 * \f}
 *
 * Inspired by ideas found here (last checked Sep. 24th, 2011):
 * http://www.blackpawn.com/texts/pointinpoly/default.html
 *
 * @cassttng PixelDetectors/\%name\%/CorrectionMaps/Mask/\%index\%/{PointA_X|PointA_Y}\n
 *           The triangles first point. Default is 500|500.
 * @cassttng PixelDetectors/\%name\%/CorrectionMaps/Mask/\%index\%/{PointB_X|PointB_Y}\n
 *           The triangles first point. Default is 500|500.
 * @cassttng PixelDetectors/\%name\%/CorrectionMaps/Mask/\%index\%/{PointC_X|PointC_Y}\n
 *           The triangles first point. Default is 500|500.
 *
 * @param data the container containing the mask where the element should be added
 * @param s the settings element to read the mask element parameters from
 *
 * @author Lutz Foucar
 */
void addTriangle(CommonData &data, CASSSettings &s)
{
  const index_t A(make_pair(s.value("PointA_X",500).toUInt(),
                            s.value("PointA_Y",500).toUInt()));
  const index_t B(make_pair(s.value("PointB_X",500).toUInt(),
                            s.value("PointB_Y",500).toUInt()));
  const index_t C(make_pair(s.value("PointC_X",500).toUInt(),
                            s.value("PointC_Y",500).toUInt()));

  if (A == B ||
      B == C ||
      A == C ||
      static_cast<int>(data.columns) <= A.first ||
      static_cast<int>(data.columns) <= B.first ||
      static_cast<int>(data.columns) <= C.first ||
      static_cast<int>(data.rows) <= A.second ||
      static_cast<int>(data.rows) <= B.second ||
      static_cast<int>(data.rows) <= C.second )
  {
    throw invalid_argument("addTriangle(): the 3 Points "
                           "A("+toString(A.first)+","+toString(A.second)+"), "
                           "B("+toString(B.first)+","+toString(B.second)+"), "
                           "C("+toString(C.first)+","+toString(C.second)+"), "
                           "are inconsistent. Or outside the the mask boundaries "+
                           toString(data.columns) +","+ toString(data.rows));
  }

  const size_t width(data.columns);
  const index_t::first_type minX(min(min(A.first,B.first),C.first));
  const index_t::first_type minY(min(min(A.second,B.second),C.second));
  const index_t::first_type maxX(max(max(A.first,B.first),C.first));
  const index_t::first_type maxY(max(max(A.second,B.second),C.second));
  const index_t lowerLeft(make_pair(minX,minY));
  const index_t upperRight(make_pair(maxX,maxY));

  for (index_t::first_type row(lowerLeft.second); row <= upperRight.second; ++row)
  {
    for (index_t::first_type column(lowerLeft.first); column <= upperRight.first; ++column)
    {
      const index_t P(make_pair(column,row));

      const index_t v0(C - A);
      const index_t v1(B - A);
      const index_t v2(P - A);

      const index_t::first_type dot00 = dot(v0, v0);
      const index_t::first_type dot01 = dot(v0, v1);
      const index_t::first_type dot02 = dot(v0, v2);
      const index_t::first_type dot11 = dot(v1, v1);
      const index_t::first_type dot12 = dot(v1, v2);

      const float invDenom(1. / (dot00 * dot11 - dot01 * dot01));
      const float u((dot11 * dot02 - dot01 * dot12) * invDenom);
      const float v((dot00 * dot12 - dot01 * dot02) * invDenom);

      data.mask[TwoD2OneD(P,width)] *=  !((u+v < 1) && (0 < u) && (0 < v));
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
}

}//end namespace pixeldetector
}//end namespace cass
