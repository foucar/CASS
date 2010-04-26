// Copyright (C) 2010 Jochen Küpper

#include <stdexcept>
#include <QtCore/QBuffer>
#include <QtCore/QByteArray>
#include <QtCore/QQueue>
#include <QtGui/QColor>
#include <QtGui/QImage>

#include "CASSsoap.nsmap"
#include "histogram_getter.h"
#include "postprocessing/postprocessor.h"
#include "tcpserver.h"


namespace cass
{

SoapServer *SoapServer::_instance(0);
QMutex SoapServer::_mutex;
const size_t SoapServer::_backlog(100);



void SoapHandler::run()
{
    _soap->serve();   // serve request
    _soap->destroy(); // dealloc C++ data, dealloc data and clean up (destroy + end)
    exit();           // terminate thread
}



SoapServer *SoapServer::instance(const EventGetter& event, const HistogramGetter& hist, size_t port)
{
    QMutexLocker locker(&_mutex);
    if(0 == _instance)
        _instance = new SoapServer(event, hist, port);
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
    _soap->max_keep_alive = 1000; // max keep-alive sequence
    // allow immediate re-use of address/socket
    _soap->bind_flags = SO_REUSEADDR;
    // start SOAP
    if(SOAP_INVALID_SOCKET == _soap->bind(NULL, _port, _backlog))
        throw std::runtime_error("No valid socket for SOAP server");
    while(true) {
        if(SOAP_INVALID_SOCKET == _soap->accept()) {
            if(_soap->errnum) {
                _soap->soap_stream_fault(std::cerr);
                std::cerr << "*** No valid socket for SOAP connection ***" << std::endl;
            } else
                std::cerr << "*** Server timeout for SOAP connection ***" << std::endl;
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
    std::cerr << "CASSsoapService::quit" << std::endl;
    cass::SoapServer::instance()->emit_quit();
    *success = true;;
    return SOAP_OK;
}



int CASSsoapService::readini(size_t what, bool *success)
{
    std::cerr << "CASSsoapService::readini(what=" << what << ")" << std::endl;
    cass::SoapServer::instance()->emit_readini(what);
    *success = true;;
    return SOAP_OK;
}



int CASSsoapService::getEvent(size_t type, unsigned t1, unsigned t2, bool *success)
{
    static QQueue<std::string *> queue;
    std::string *data(new std::string(cass::SoapServer::instance()->get_event(cass::EventParameter(type, t1, t2))));
    queue.enqueue(data);
    if(10 < queue.size())
        queue.dequeue();
    *success = true;
    soap_set_dime(this); // enable dime
    return soap_set_dime_attachment(this, (char *)data->c_str(), data->size(), "application/cassevent",
                                    QString::number(type).toStdString().c_str(), 0, NULL);
}



int CASSsoapService::getHistogram(size_t type, bool *success)
{
    static QQueue<std::pair<size_t, std::string> *> queue;
    try {
        // get data
        cass::SoapServer *server(cass::SoapServer::instance());
        std::pair<size_t, std::string> *data(new std::pair<size_t, std::string>(server->get_histogram(cass::HistogramParameter(type))));
        // MIME type
        std::string mimetype;
        switch(data->first) {
        case 0:
            mimetype = std::string("application/cass0Dhistogram");
            break;
        case 1:
            mimetype = std::string("application/cass1Dhistogram");
            break;
        case 2:
            mimetype = std::string("application/cass2Dhistogram");
            break;
        default:
            mimetype = std::string("application/casshistogram");
            break;
        }
        // keep bytes around for a while -- this should mitigate the "zeros" problem
        queue.enqueue(data);
        if(10 < queue.size())
            delete queue.dequeue();
        // answer request
        *success = true;
        soap_set_dime(this);
        return soap_set_dime_attachment(this, (char *)data->second.data(), data->second.size(), mimetype.c_str(),
                                        QString::number(type).toStdString().c_str(), 0, NULL);
    } catch(cass::InvalidHistogramError&) {
        *success = false;
        return SOAP_FATAL_ERROR;
    }
}



int CASSsoapService::getImage(size_t format, size_t type, bool *success)
{
    // keep bytes around for a while -- this should mitigate the "zeros" problem
    static QQueue<QByteArray *> queue;
    int result;
    try {
        QByteArray *ba(new QByteArray);
        QBuffer buffer(ba);
        // get image from histogram
        QImage image(cass::SoapServer::instance()->get_histogram.qimage(cass::HistogramParameter(type)));
        // save image to bytearray and attach it to SOAP message
        buffer.open(QIODevice::WriteOnly);
        switch(format) {
        case 1:  // TIFF
            image.save(&buffer, "TIFF");
            *success = true;
            soap_set_dime(this); // enable dime
            result = soap_set_dime_attachment(this, ba->data(), ba->size(), "image/tiff",
                                              QString::number(type).toStdString().c_str(), 0, NULL);
            break;
        case 2:  // PNG
            image.save(&buffer, "PNG");
            *success = true;
            soap_set_dime(this); // enable dime.
            result = soap_set_dime_attachment(this, ba->data(), ba->size(), "image/png",
                                              QString::number(type).toStdString().c_str(), 0, NULL);
            break;
        default:
            success = false;
            result = SOAP_FATAL_ERROR;
            break;
        }
        queue.enqueue(ba);
        if(10 < queue.size())
            delete queue.dequeue();
    } catch(std::exception&) { // includes InvalidHistogram
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
