// Copyright (C) 2010-2013 Lutz Foucar

/**
 * @file imaging.cpp processors to generate a test image
 *
 * @author Stephan Kassemeyer
 * @author Lutz Foucar
 */

#include <QtCore/QString>

#include "imaging.h"

#include "cass_settings.h"
#include "log.h"


using namespace cass;
using namespace std;
using namespace std::tr1;
using placeholders::_1;
using placeholders::_2;

namespace cass
{

/** helper to convert shape into the 1D index
 *
 * @return index in the linearised array
 * @param matrixIndex in the matrix
 * @param width the width of the matrix
 */
pp240::index_t::first_type toOneD(const pp240::index_t matrixIndex, pp240::index_t::first_type width)
{
  return matrixIndex.second * width + matrixIndex.first;
}

/** operates a minus on two indices
 *
 * performs \f$(lhs_1-rhs_1)(lhs_2-rhs_2)\f$.
 *
 * @return the result of the operation
 * @param lhs the left hand side of the operation
 * @param rhs the right hand side of the opeation
 */
pp240::index_t operator-(const pp240::index_t& lhs, const pp240::index_t& rhs)
{
  return make_pair(lhs.first - rhs.first, lhs.second - rhs.second);
}

/** operates times on two indices
 *
 * performs \f$(lhs_1*rhs_1)(lhs_2*rhs_2)\f$.
 *
 * @return the result of the operation
 * @param lhs the left hand side of the operation
 * @param rhs the right hand side of the opeation
 */
pp240::index_t operator*(const pp240::index_t& lhs, const pp240::index_t& rhs)
{
  return make_pair(lhs.first * rhs.first, lhs.second * rhs.second);
}

/** operates a plus on two indices
 *
 * performs \f$(lhs_1+rhs_1)(lhs_2+rhs_2)\f$.
 *
 * @return the result of the operation
 * @param lhs the left hand side of the operation
 * @param rhs the right hand side of the opeation
 */
pp240::index_t operator+(const pp240::index_t& lhs, const pp240::index_t& rhs)
{
  return make_pair(lhs.first + rhs.first, lhs.second + rhs.second);
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
bool operator<(const pp240::indexf_t& lhs, const pp240::indexf_t::first_type rhs)
{
  return ((lhs.first + lhs.second) < rhs);
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
pp240::indexf_t operator/(const pp240::indexf_t& lhs, const pp240::indexf_t& rhs)
{
  return make_pair(lhs.first / rhs.first,
                   lhs.second / rhs.second);
}

} //end namespace cass

// *** test image ***
pp240::pp240(const name_t &name)
  : Processor(name)
{
  loadSettings(0);
}

void pp240::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("Processor");
  s.beginGroup(QString::fromStdString(name()));
  _result = result_t::shared_pointer(new result_t(s.value("sizeX", 1024).toInt(),
                                                  s.value("sizeY", 1024).toInt()));
  for (int xx=0; xx<s.value("sizeX", 1024).toInt(); ++xx)
    for (int yy=0; yy<s.value("sizeY", 1024).toInt(); ++yy)
      (*_result)[histogramming::bin(_result->axis(result_t::xAxis),
                                    _result->axis(result_t::yAxis),
                                    make_pair(yy,xx))] =
        s.value("FixedValue",false).toBool() ? s.value("Value",0).toFloat() : xx*yy;
  string logout("processor " + name() +": creates test image with shape '" +
           toString(s.value("sizeX", 1024).toInt()) + "x" +
           toString(s.value("sizeY", 1024).toInt()) + "'");
  // now add masks elements to it
  map<string,function<void(result_t&, CASSSettings&)> > functions;
  functions["circle"] = bind(&pp240::circle,this,_1,_2);
  functions["square"] = bind(&pp240::square,this,_1,_2);
  functions["triangle"] = bind(&pp240::triangle,this,_1,_2);
  functions["ellipse"] = bind(&pp240::ellipse,this,_1,_2);
  functions["ring"] = bind(&pp240::ring,this,_1,_2);
  int size = s.beginReadArray("Mask");
  for (int i = 0; i < size; ++i)
  {
    s.setArrayIndex(i);
    string type(s.value("Type","Unknown").toString().toStdString());
    if (type == "Unknown")
      continue;
    if (functions.find(type) == functions.end())
      throw invalid_argument("pp240(): '" + name() + "': Unknown Mask  Type '" +
                             type + "'");
    Log::add(Log::DEBUG0,"pp240 '"+name()+"': add mask element type '" + type +"'");
    functions[type](*_result,s);
  }
  s.endArray();

  Log::add(Log::INFO,logout);
}


void pp240::circle(result_t &data, CASSSettings &s)
{
  const index_t center(make_pair(s.value("CenterX",500).toUInt(),
                                 s.value("CenterY",500).toUInt()));
  const index_t::first_type radius(s.value("Radius",2).toUInt());

  if ((center.first < radius) ||
      (center.second < radius) ||
      (static_cast<int>(data.shape().first) <= (center.first + radius)) ||
      (static_cast<int>(data.shape().second) <= (center.second + radius)))
  {
    throw out_of_range("pp204:circle(): The radius '" + toString(radius) +
                       "' is choosen to big and does not fit the image. Center of circle ("
                       + toString(center.first) +","
                       + toString(center.second)+")");
  }
  const size_t radius_sq(radius*radius);
  const index_t lowerLeft(make_pair(center.first-radius, center.second-radius));
  const index_t upperRight(make_pair(center.first+radius, center.second+radius));
  const size_t width(data.shape().first);

  for (index_t::first_type row(lowerLeft.second); row < upperRight.second; ++row)
  {
    for (index_t::first_type column(lowerLeft.first); column < upperRight.first; ++column)
    {
      const index_t idx(make_pair(column,row));
      const index_t idx_sq((idx - center)*(idx - center));
      if (idx_sq < radius_sq)
        data[toOneD(idx,width)] = s.value("Value",1.f).toFloat();
    }
  }
}

void pp240::square(result_t &data, CASSSettings &s)
{
  const index_t lowerLeft(make_pair(s.value("LowerLeftX",0).toUInt(),
                                    s.value("LowerLeftY",0).toUInt()));
  const index_t upperRight(make_pair(s.value("UpperRightX",1024).toUInt(),
                                     s.value("UpperRightY",1024).toUInt()));
  if ((static_cast<int>(data.shape().first) <= upperRight.first) ||
      (static_cast<int>(data.shape().second) <= upperRight.second))
    throw invalid_argument("addSquare(): The upper right coordinate ("
                           + toString(upperRight.first) +","
                           + toString(upperRight.second)+") "+
                           "is too big for the mask that has a size of ("
                           + toString(data.shape().first) +","
                           + toString(data.shape().second)+") ");
  if((upperRight.first < lowerLeft.first) ||
     (upperRight.second < lowerLeft.second))
    throw out_of_range("addSquare(): The lowerLeft corner ("
                       + toString(lowerLeft.first) +","
                       + toString(lowerLeft.second)+") "+
                       "is not really to the lower left of ("
                       + toString(upperRight.first) +","
                       + toString(upperRight.second)+") ");

  const size_t width(data.shape().first);
  for (index_t::first_type row(lowerLeft.second); row <= upperRight.second; ++row)
  {
    for (index_t::first_type column(lowerLeft.first); column <= upperRight.first; ++column)
    {
      const index_t idx(make_pair(column,row));
      data[toOneD(idx,width)] = s.value("Value",1.f).toFloat();
    }
  }
}

void pp240::ellipse(result_t &data, CASSSettings &s)
{
  const index_t center(make_pair(s.value("CenterX",500).toUInt(),
                                 s.value("CenterY",500).toUInt()));
  const index_t::first_type a(s.value("SemiAxisX",5).toUInt());
  const index_t::first_type b(s.value("SemiAxisY",2).toUInt());
  const size_t width(data.shape().first);

  if ((center.first < a) ||
      (center.second < b))
    throw invalid_argument("addCircle(): The semi axis a '" + toString(a) +
                           "' and b '" + toString(b) +
                           "' are choosen to big and do not fit with center ("
                           + toString(center.first) +","
                           + toString(center.second)+")");
  if((static_cast<int>(data.shape().first) <= (center.first + a)) ||
     (static_cast<int>(data.shape().second) <= (center.second + b)))
    throw out_of_range("addCircle(): The semi axis boundaries a '" + toString(center.first + a) +
                           "' and b '" + toString(center.second + b) +
                           "' are choosen to big and do not fit with center ("
                           + toString(data.shape().first) +","
                           + toString(data.shape().second)+")");

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
      if (idx_tmp < 1)
        data[toOneD(idx,width)] = s.value("Value",1.f).toFloat();
    }
  }
}

void pp240::ring(result_t &data, CASSSettings &s)
{
  const index_t outer_center(make_pair(s.value("OuterCenterX",500).toUInt(),
                                       s.value("OuterCenterY",500).toUInt()));
  const index_t::first_type outer_a(s.value("OuterSemiAxisX",5).toUInt());
  const index_t::first_type outer_b(s.value("OuterSemiAxisY",2).toUInt());
  const index_t inner_center(make_pair(s.value("InnerCenterX",500).toUInt(),
                                       s.value("InnerCenterY",500).toUInt()));
  const index_t::first_type inner_a(s.value("InnerSemiAxisX",20).toUInt());
  const index_t::first_type inner_b(s.value("InnerSemiAxisY",20).toUInt());
  const size_t width(data.shape().first);

  if ((outer_center.first < outer_a) ||
      (outer_center.second < outer_b))
    throw invalid_argument("addCircle(): The outer semi axis x '" +
                           toString(outer_a) + "' and b '" + toString(outer_b) +
                           "' are choosen to big and do not fit with center ("
                           + toString(outer_center.first) +","
                           + toString(outer_center.second)+")");
  if ((static_cast<int>(data.shape().first) <= (outer_center.first + outer_a)) ||
      (static_cast<int>(data.shape().second) <= (outer_center.second + outer_b)))
    throw out_of_range(string("addCircle(): The outer semi axis boundaries a '") +
                       toString(outer_center.first + outer_a) +
                       "' and b '" + toString(outer_center.second + outer_b) +
                       "' are choosen to big and do not fit into image ("
                       + toString(data.shape().first) +","
                       + toString(data.shape().second)+")");

  if ((inner_center.first < inner_a) ||
      (inner_center.second < inner_b))
    throw invalid_argument(string("addCircle(): The inner semi axis x '") +
                           toString(inner_a) + "' and b '" + toString(inner_b) +
                           "' are choosen to big and do not fit with center ("
                           + toString(inner_center.first) +","
                           + toString(inner_center.second)+")");

  if((static_cast<int>(data.shape().first) <= (inner_center.first + inner_a)) ||
     (static_cast<int>(data.shape().second) <= (inner_center.second + inner_b)))
    throw out_of_range(string("addCircle(): The inner semi axis boundaries a '") +
                       toString(inner_center.first + inner_a) + "' and b '" +
                       toString(inner_center.second + inner_b) +
                       "' are choosen to big and do not fit into image ("
                       + toString(data.shape().first) +","
                       + toString(data.shape().second)+")");

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

      if (isInOuter && isNotInInner)
        data[toOneD(idx,width)] = s.value("Value",1.f).toFloat();
    }
  }
}

void pp240::triangle(result_t &data, CASSSettings &s)
{
  const index_t A(make_pair(s.value("PointA_X",500).toUInt(),
                            s.value("PointA_Y",500).toUInt()));
  const index_t B(make_pair(s.value("PointB_X",500).toUInt(),
                            s.value("PointB_Y",500).toUInt()));
  const index_t C(make_pair(s.value("PointC_X",500).toUInt(),
                            s.value("PointC_Y",500).toUInt()));

  if (A == B ||
      B == C ||
      A == C)
    throw invalid_argument("addTriangle(): the 3 Points "
                           "A("+toString(A.first)+","+toString(A.second)+"), "
                           "B("+toString(B.first)+","+toString(B.second)+"), "
                           "C("+toString(C.first)+","+toString(C.second)+"), "
                           "are inconsistent.");

  if (static_cast<int>(data.shape().first) <= A.first ||
      static_cast<int>(data.shape().first) <= B.first ||
      static_cast<int>(data.shape().first) <= C.first ||
      static_cast<int>(data.shape().second) <= A.second ||
      static_cast<int>(data.shape().second) <= B.second ||
      static_cast<int>(data.shape().second) <= C.second )
    throw out_of_range("addTriangle(): the 3 Points "
                       "A("+toString(A.first)+","+toString(A.second)+"), "
                       "B("+toString(B.first)+","+toString(B.second)+"), "
                       "C("+toString(C.first)+","+toString(C.second)+"), "
                       "are outside the the mask boundaries "+
                       toString(data.shape().first) +","+ toString(data.shape().second));

  const size_t width(data.shape().first);
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

      if ((0<=l1) && (l1<=1) &&  (0<=l2) && (l2<=1) && (0<=l3) && (l3<=1))
        data[toOneD(P,width)] = s.value("Value",1.f).toFloat();
    }
  }
}
