//Copyright (C) 2010-2011 Lutz Foucar

/**
 * @file delayline_detector_analyzer_simple.cpp file contains the definition of
 *                                              classes and functions that
 *                                              analyzses a delayline detector.
 *
 * @author Lutz Foucar
 */
#ifndef _POSCALCULATOR_H
#define _POSCALCULATOR_H

#include <utility>
#include <cmath>

namespace cass
{
  namespace ACQIRIS
  {
    /** position calculator base class
     *
     * @author Lutz Foucar
     */
    class PositionCalculator
    {
    public:
      virtual ~PositionCalculator() {}
      virtual std::pair<double,double> operator()(const std::pair<double, double>&)=0;
    };

    /** position calculator for quad anode
     *
     * @author Lutz Foucar
     */
    class XYCalc : public PositionCalculator
    {
    public:
      virtual std::pair<double,double> operator()(const std::pair<double, double>& layer)
      {
        return layer;
      }
    };

    /** position calculator for hex anodes u and v layer
     *
     * @author Lutz Foucar
     */
    class UVCalc : public PositionCalculator
    {
    public:
      virtual std::pair<double,double> operator()(const std::pair<double, double>& layer)
      {
        const double u(layer.first);
        const double v(layer.second);
        return std::make_pair(u, 1./std::sqrt(3) * (u-2.*v));
      }
    };

    /** position calculator for hex anodes u and w layer
     *
     * @author Lutz Foucar
     */
    class UWCalc : public PositionCalculator
    {
    public:
      virtual std::pair<double,double> operator()(const std::pair<double, double>& layer)
      {
        const double u(layer.first);
        const double w(layer.second);
        return std::make_pair(u, 1./std::sqrt(3) * (2.*w-u));
      }
    };

    /** position calculator for hex anodes u and v layer
     *
     * @author Lutz Foucar
     */
    class VWCalc : public PositionCalculator
    {
    public:
      virtual std::pair<double,double> operator()(const std::pair<double, double>& layer)
      {
        const double v(layer.first);
        const double w(layer.second);
        return std::make_pair(v+w, 1./std::sqrt(3) * (w-v));
      }
    };
  }
}
#endif
