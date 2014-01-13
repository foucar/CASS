// Copyright (C) 2014 Lutz Foucar

/**
 * @file logcolor_map.cpp contains a logarithmic color map.
 *
 * @author Lutz Foucar
 */

#include <cmath>

#include <QtCore/QDebug>

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
  return QwtLinearColorMap::rgb(QwtInterval(std::log10(interval.minValue()),
                                            std::log10(interval.maxValue())),
                                std::log10(value));
}
