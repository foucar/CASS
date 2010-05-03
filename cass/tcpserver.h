// CASS TCP server
//
// Copyright (C) 2010 Jochen Küpper

#ifndef CASS_TCPSERVER_H
#define CASS_TCPSERVER_H

#include <stdexcept>
#include <QtCore/QThread>

#include "cass_event.h"
#include "event_getter.h"
#include "histogram_getter.h"
#include "soapCASSsoapService.h"


namespace cass
{


/*! Handle a single SOAP request

@author Jochen Küpper
*/
class SoapHandler : public QThread
{
    Q_OBJECT;

public:

    SoapHandler(CASSsoapService *soap)
        : _soap(soap)
        {};

    virtual ~SoapHandler() { delete _soap; };

    /** handle request and terminate*/
    virtual void run();


protected:

    /** the service */
    CASSsoapService *_soap;
};



/** SOAP server

@author Jochen Küpper

@todo Update getImage to actually return an direct (i.e., unscaled) TIFF image of the float values
@todo Provide a multi-content query -- with the SOAP-attachment strategy, we can very well/easily deliver
multiple histograms/images within one message. Let's do it...(requires a smart updated API).
*/
class SoapServer : public QThread
{
    Q_OBJECT;

    friend class ::CASSsoapService;


public:

    /** create the instance if not it does not exist already */
    static SoapServer *instance(const EventGetter& event, const HistogramGetter& hist, size_t port=12321);

    /** destroy the instance */
    static void destroy();


signals:

    void quit();

    void readini(size_t what);


protected:

    /** perform thread-work */
    virtual void run();

    /** return existing instance for our friends -- if it doesn't exist, throw exception */
    static SoapServer *instance() {
        QMutexLocker locker(&_mutex);
        if(0 == _instance)
            throw std::runtime_error("SoapServer does not exist");
        return _instance;
    };


    /** get_event functor */
    const EventGetter& get_event;

    /** get_histogram functor */
    const HistogramGetter& get_histogram;

    /** allow our friends to emit the quit() signal */
    void emit_quit() { emit quit(); };

    /** allow our friends to emit the readinin() signal */
    void emit_readini(size_t what) { emit readini(what); };

    /** the service */
    CASSsoapService *_soap;

    /** maximum backlog of open requests */
    static const size_t _backlog;

    /** server port */
    const size_t _port;


private:

    /** Constructor */
    SoapServer(const EventGetter& event, const HistogramGetter& hist, size_t port, QObject *parent=0)
        : QThread(parent), get_event(event), get_histogram(hist),
          _soap(new CASSsoapService), _port(port)
        {
            VERBOSEOUT(std::cout << "SoapServer starting on port " << _port << std::endl);
        };

    /** Disabled constructor */
    SoapServer();

    /** Disabled copy constructor */
    SoapServer(const SoapServer&);

    /** Disabled assignment */
    SoapServer& operator=(const SoapServer&);

    /** Destructor

    clean up SOAP
    */
    ~SoapServer() { _soap->destroy(); delete _soap; };

    /** pointer to the singleton instance */
    static SoapServer *_instance;

    /** Singleton operation locker */
    static QMutex _mutex;
};

}



#endif



// Local Variables:
// coding: utf-8
// mode: C++
// c-file-style: "Stroustrup"
// c-file-offsets: ((c . 0) (innamespace . 0))
// fill-column: 100
// End:
