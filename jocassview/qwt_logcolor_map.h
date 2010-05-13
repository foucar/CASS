/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_LOGCOLOR_MAP_H
#define QWT_LOGCOLOR_MAP_H

#include <qglobal.h>
#include <qcolor.h>
#include "math.h"

#include "qwt_color_map.h"

#if QT_VERSION < 0x040000
#include <qvaluevector.h>
#else
#include <qvector.h>
#endif
#include "qwt_array.h"
#include "qwt_double_interval.h"

#if defined(QWT_TEMPLATEDLL)
// MOC_SKIP_BEGIN
template class QWT_EXPORT QwtArray<double>;
// MOC_SKIP_END
#endif

/*!
  \brief QwtColorMap is used to map values into colors. 

  For displaying 3D data on a 2D plane the 3rd dimension is often
  displayed using colors, like f.e in a spectrogram. 

  Each color map is optimized to return colors for only one of the
  following image formats:

  - QImage::Format_Indexed8\n
  - QImage::Format_ARGB32\n

  \sa QwtPlotSpectrogram, QwtScaleWidget
*/



/*!
  \brief QwtLogColorMap builds a color map from color stops.
  
  A color stop is a color at a specific position. The valid
  range for the positions is [0.0, 1.0]. When mapping a value
  into a color it is translated into this interval. If 
  mode() == FixedColors the color is calculated from the next lower
  color stop. If mode() == ScaledColors the color is calculated
  by interpolating the colors of the adjacent stops. 
*/
class QWT_EXPORT QwtLogColorMap: public QwtColorMap
{
public:
    /*!
       Mode of color map
       \sa setMode(), mode()
    */
    enum Mode
    {
        FixedColors,
        ScaledColors
    };

    enum transformId
    {
        trans_lin,
        trans_pow10,
        trans_log10,
        trans_sqrt,
        trans_square
    };

    QwtLogColorMap(QwtColorMap::Format = QwtColorMap::RGB);
    QwtLogColorMap( const QColor &from, const QColor &to,
        QwtColorMap::Format = QwtColorMap::RGB);

    QwtLogColorMap(const QwtLogColorMap &);

    virtual ~QwtLogColorMap();

    QwtLogColorMap &operator=(const QwtLogColorMap &);

    virtual QwtColorMap *copy() const;

    void setMode(Mode);
    Mode mode() const;

    void setColorInterval(const QColor &color1, const QColor &color2);
    void addColorStop(double value, const QColor&);
    QwtArray<double> colorStops() const;

    QColor color1() const;
    QColor color2() const;

    virtual QRgb rgb(const QwtDoubleInterval &, double value) const;
    virtual unsigned char colorIndex(
        const QwtDoubleInterval &, double value) const;

    class ColorStops;

    //void setTransform( (*foo)(double) transform ) { _transformfunc = transform; };

    void setTransformId(transformId id) {_transformId = id;};
protected:
    transformId _transformId;
    //void (*foo)(double) _transformfunc;
private:
    class PrivateData;
    PrivateData *d_data;
};



#endif
