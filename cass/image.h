// Copyright (C) 2009 Jochen Küpper

#ifndef CASS_IMAGE_H
#define CASS_IMAGE_H

#include <map>
#include <QtGui/QImage>
#include "cass.h"

namespace cass {

/** @class Image container

This class provides an abstraction of a 2D image. The data can be stored as

@todo Can we get this independent of Qt without to much work in data translation? We should do this!

@author Jochen Küpper
@version 0.1
*/
class CASSSHARED_EXPORT Image {
public:

    /** Internal storage formats

    @param QImage store as standard Qt 2D image data (QImage)
    @param Tuple store sparse images as 3-tuples (row, column, intensity)
    */
    enum Storage {IMAGE, TUPLE};

    /** Construct an Image according to the specified storage type */
    Image(Storage type=IMAGE)
        : _storagetype(type)
    {};

    /** return a real iage form the internal data */
    QImage qimage() const;

    /** add another image to ourself

    @param img other image
    @return Sum of images
    */
    const Image& operator+(const Image& img) { return *this; };


protected:

    Storage _storagetype;

};


}

#endif



// Local Variables:
// coding: utf-8
// mode: C++
// c-file-offsets: ((c . 0) (innamespace . 0))
// c-file-style: "Stroustrup"
// fill-column: 100
// End:
