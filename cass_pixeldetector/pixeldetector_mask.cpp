//Copyright (C) 2011, 2012 Lutz Foucar

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
#include <iostream>

#include "pixeldetector_mask.h"

#include "cass_settings.h"
#include "common_data.h"
#include "log.h"

using namespace cass;
using namespace pixeldetector;
using namespace std;

namespace cass
{
namespace pixeldetector
{
/** an index within a matrix */
typedef pair<int,int> index_t;

/** an index within a matrix but with with floating point precision */
typedef pair<float,float> indexf_t;

/** operates a plus on two indices
 *
 * performs \f$(lhs_1+rhs_1)(lhs_2+rhs_2)\f$.
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
 * performs \f$(lhs_1-rhs_1)(lhs_2-rhs_2)\f$.
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
 * performs \f$(lhs_1*rhs_1)(lhs_2*rhs_2)\f$.
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
 * performs \f$(lhs_1/rhs_1)(lhs_2/rhs_2)\f$.
 *
 * @return the result of the operation
 * @param lhs the left hand side of the operation
 * @param rhs the right hand side of the opeation
 *
 * @author Lutz Foucar
 */
indexf_t operator/(const indexf_t& lhs, const indexf_t& rhs)
{
  return make_pair(lhs.first / rhs.first,
                   lhs.second / rhs.second);
}

/** operates less of an  indices to a scalar
 *
 * performs \f$(lhs_1+lhs_2)<rhs\f$.
 *
 * @return the result of the operation
 * @param lhs the left hand side of the operation
 * @param rhs the right hand side of the opeation
 *
 * @author Lutz Foucar
 */
bool operator<(const indexf_t& lhs, const indexf_t::first_type rhs)
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
 * @param matrixIndex in the matrix
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
 * @param linearisedIndex the linearized index in the matrix
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
      /** @todo check whether this works */
      const index_t idx(make_pair(column,row));
      const index_t idx_sq((idx - center)*(idx - center));
      const indexf_t idx_tmp(idx_sq / axis_sq);
      data.mask[TwoD2OneD(idx,width)] *= !(idx_tmp < 1);
//      const float first(static_cast<float>(idx_sq.first)/static_cast<float>(axis_sq.first));
//      const float second(static_cast<float>(idx_sq.second)/static_cast<float>(axis_sq.second));
//      data.mask[TwoD2OneD(idx,width)] *= (1 < (first+second));
    }
  }
}

/** add a ring
 *
 * define the ring by two ellipses; an inner and outer ellipse. The area covered
 * by the out but not by the inner will be masked. The two ellipsoids can have
 * different centers and axis. Therefore the ring can take any shape.
 *
 * @cassttng PixelDetectors/\%name\%/CorrectionMaps/Mask/\%index\%/{InnerCenterX|InnerCenterY}\n
 *           The central point of the inner ellipse. Default is 500|500.
 * @cassttng PixelDetectors/\%name\%/CorrectionMaps/Mask/\%index\%/{InnerSemiAxisX|InnerSemiAxisY}\n
 *           The semi axis along x and y of the inner ellipse. By definition the
 *           longer one defines the major axis and the smaller on the minor axis.
 *           Default is 5|4.
 * @cassttng PixelDetectors/\%name\%/CorrectionMaps/Mask/\%index\%/{OuterCenterX|OuterCenterY}\n
 *           The central point of the outer ellipse. Default is 500|500.
 * @cassttng PixelDetectors/\%name\%/CorrectionMaps/Mask/\%index\%/{OuterSemiAxisX|OuterSemiAxisY}\n
 *           The semi axis along x and y of the outer ellipse. By definition the
 *           longer one defines the major axis and the smaller on the minor axis.
 *           Default is 20|20.
 *
 * @author Lutz Foucar
 */
void addRing(CommonData &data, CASSSettings &s)
{
  QWriteLocker lock(&data.lock);
  const index_t outer_center(make_pair(s.value("OuterCenterX",500).toUInt(),
                                       s.value("OuterCenterY",500).toUInt()));
  const index_t::first_type outer_a(s.value("OuterSemiAxisX",5).toUInt());
  const index_t::first_type outer_b(s.value("OuterSemiAxisY",2).toUInt());
  const index_t inner_center(make_pair(s.value("InnerCenterX",500).toUInt(),
                                       s.value("InnerCenterY",500).toUInt()));
  const index_t::first_type inner_a(s.value("InnerSemiAxisX",20).toUInt());
  const index_t::first_type inner_b(s.value("InnerSemiAxisY",20).toUInt());
  const size_t width(data.columns);

  if ((outer_center.first < outer_a) ||
      (outer_center.second < outer_b) ||
      (static_cast<int>(data.columns) <= (outer_center.first + outer_a)) ||
      (static_cast<int>(data.rows) <= (outer_center.second + outer_b)))
  {
    throw invalid_argument("addCircle(): The outer semi axis x '" + toString(outer_a) +
                           "' and b '" + toString(outer_b) +
                           "' are choosen to big and do not fit with center ("
                           + toString(outer_center.first) +","
                           + toString(outer_center.second)+")");
  }
  if ((inner_center.first < inner_a) ||
      (inner_center.second < inner_b) ||
      (static_cast<int>(data.columns) <= (inner_center.first + inner_a)) ||
      (static_cast<int>(data.rows) <= (inner_center.second + inner_b)))
  {
    throw invalid_argument("addCircle(): The inner semi axis x '" + toString(inner_a) +
                           "' and b '" + toString(inner_b) +
                           "' are choosen to big and do not fit with center ("
                           + toString(inner_center.first) +","
                           + toString(inner_center.second)+")");
  }

  const size_t min_col(min(outer_center.first - outer_a,
                           inner_center.first - inner_a));
  const size_t max_col(max(outer_center.first + outer_a,
                           inner_center.first + inner_a));
  const size_t min_row(min(outer_center.second - outer_b,
                           inner_center.second - inner_b));
  const size_t max_row(max(outer_center.second + outer_b,
                           inner_center.second + inner_b));

  const index_t outer_axis_sq(make_pair(outer_a,outer_b)*make_pair(outer_a,outer_b));
  const index_t inner_axis_sq(make_pair(inner_a,inner_b)*make_pair(inner_a,inner_b));

  for (size_t row(min_row); row <= max_row; ++row)
  {
    for (size_t column(min_col); column <= max_col; ++column)
    {
      const index_t idx(make_pair(column,row));

      const index_t idx_sq_inner((idx - inner_center)*(idx - inner_center));
      const indexf_t idx_tmp_inner(idx_sq_inner / inner_axis_sq);
      const bool isNotInInner(!(idx_tmp_inner < 1));

      const index_t idx_sq_outer((idx - outer_center)*(idx - outer_center));
      const indexf_t idx_tmp_outer(idx_sq_outer / outer_axis_sq);
      const bool isInOuter(idx_tmp_outer < 1);

      data.mask[TwoD2OneD(idx,width)] *= !(isInOuter && isNotInInner);
    }
  }
}

/** add a triangluar element to the mask
 *
 * To see whether a point is within a triangle one can use barycentric coordinates.
 * See http://en.wikipedia.org/wiki/Barycentric_coordinates_(mathematics) for
 * details. A point within barycentric can be defined by converting to these
 * coordinates. The bayrocentric coordinates are represented by three points
 * A, B, C. Each point can be represented by
 * \f$ \vec{P} = \lambda_1\vec{A} + \lambda_2\vec{B} + \lambda_3\vec{C}\f$
 * where \f$ \lambda_1, \lambda_2, \lambda_3\f$ can be determined from
 * \f$ \vec{P} \f$ using the components of the triangle points and the wanted point
 * \f$ \vec{P} = (x,y) ; \vec{A} = (x_1,y_1); \vec{B} = (x_2,y_2); \vec{C} = (x_3,y_3)\f$
 * with these definition the following equalities are given:
 * \f{eqnarray*}{
 * \lambda_1&=&\frac{(y_2-y_3)(x-x_3)+(x_3-x_2)(y-y_3)}{(y_2-y_3)(x_1-x_3)+(x_3-x_2)(y_1-y_3)}\\
 * \lambda_2&=&\frac{(y_3-y_1)(x-x_3)+(x_1-x_3)(y-y_3)}{(y_2-y_3)(x_1-x_3)+(x_3-x_2)(y_1-y_3)}\\
 * \lambda_3&=&1-\lambda_1-\lambda_2
 * \f}
 * With this we know that \f$ \vec{P} \f$ lies within the triangluar when
 * \f[ 0 \leq \lambda_i \leq 1 \f]
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
  const float x1(A.first);
  const float x2(B.first);
  const float x3(C.first);
  const float y1(A.second);
  const float y2(B.second);
  const float y3(C.second);
  const float denom( (y2-y3)*(x1-x3) + (x3-x2)*(y1-y3) );

  for (index_t::first_type row(lowerLeft.second); row <= upperRight.second; ++row)
  {
    for (index_t::first_type column(lowerLeft.first); column <= upperRight.first; ++column)
    {
      const index_t P(make_pair(column,row));
      const float x(P.first);
      const float y(P.second);

      const float l1( ( (y2-y3)*(x-x3) + (x3-x2)*(y-y3) ) / denom );
      const float l2( ( (y3-y1)*(x-x3) + (x1-x3)*(y-y3) ) / denom );
      const float l3( 1 - l1 - l2 );

      data.mask[TwoD2OneD(P,width)] *= !((0<=l1) && (l1<=1) &&
                                         (0<=l2) && (l2<=1) &&
                                         (0<=l3) && (l3<=1));
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
  functions["ring"] = &addRing;

  /** reset the mask before creating it*/
  data.mask.resize(data.columns*data.rows);
  fill(data.mask.begin(),data.mask.end(),1);
  int size = s.beginReadArray("Mask");
  for (int i = 0; i < size; ++i)
  {
    s.setArrayIndex(i);
    string type(s.value("MaskElementType","square").toString().toStdString());
    if (functions.find(type) == functions.end())
      throw invalid_argument("createCASSMask(): Unknown Mask Element Type '" +type+ "'");
    Log::add(Log::DEBUG0,"createCASSMask: add mask element type '" + type +"'");
    functions[type](data,s);
  }
  s.endArray();
}

}//end namespace pixeldetector
}//end namespace cass
