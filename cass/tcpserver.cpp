// Copyright (C) 2010 Jochen Küpper

#include <QtCore/QBuffer>
#include <QtCore/QByteArray>
#include <QtGui/QColor>
#include <QtGui/QImage>

#include "CASSsoap.nsmap"
#include "histogram_getter.h"
#include "tcpserver.h"


namespace cass
{

SoapServer *SoapServer::_instance(0);
QMutex SoapServer::_mutex;


SoapServer *SoapServer::instance(const EventGetter& event, const HistogramGetter& hist)
{
    QMutexLocker locker(&_mutex);
    if(0 == _instance)
        _instance = new SoapServer(event, hist);
    return _instance;
}



void SoapServer::destroy()
{
    QMutexLocker locker(&_mutex);
    delete _instance;
    _instance = 0;
}



SoapServer::SoapServer(const EventGetter& event, const HistogramGetter& hist, QObject *parent)
    : QObject(parent), get_event(event), get_histogram(hist),
      _soap(new CASSsoapService)
{
    // start SOAP
    std::cerr << "soap.run() says " << _soap->run(12321) << std::endl;
}


} // end namespace cass



int CASSsoapService::quit(bool *success)
{
    cass::SoapServer::instance()->emit_quit();
    *success = true;;
    return SOAP_OK;
}



int CASSsoapService::readini(size_t what, bool *success)
{
    cass::SoapServer::instance()->emit_readini(what);
    *success = true;;
    return SOAP_OK;
}



int CASSsoapService::getEvent(size_t type, unsigned t1, unsigned t2, bool *success)
{
    std::string data(cass::SoapServer::instance()->get_event(cass::EventParameter(type, t1, t2)));
    *success = true;
    soap_set_dime(this); // enable dime
    return soap_set_dime_attachment(this, (char *)data.c_str(), sizeof(data.c_str()),
                                    "application/octet-stream", NULL, 0, NULL);
}



int CASSsoapService::getHistogram(size_t type, bool *success)
{
    std::string data(cass::SoapServer::instance()->get_histogram(cass::HistogramParameter(type)));
    *success = true;
    soap_set_dime(this); // enable dime
    return soap_set_dime_attachment(this, (char *)data.c_str(), sizeof(data.c_str()),
                                    "application/octet-stream", NULL, 0, NULL);
}



int CASSsoapService::getImage(size_t format, size_t type, bool *success)
{
    std::cout << "CASSsoapService::getImage" << std::endl;
    int result;
    try {
        // QImage image(cass::SoapServer::instance()->get_histogram.qimage(cass::HistogramParameter(type)));
        QImage image(1024, 1024, QImage::Format_Indexed8);
        image.setColorCount(256);
        for(unsigned i=0; i<256; ++i)
            image.setColor(i, QColor(i, i, i).rgb());
        image.fill(type);
        QByteArray ba;
        QBuffer buffer(&ba);
        buffer.open(QIODevice::WriteOnly);
        std::cout << "CASSsoapService::getImage - buffer opened" << std::endl;
        switch(format) {
        case 1:  // TIFF
            image.save(&buffer, "TIFF");
            buffer.close();
            std::cout << "CASSsoapService::getImage - image saved" << std::endl;
            *success = true;
            soap_set_dime(this); // enable dime.
            std::cout << "CASSsoapService::getImage - sizeof(ba.data()): " << sizeof(ba.data()) << std::endl;
            result = soap_set_dime_attachment(this, ba.data(), sizeof(ba.data()), "image/tiff", NULL, 0, NULL);
            break;
        case 2:  // PNG
            image.save(&buffer, "PNG");
            std::cout << "CASSsoapService::getImage - image saved" << std::endl;
            *success = true;
            soap_set_dime(this); // enable dime.
            std::cout << "CASSsoapService::getImage - sizeof(ba.data()): " << sizeof(ba.data()) << std::endl;
            result = soap_set_dime_attachment(this, ba.data(), sizeof(ba.data()), "image/png", NULL, 0, NULL);
            break;
        default:
            success = false;
            result = SOAP_FATAL_ERROR;
            break;
        }
    } catch(std::exception) {
        success = false;
        return SOAP_FATAL_ERROR;
    }
    std::cout << "CASSsoapService::getImage done" << std::endl;
    return result;
}






// Local Variables:
// coding: utf-8
// mode: C++
// c-file-offsets: ((c . 0) (innamespace . 0))
// c-file-style: "Stroustrup"
// fill-column: 100
// End:
