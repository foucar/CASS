// CASS TCP server
//
// Copyright (C) 2010 Jochen Küpper

#ifndef CASS_TCPSERVER_H
#define CASS_TCPSERVER_H

#include <stdexcept>
#include <QtCore/QObject>

#include "cass_event.h"
#include "event_getter.h"
#include "histogram_getter.h"
#include "soapCASSsoapService.h"


namespace cass
{


class SoapServer : public QObject
{
    Q_OBJECT;

    friend class ::CASSsoapService;


public:

    /** create the instance if not it does not exist already */
    static SoapServer *instance(const EventGetter& event, const HistogramGetter& hist);

    /** destroy the instance */
    static void destroy();


signals:

    void quit();

    void readini(size_t what);


protected:

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


private:

    /** Constructor */
    SoapServer(const EventGetter& event, const HistogramGetter& hist, QObject *parent=0);

    SoapServer();

    SoapServer(const SoapServer&);

    SoapServer& operator=(const SoapServer&);

    ~SoapServer() { delete _soap; };

    /** pointer to the singleton instance */
    static SoapServer *_instance;

    /** Singleton operation locker */
    static QMutex _mutex;

    /** the service */
    CASSsoapService *_soap;
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
