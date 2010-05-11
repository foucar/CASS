// Copyright (C) 2010 Jochen Küpper

#include <QtGui/QColor>

#include "histogram.h"


namespace cass
{
/** Convert Histogram2DFloat::value_t to uint8_t

@author Jochen Küpper
*/
class value2pixel
{
public:

    value2pixel(Histogram2DFloat::value_t min, Histogram2DFloat::value_t max)
        : _min(min), _max(max)
        {};

    uint8_t operator()(Histogram2DFloat::value_t val) {
        return uint8_t((val - _min) / (_max - _min) * 0xff);
    };

protected:

    Histogram2DFloat::value_t _min, _max;
};




QImage Histogram2DFloat::qimage()
{
    QImage qi(shape().first, shape().second, QImage::Format_Indexed8);
    qi.setColorCount(256);
    for(unsigned i=0; i<256; ++i)
        qi.setColor(i, QColor(i, i, i).rgb());
    qi.fill(0);
    uint8_t *data(qi.bits());
//    value2pixel converter(0,1);
    value2pixel converter(min(), max());
    lock.lockForRead();
    // Subtract 8 to get the size of the buffer excluding over/underflow flags
    std::transform(_memory.begin(), _memory.end()-8, data, converter);
    lock.unlock();
    return qi;
}

} // end namespace cass



// Local Variables:
// coding: utf-8
// mode: C++
// c-file-style: "gnu"
// c-file-offsets: ((c . 0) (innamespace . 0))
// fill-column: 100
// End:
