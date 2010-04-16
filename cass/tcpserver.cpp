// Copyright (C) 2010 Jochen Küpper

#include <stdexcept>

#include "soapCASSsoapService.h"
#include "CASSsoap.nsmap"
#include "tcpserver.h"



// cass::SoapServerHelper *cass::SoapServerHelper::_instance(0);
// QMutex cass::SoapServerHelper::_mutex;


// // create an instance of the singleton
// cass::SoapServerHelper *cass::SoapServerHelper::instance(const EventGetter& event, const HistogramGetter& hist)
// {
//     QMutexLocker locker(&_mutex);
//     if(0 == _instance)
//         _instance = new SoapServerHelper(event, hist);
//     return _instance;
// }



// cass::SoapServerHelper *cass::SoapServerHelper::instance()
// {
//     QMutexLocker locker(&_mutex);
//     if(0 == _instance)
//         throw std::runtime_error("SoapServerHelper does not exist");
//     return _instance;
// }



// // destroy the instance of the singleton
// void cass::SoapServerHelper::destroy()
// {
//     QMutexLocker locker(&_mutex);
//     delete _instance;
//     _instance = 0;
// }



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
