// Copyright (C) 2009 Jochen Küpper

#ifndef CASS_EVENTQUEUE_H
#define CASS_EVENTQUEUE_H

#include <QtCore/QObject>
#include "cass.h"

namespace cass {

/** @class EventQueue

@author Jochen Küpper
@version 0.1

@todo Make Singleton
*/
class CASSSHARED_EXPORT EventQueue : public QObject {
    Q_OBJECT;
public:

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
