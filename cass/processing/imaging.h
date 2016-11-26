// Copyright (C) 2010-2013 Lutz Foucar

/**
 * @file imaging.h processors to generate a test image
 *
 * @author Stephan Kassemeyer
 * @author Lutz Foucar
 */

#ifndef _IMAGING_H_
#define _IMAGING_H_

#include "processor.h"

namespace cass
{
//forward declaration
class CASSEvent;
class CASSSettings;

/** Test image
 *
 * @PPList "240": Test image
 *
 * @see Processor for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng Processor/\%name\%/{sizeX} \n
 *           Width of testimage (default = 1024)
 * @cassttng Processor/\%name\%/{sizeY} \n
 *           Height of testimage (default = 1024)
 * @cassttng Processor/\%name\%/{FixedValue} \n
 *           Use a fixed value instead of the product of the column and row index.
 *           Default is false
 * @cassttng Processor/\%name\%/{Value} \n
 *           In case FixedValue is true, this is the value that the image will
 *           be filled with. Default is 0
 * @cassttng Processor/\%name\%/Mask/{size}\n
 *           The number of mask elements that are part of the complete mask.
 *           Default is 0.
 * @cassttng Processor/\%name\%/Mask/\%index\%/{Type}\n
 *           Name of the mask element. Default is "Unknown", which let the code
 *           ignore that element. Possible values are:
 *           - "square": a square region of the mask. See cass::pp240::square
 *                       for details.
 *           - "circle" or "circ": a circular region of the mask. See
 *                                 cass::pp240::circle for details
 *           - "ellipse": a ellipsodial region of the mask. See
 *                        cass::pp240::ellipse for details
 *           - "triangle": a triangular region of the mask. See
 *                         cass::pp240::triangle for details
 *           - "ring": a ring with inner and outer part composed by ellipsoids.
 *                     See cass::pp240::ring for details
 *
 * @author Stephan Kassemeyer
 * @author Lutz Foucar
 */
class pp240 : public Processor
{
public:
  /** constructor. */
  pp240(const name_t&);

  /** overwrite default behaviour and just return the constant */
  virtual const result_t& result(const CASSEvent::id_t)
  {
    return *_result;
  }

  /** overwrite default behaviour don't do anything */
  virtual void releaseEvent(const CASSEvent &){}

  /** overwrite default behaviour don't do anything */
  virtual void processEvent(const CASSEvent&){}

  /** load the settings of this pp */
  virtual void loadSettings(size_t);

  /** define the index type */
  typedef std::pair<int,int> index_t;

  /** define a floating point version of the index */
  typedef std::pair<float,float> indexf_t;

private:
  /** add a circle to the mask
   *
   * goes through the sqare that conatins the cirlce and checks whether the index
   * is covered by the circle. If so the mask at that index will be set to false.
   *
   * @cassttng Processor/\%name\%/Mask/\%index\%/{CenterX|CenterY}\n
   *           The center of the circle. Default is 500|500.
   * @cassttng Processor/\%name\%/Mask/\%index\%/{Radius}\n
   *           The radius of the circle. Default is 2.
   * @cassttng Processor/\%name\%/Mask/\%index\%/{Value}\n
   *           The value that will be set to all the pixels within the mask.
   *           Default is 1
   *
   * @param data the container containing the mask where the element should be added
   * @param s the settings element to read the mask element parameters from
   *
   * @author Nicola Coppola
   * @author Lutz Foucar
   */
  void circle(result_t &data, CASSSettings &s);

  /** add a square element to the mask
   *
   * sets all pixels covered by the square to 0.
   *
   * @cassttng Processor/\%name\%/Mask/\%index\%/{LowerLeftX|LowerLeftY}\n
   *           The lower left pixel of the square element. The indizes given are
   *           included in the square. Default is 0|0.
   * @cassttng Processor/\%name\%/Mask/\%index\%/{UpperRightX|UpperRightY}\n
   *           The upper right pixel of the square element. The indizes given are
   *           included in the square. Default is 1023|1023.
   * @cassttng Processor/\%name\%/Mask/\%index\%/{Value}\n
   *           The value that will be set to all the pixels within the mask.
   *           Default is 1
   *
   * @param data the container containing the mask where the element should be added
   * @param s the settings element to read the mask element parameters from
   *
   * @author Nicola Coppola
   * @author Lutz Foucar
   */
  void square(result_t &data, CASSSettings &s);

  /** add a ellipsodial element to the mask
   *
   * adds an ellipsodial to the mask. Will iterate trhough the sqare that contains
   * the ellipse and checks which pixels should be masked.
   *
   * @cassttng Processor/\%name\%/Mask/\%index\%/{CenterX|CenterY}\n
   *           The central point of the ellipse. Default is 500|500.
   * @cassttng Processor/\%name\%/Mask/\%index\%/{SemiAxisX|SemiAxisY}\n
   *           The semi axis along x and y of the ellipse. By definition the
   *           longer one defines the major axis and the smaller on the minor axis.
   *           Default is 5|4.
   * @cassttng Processor/\%name\%/Mask/\%index\%/{Value}\n
   *           The value that will be set to all the pixels within the mask.
   *           Default is 1
   *
   * @param data the container containing the mask where the element should be added
   * @param s the settings element to read the mask element parameters from
   *
   * @author Nicola Coppola
   * @author Lutz Foucar
   */
  void ellipse(result_t &data, CASSSettings &s);

  /** add a ring
   *
   * define the ring by two ellipses; an inner and outer ellipse. The area covered
   * by the out but not by the inner will be masked. The two ellipsoids can have
   * different centers and axis. Therefore the ring can take any shape.
   *
   * @cassttng Processor/\%name\%/Mask/\%index\%/{InnerCenterX|InnerCenterY}\n
   *           The central point of the inner ellipse. Default is 500|500.
   * @cassttng Processor/\%name\%/Mask/\%index\%/{InnerSemiAxisX|InnerSemiAxisY}\n
   *           The semi axis along x and y of the inner ellipse. By definition the
   *           longer one defines the major axis and the smaller on the minor axis.
   *           Default is 5|4.
   * @cassttng Processor/\%name\%/Mask/\%index\%/{OuterCenterX|OuterCenterY}\n
   *           The central point of the outer ellipse. Default is 500|500.
   * @cassttng Processor/\%name\%/Mask/\%index\%/{OuterSemiAxisX|OuterSemiAxisY}\n
   *           The semi axis along x and y of the outer ellipse. By definition the
   *           longer one defines the major axis and the smaller on the minor axis.
   *           Default is 20|20.
   * @cassttng Processor/\%name\%/Mask/\%index\%/{Value}\n
   *           The value that will be set to all the pixels within the mask.
   *           Default is 1
   *
   * @author Lutz Foucar
   */
  void ring(result_t &data, CASSSettings &s);

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
   * @cassttng Processor/\%name\%/Mask/\%index\%/{PointA_X|PointA_Y}\n
   *           The triangles first point. Default is 500|500.
   * @cassttng Processor/\%name\%/Mask/\%index\%/{PointB_X|PointB_Y}\n
   *           The triangles first point. Default is 500|500.
   * @cassttng Processor/\%name\%/Mask/\%index\%/{PointC_X|PointC_Y}\n
   *           The triangles first point. Default is 500|500.
   * @cassttng Processor/\%name\%/Mask/\%index\%/{Value}\n
   *           The value that will be set to all the pixels within the mask.
   *           Default is 1
   *
   * @param data the container containing the mask where the element should be added
   * @param s the settings element to read the mask element parameters from
   *
   * @author Lutz Foucar
   */
  void triangle(result_t &data, CASSSettings &s);

  /** the constant image */
  result_t::shared_pointer _result;

};



}
#endif
