// Copyright (C) 2010 Jochen Küpper

#ifndef __CASS_IMAGE_H__
#define __CASS_IMAGE_H__

#include <algorithm>
#include <QtGui/QImage>
#include "histogram.h"

namespace cass
{

/*! 2D image wrapper for 2D histogram

Currently this wrapper creates an image of the histogram and then decouples itself. You can update
the image using the update method.

@author Jochen Küpper

@todo Make this an bidirectional wrapper around 2d histograms. This, first of all, requires to use a
float-representation of pixels.
*/
class Image : public QImage
{
    /*! Create image from 2D histogram */
    Image(const Histogram2DFloat& hist)
        : QImage(hist.shape().first, hist.shape().second, QImage::Format_Indexed8)
        { update(hist); };

    /*! Reset image to new histogram data */
    void update(const Histogram2DFloat& hist) {
        for(int r=0; r<hist.shape().first; ++r)
            for(int c=0; c<hist.shape().second; ++c)
                setPixel(r, c, hist.bin(r, c));
    };

    QByteArray tiff() {
        QByteArray ba;
        QBuffer buffer(&ba);
        buffer.open(QIODevice::WriteOnly);
        save(&buffer, "TIFF");
        return ba;
    };
};

} // end namespace cass


#endif // __CASS_IMAGE_H__



// Local Variables:
// coding: utf-8
// mode: C++
// c-file-offsets: ((c . 0) (innamespace . 0))
// c-file-style: "Stroustrup"
// fill-column: 100
// End:
