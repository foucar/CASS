// Copyright (C) 2009 Jochen Küpper

#ifndef CASS_CONVERSIONBACKEND_H
#define CASS_CONVERSIONBACKEND_H

#include "cass.h"
#include "VMIEvent.h"


namespace cass {

/** @class abstract base for all data conversion backends

@author Jochen Küpper
@version 0.1
*/
class CASSSHARED_EXPORT ConversionBackend {
public:
    virtual ~ConversionBackend() = 0;

    /* Convert LCLS data to internal data format

    @param data LCLS data
    @param event CASS event object to fill in data
    */
    virtual void operator()(const void *data, cass::VMI::VMIEvent& event) = 0;
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
