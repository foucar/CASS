// Copyright (C) 2009 Jochen Küpper

#ifndef DIODE_IMAGEHANDLER_H
#define DIODE_IMAGEHANDLER_H


#include <QtCore/QReadWriteLock>
#include <QtCore/QTimer>
#include <QtGui/QImage>
#include <list>
#include <map>
#include "diode.h"


/** @class ImageHandler

@author Jochen Küpper
@version 0.1
*/
class ImageHandler : QObject
{
    Q_OBJECT

public:

    /** Construct an ImageHandler

    @param updatePeriod Signal an updated image after every period of the specified length (default
    0.1 s)
    */
    ImageHandler(const QString& expression, double updatePeriod=0.1);

signals:

    /** signal (and send) new composite image ready for display

    @param image The latest composite image ready for display
    @param lock A ReadWriteLock tha neds to be obtained for reading when the image is used.
    */
    void image(const QImage& image, const QReadWriteLock& lock);


public slots:

    /** Add new image to handler

    @param image New image
    @param type Experiment-type of image
    */
    void addImage(const QImage& image, const ExperimentType& type);


protected slots:

    /** Send new image vis signal image */
    void sendImage();


protected:

    QString _expression;

    QImage _image;
    QReadWriteLock _imageLock;

    std::map<ExperimentType, std::list<QImage> > _images;

    QTimer *_timer;
};


#endif



// Local Variables:
// coding: utf-8
// mode: C++
// c-file-offsets: ((c . 0) (innamespace . 0))
// c-file-style: "Stroustrup"
// fill-column: 100
// End:
