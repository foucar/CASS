// Copyright (C) 2014 Lutz Foucar

/**
 * @file logcolor_map.cpp contains a logarithmic color map.
 *
 * @author Lutz Foucar
 */

#include <cmath>

#include <QtGui/QColor>
#include <QtGui/QRgb>

#include "logcolor_map.h"

using namespace jocassview;


LogColorMap::LogColorMap(const QColor &from, const QColor &to)
  : QwtLinearColorMap(from, to)
{

}

QRgb LogColorMap::rgb(const QwtInterval &interval, double value) const
{
  return QwtLinearColorMap::rgb(QwtInterval(std::log(interval.minValue()),
                                            std::log(interval.maxValue())),
                                std::log(value));
}
