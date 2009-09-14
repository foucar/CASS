// Copyright (C) 2009 Jochen Küpper

#include "image_handler.h"


ImageHandler::ImageHandler(const QString& expression, double updatePeriod)
    : _expression(expression), _timer(new QTimer(this))
{
    // create signal timer
    connect(_timer, SIGNAL(timeout()), this, SLOT(sendImage()));
    _timer->start(int(updatePeriod*1000));

}



void ImageHandler::addImage(const QImage& image, const ExperimentType& type)
{
    // _images[type].append(image);
}



void ImageHandler::sendImage()
{
    emit image(_image, _imageLock);
}






// Local Variables:
// coding: utf-8
// mode: C++
// c-file-offsets: ((c . 0) (innamespace . 0))
// c-file-style: "Stroustrup"
// fill-column: 100
// End:
