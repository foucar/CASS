// Copyright (C) 2010 Jochen Küpper

#include <stdexcept>
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
const size_t SoapServer::_backlog(100);
const size_t SoapServer::_port(12321);



void SoapHandler::run()
{
    _soap->serve();   // serve request
    _soap->destroy(); // dealloc C++ data, dealloc data and clean up (destroy + end)
    exit();           // terminate thread
}



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



void SoapServer::run()
{
    // define timeouts and such for individual requests
    _soap->send_timeout   =   60; // 60 seconds
    _soap->recv_timeout   =   60; // 60 seconds
    _soap->accept_timeout = 3600; // server stops after 1 hour of inactivity
    _soap->max_keep_alive =  100; // max keep-alive sequence
    // allow immediate re-use of address/socket
    _soap->bind_flags = SO_REUSEADDR;
    // start SOAP
    if(SOAP_INVALID_SOCKET == _soap->bind(NULL, _port, _backlog))
        throw std::runtime_error("No valid socket for SOAP server");
    while(true) {
        if(SOAP_INVALID_SOCKET == _soap->accept()) {
            if(_soap->errnum) {
                _soap->soap_stream_fault(std::cerr);
                throw std::runtime_error("No valid socket for SOAP connection");
            }
#warning What is the right action to do here?
            throw std::runtime_error("Server timeout for SOAP connection");
            break;
        }
        CASSsoapService *tsoap(_soap->copy()); // make a safe copy
        if(! tsoap)
            break;
        SoapHandler *handler(new SoapHandler(tsoap));
        handler->run();
    }
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
    return soap_set_dime_attachment(this, (char *)data.c_str(), sizeof(data.c_str()), "application/octet-stream",
                                    QString::number(type).toStdString().c_str(), 0, NULL);
}



int CASSsoapService::getHistogram(size_t type, bool *success)
{
    std::string data(cass::SoapServer::instance()->get_histogram(cass::HistogramParameter(type)));
    std::cerr << "CASSsoapService::getHistogram -- size = " << data.size() << std::endl;
    *success = true;
    soap_set_dime(this); // enable dime
    return soap_set_dime_attachment(this, (char *)data.data(), data.size(), "application/octet-stream",
                                    QString::number(type).toStdString().c_str(), 0, NULL);
}



int CASSsoapService::getImage(size_t format, size_t type, bool *success)
{
    int result;
    try {
        QImage image(cass::SoapServer::instance()->get_histogram.qimage(cass::HistogramParameter(type)));
        QByteArray ba;
        QBuffer buffer(&ba);
        buffer.open(QIODevice::WriteOnly);
        switch(format) {
        case 1:  // TIFF
            image.save(&buffer, "TIFF");
            *success = true;
            soap_set_dime(this); // enable dime
            result = soap_set_dime_attachment(this, ba.data(), ba.size(), "image/tiff",
                                              QString::number(type).toStdString().c_str(), 0, NULL);
            break;
        case 2:  // PNG
            image.save(&buffer, "PNG");
            *success = true;
            soap_set_dime(this); // enable dime.
            result = soap_set_dime_attachment(this, ba.data(), ba.size(), "image/png",
                                              QString::number(type).toStdString().c_str(), 0, NULL);
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
    return result;
}






// Local Variables:
// coding: utf-8
// mode: C++
// c-file-offsets: ((c . 0) (innamespace . 0))
// c-file-style: "Stroustrup"
// fill-column: 100
// End:
