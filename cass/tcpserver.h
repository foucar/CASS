// CASS TCP server
//
// Copyright (C) 2010 Jochen Küpper

#ifndef CASS_TCPSERVER_H
#define CASS_TCPSERVER_H

#include <QtCore/QObject>

#include "cass_event.h"
#include "event_getter.h"
#include "histogram_getter.h"


namespace cass
{

class SoapServerHelper : public QObject
{
    Q_OBJECT;

public:

    /** create the instance if not it does not exist already */
    static SoapServerHelper *instance(const EventGetter& event, const HistogramGetter& hist);

    /** return instance -- if it doesn't exist, throw exception */
    static SoapServerHelper *instance();

    /** destroy the instance */
    static void destroy();

    /** get_event functor */
    const EventGetter& get_event;

    /** get_histogram functor */
    const HistogramGetter& get_histogram;


public slots:

    void emit_quit() { emit quit(); };

    void emit_readini(size_t what) { emit readini(what); };


signals:

    void quit();

    void readini(size_t what);


protected:

    /** Constructor */
    SoapServerHelper(const EventGetter& event, const HistogramGetter& hist, QObject *parent=0)
        : QObject(parent), get_event(event), get_histogram(hist)
        {};

    SoapServerHelper();

    SoapServerHelper(const SoapServerHelper&);

    SoapServerHelper& operator=(const SoapServerHelper&);

    ~SoapServerHelper() {};

    /** pointer to the singleton instance */
    static SoapServerHelper *_instance;

    /** Singleton operation locker */
    static QMutex _mutex;
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
