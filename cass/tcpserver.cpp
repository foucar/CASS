// Copyright (C) 2010 Jochen Küpper

#include "soapCASSsoapService.h"
#include "CASSsoap.nsmap"
#include "tcpserver.h"


namespace cass
{

SoapServer *SoapServer::_instance(0);
QMutex SoapServer::_mutex;


SoapServer::SoapServer(const EventGetter& event, const HistogramGetter& hist, QObject *parent)
    : QObject(parent), get_event(event), get_histogram(hist)
{ // leave thsi definition here (out-lined) so GCC knows how to make a vtable
}


SoapServer *SoapServer::instance(const EventGetter& event, const HistogramGetter& hist)
// create an instance of the singleton
{
    QMutexLocker locker(&_mutex);
    if(0 == _instance)
        _instance = new SoapServer(event, hist);
    return _instance;
}



// destroy the instance of the singleton
void SoapServer::destroy()
{
    QMutexLocker locker(&_mutex);
    delete _instance;
    _instance = 0;
}



} // end namespace cass



int CASSsoapService::quit(bool *success)
{
    *success = true;;
    return SOAP_OK;
}



int CASSsoapService::readini(size_t what, bool *success)
{
    *success = true;;
    return SOAP_OK;
}



int CASSsoapService::getEvent(size_t type, bool *success)
{
    int *data = new int[128];
    *success = true;
    soap_set_dime(this); // enable dime
    return soap_set_dime_attachment(this, (char *)data, sizeof(data), "application/octet-stream", NULL, 0, NULL);
}



int CASSsoapService::getHistogram(size_t type, bool *success)
{
    int *data = new int[128];
    *success = true;
    soap_set_dime(this); // enable dime
    return soap_set_dime_attachment(this, (char *)data, sizeof(data), "application/octet-stream", NULL, 0, NULL);
}



int CASSsoapService::getImage(size_t format, size_t type, bool *success)
{
    int *data = new int[128];
    *success = true;
    soap_set_dime(this); // enable dime
    return soap_set_dime_attachment(this, (char *)data, sizeof(data), "application/octet-stream", NULL, 0, NULL);
}






// Local Variables:
// coding: utf-8
// mode: C++
// c-file-offsets: ((c . 0) (innamespace . 0))
// c-file-style: "Stroustrup"
// fill-column: 100
// End:
